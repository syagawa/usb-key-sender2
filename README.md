# USB Key Sender2

## specs

* M5AtomS3U
https://docs.m5stack.com/en/core/AtomS3U


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

## build

### initial build
1. long press reset button
2. click build button (spanner icon) at extension.
3. remove usb and connect
4. long press reset button
  * 
  ```bash
  #clear rom
  $ esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_region 0x110000 0x100000
  ```
5. remove usb and connect usb with button pressing
6. edit settings 
7. remove removal device at pc


### reset
```bash
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
