# USB Key Sender2

## Prerequisites

* M5AtomS3U
https://docs.m5stack.com/en/core/AtomS3U
* PC(Windows, Linux, Mac)
* VS code
* Docker

## Instalation (VS Code Dev Container)

1. Open the project folder in VS Code.
2. When prompted, click "Reopen in Container" (or run Dev Containers: Reopen in Container from the Command Palette).
3. Wait for the container to build. All tools and dependencies will be installed automatically.


* Using Local ESP-IDF: While not explicitly tested, the project follows the standard ESP-IDF structure. It should build and flash using idf.py commands in any environment where ESP-IDF is correctly installed.

## build and settings

1. Connect AtomS3U to PC
2. Press reset button long
3. Open terminal in extension and `$ idf.py -p /dev/ttyACM0 erase-flash`
4. Press reset button long
5. Click `Build Project` button (spanner icon) at VS Code extension.
6. Click `Flash Device` button (thunder icon) at VS Code extension.
7. Remove AtomS3U from PC
8. Connect AtomS3U to PC with pressing button (The following steps are required for the initial build)
9. Open and edit settings.txt
10. Close settings.txt
11. Unmount the USB device, wait about 5 seconds, and then unplug it. (On Windows, when it is unmounted, the LED blinks blue or green. Then unplug it.)



## Usages

### Settings
1. Connect AtomS3U to PC with pressing button
2. Open and edit settings.txt
3. Close settings.txt
4. Unmount the USB device, wait about 10 seconds, and then unplug it.

### Input key
1. Connect AtomS3U to PC
2. Wait until the LED lights up in green and then turns off.(If it blinks red, it indicates an error. Please check `settings.txt`.)
3. Press button to input key to PC.


## Examples of Modifier Key Input

* Shift:
    * Shift Key + A
    * `{"mod": "SHIFT", "key": "a"}`
* Control:
    * Ctrl Key + C 
    * `{"mod": "CTRL", "key": "c"}`
* Alt:
    * Alt Key + L
    * `{"mod": "ALT", "key": "l"}`
* Windows Key (Win/Cmd on Mac): 
    * Windows Key + D
    * `{"mod": "GUI", "key": "d"}`


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

* Login
```json
{
  "keys": [
    "<id or email>",
    {"key": "TAB"},
    "<password>",
    {"key": "ENTER"}
  ]
}
```

* Symbols / Punctuations
```json
{
  "keys": "!\"#$%&'()=-~^`@{}[]+;*:,.<>?/_|\\"
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

* Browser (windows / Linux)
```json
{
  "keys": [
    {"mod": "GUI"},
    "chrome",
    {"key": "ENTER"},
    {"key": "TAB"},
    {"key": "ENTER"},
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

* US Layout
```json
{
  "keys": [
    "[]@{}[]~"
  ],
  "layout": "us"
}
```



---

## erase rom
```bash
# check size of flash rom
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 flash_id

# remove 2MB-3MB
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_region 0x110000 0x100000

# remove 2MB-
$ esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_region 0x110000 0x6F0000
```

## hid settings

idf.py menuconfig under Component config > TinyUSB Stack > Human Interface Device Class (HID), setting TINYUSB_HID_COUNT to a value greater than 0 will enable TinyUSB HID.


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
