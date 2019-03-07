from bluetool import *

bluetooth = Bluetooth()
bluetooth.scan()
devices = bluetooth.get_available_devices()
print(devices)