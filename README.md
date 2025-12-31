# USB Key Sender2

## specs

* M5AtomS3U
https://docs.m5stack.com/en/core/AtomS3U

* PC
VS code
Docker


## erase rom
```bash
# フラッシュロムのサイズを確認
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 flash_id

# storageの 2MB-3MBを削除
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_region 0x110000 0x100000

# storageの 2MB以降を削除
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_region 0x110000 0x6F0000
```

## hid settings

idf.py menuconfig under Component config > TinyUSB Stack > Human Interface Device Class (HID), setting TINYUSB_HID_COUNT to a value greater than 0 will enable TinyUSB HID.

## build and settings

1. Connect AtomS3U to PC
2. Press reset button long
3. `idf.py -p /dev/ttyACM0 erase-flash`
4. Press reset button long
5. Click build button (spanner icon) at VS Code extension.
6. Remove AtomS3U from PC
7. Connect AtomS3U to PC with pressing button (The following steps are required for the initial build)
8. Open and edit settings.txt
9. Close settings.txt
10. Unmount the USB device, wait about 5 seconds, and then unplug it. (On Windows, when it is unmounted, the LED blinks blue or green. Then unplug it.)



## usages

### settings
1. Connect AtomS3U to PC with pressing button
2. Open and edit settings.txt
3. Close settings.txt
4. Unmount the USB device, wait about 10 seconds, and then unplug it.

### input key
1. Connect AtomS3U to PC
2. Wait until the LED lights up in green and then turns off.(If it blinks red, it indicates an error. Please check `settings.txt`.)
3. Press button to input key to PC.


## JSON examples

* Notepad (Windows)
```json
{
  "keys": [
    {"mod": "GUI", "key": "r"},
    "notepad",
    {"key": "ENTER"},
    "Hello! This is an auto-typed message.",
    {"mod": "CTRL", "key": "a"},
    {"mod": "CTRL", "key": "x"},
    "This is the pasted text: ",
    {"mod": "CTRL", "key": "v"},
    {"key": "ENTER"},
    "Have a good one!"
  ]
}
```

* Browser (Windows)
```json
{
  "keys": [
    {"mod": "GUI", "key": "r"},
    "https://gemini.google.com/",
    {"key": "ENTER"},
    "What is the meaning of 42?",
    {"key": "ENTER"}
  ]
}
```

* ipconfig (Windows)
```json
{
  "keys": [
    {"mod": "GUI", "key": "r"},
    "cmd",
    {"key": "ENTER"},
    "ipconfig",
    {"key": "ENTER"}
  ]
}
```
* Excel: Copy and paste to the line below
```json
{
  "keys": [
    {"mod": "CTRL", "key": "c"},
    {"key": "ARROW_DOWN"},
    {"mod": "CTRL", "key": "v"},
    {"key": "ENTER"}
  ]
}
```

* tail (linux)
```json
{
  "keys": [
    "tail -f /var/log/syslog",
    {"key": "ENTER"}
  ]
}
```

---

## reset and build
```bash
$ idf.py -p /dev/ttyACM0 erase-flash
$ idf.py fullclean
$ idf.py build flash monitor
```


## flash size to 8MB

```bash
$ idf.py menuconfig 
Serial flasher config  > Flash size > 8MB
```

## errors

* Error: device reports readiness to read but returned no data (device disconnected or multiple access on port?)

* Saved PC:0x40379836
    -- 0x40379836: esp_cpu_wait_for_intr at /opt/esp/idf/components/esp_hw_support/cpu.c:145
  `not an error`

---

This repository is based on tusb_composite_msc_serialdevice from the esp-idf framework, which can be found at https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb/device/tusb_composite_msc_serialdevice .
