import asyncio
from bleak import BleakScanner, BleakClient
from threading import Thread
import time

# ---- Encoding ----
def to_byte(val):
    """Convert -1.0 to +1.0 to a signed byte (two's complement)"""
    signed = max(-128, min(127, int(val * 128.0)))
    return signed & 0xFF

# ---- BLE UUIDs (must match ble_server.ino) ----
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
DRIVE_UUID   = "12345678-1234-5678-1234-56789abcdef1"
DEVICE_NAME  = "RobotCar"

class DiffCarBLEClient:
    def __init__(self, device_name=DEVICE_NAME, resolution=0.05):
        self.device_name = device_name
        self.resolution = resolution

        self.x = 0.0  # linear  (-1.0=back,  1.0=forward)
        self.w = 0.0  # angular (-1.0=left, 1.0=right)

        self.client = None
        self._cached = None

        self._pending = None
        self._queued  = None

        self._reconnect_enabled = False
        self._reconnect_thread  = None

        self.loop   = asyncio.new_event_loop()
        self.thread = Thread(target=self.loop.run_forever, daemon=True)
        self.thread.start()

    # ---- Internal async ----
    async def _async_find_device(self):
        print(f"Scanning for '{self.device_name}' (service UUID: {SERVICE_UUID})...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: SERVICE_UUID.lower() in [str(u).lower() for u in adv.service_uuids],
            timeout=10.0
        )
        if not device:
            print("Device not found")
            return None
        print(f"Found '{device.name}' at {device.address}")
        return device

    async def _async_connect(self):
        device = await self._async_find_device()
        if not device:
            raise Exception("Device not found")
        self.client = BleakClient(device.address)
        await self.client.connect()
        if not self.client.is_connected:
            raise Exception("Connection failed")
        print("Connected\n")

    async def _async_disconnect(self):
        if self.client:
            await self.client.disconnect()
            print("Disconnected")

    async def _async_send(self, x, w, force=False):
        if not self.is_connected:
            return
        payload = (to_byte(x), to_byte(w))
        if not force and self._cached == payload:
            return
        await self.client.write_gatt_char(DRIVE_UUID, bytes(payload))
        self._cached = payload
        print(f"Sent -> x: {x:+.2f}  w: {w:+.2f}  (bytes: {payload[0]:3d}, {payload[1]:3d})")

    # ---- Reconnect loop ----
    def _reconnect_loop(self):
        while self._reconnect_enabled:
            if not self.is_connected:
                try:
                    asyncio.run_coroutine_threadsafe(self._async_connect(), self.loop).result()
                except Exception as e:
                    print(f"Connection failed ({e}). Retrying in 3s...")
                    time.sleep(3)
            else:
                time.sleep(1)

    # ---- Public API ----
    @property
    def is_connected(self):
        return self.client is not None and self.client.is_connected

    def connect(self):
        """Start background auto-connect loop (non-blocking)"""
        if self._reconnect_thread and self._reconnect_thread.is_alive():
            return
        self._reconnect_enabled = True
        self._reconnect_thread = Thread(target=self._reconnect_loop, daemon=True)
        self._reconnect_thread.start()

    def disconnect(self):
        """Stop reconnect loop and disconnect (blocking)"""
        self._reconnect_enabled = False
        if self.is_connected:
            asyncio.run_coroutine_threadsafe(self._async_disconnect(), self.loop).result()

    def send(self, force=False):
        """Send current x/w values (non-blocking, queues if busy)"""
        payload = (self.x, self.w, force)
        if self._pending is None or self._pending.done():
            self._pending = asyncio.run_coroutine_threadsafe(
                self._async_send(*payload), self.loop
            )
            self._pending.add_done_callback(lambda f: self._on_send_complete())
        else:
            self._queued = payload

    def _on_send_complete(self):
        if self._queued is not None:
            x, w, force = self._queued
            self._queued = None
            self.x = x
            self.w = w
            self.send(force=force)

    def set_velocity(self, x, w, force=False):
        """Set velocity and send. x: linear (-1=back, 1=fwd), w: angular (-1=left, 1=right)"""
        r = self.resolution
        self.x = round(x / r) * r
        self.w = round(w / r) * r
        self.send(force=force)

    def stop(self):
        """Stop all motors (blocking)"""
        asyncio.run_coroutine_threadsafe(
            self._async_send(0.0, 0.0, force=True), self.loop
        ).result()
        self.x = self.w = 0.0


if __name__ == "__main__":
    import user_input as inp

    car = DiffCarBLEClient()
    car.connect()

    print("Controls: W/S=forward/back  A/D=turn  Left Stick=drive  Ctrl+C=quit")

    SLOW = 0.4
    FAST = 0.75

    try:
        while True:
            boost = inp.get_bipolar_ctrl(high_key='c', high_game='RT0')
            scale = SLOW + (FAST - SLOW) * boost

            x = inp.get_bipolar_ctrl('w', 's', 'LY0') * scale
            w = inp.get_bipolar_ctrl('a', 'd', 'LX0') * scale
            car.set_velocity(x, w)
            time.sleep(0.02)

    except KeyboardInterrupt:
        pass
    finally:
        car.stop()
        car.disconnect()
