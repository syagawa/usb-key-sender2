
#ifndef STORAGE_H
#define STORAGE_H
// #include "tinyusb.h"
// #include "tusb_msc_storage.h"
// #include "tusb_cdc_acm.h"
// #include "esp_log.h"
// #include "sdkconfig.h"
// #include <stdlib.h>
// #include <time.h>
// #include <string.h>
// #include "cJSON.h"

#define BASE_PATH "/usb" // base path to mount the partition

static const char *TAG = "msc-storage";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

const char *directory = "/usb/esp";
const char *file_path = "/usb/esp/settings.txt";

const char *file_path_readme = "/usb/esp/readme.txt";

static const tusb_desc_device_t msc_device_descriptor = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = 0x00,        // MSCはインターフェースで指定
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor           = 0x303A,
  .idProduct          = 0x4002,      // MSC用とわかるID（HIDとは変える）
  .bcdDevice          = 0x0100,
  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,
  .bNumConfigurations = 0x01
};

static const uint8_t msc_configuration_descriptor[] = {
  // 構成記述子のヘッダー (Configuration Descriptor)
  0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xC0, 0x32,
  // インターフェース記述子 (Interface Descriptor)
  0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00,
  // エンドポイント記述子 (Bulk In)
  0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
  // エンドポイント記述子 (Bulk Out)
  0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00
};

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
  /* initialization */
  size_t rx_size = 0;

  /* read */
  esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Data from channel %d:", itf);
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
  } else {
    ESP_LOGE(TAG, "Read error");
  }

  /* write back */
  tinyusb_cdcacm_write_queue(itf, buf, rx_size);
  tinyusb_cdcacm_write_flush(itf, 0);
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
  int dtr = event->line_state_changed_data.dtr;
  int rts = event->line_state_changed_data.rts;
  ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

static bool exists(const char *path) {
  struct stat buffer;
  return stat(path, &buffer) == 0;
}


static esp_err_t storage_init_spiflash(wl_handle_t *wl_handle)
{
  ESP_LOGI(TAG, "Initializing wear levelling");
  const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
  if (data_partition == NULL) {
    ESP_LOGE(TAG, "Failed to find FATFS partition. Check the partition table.");
    return ESP_ERR_NOT_FOUND;
  }
  return wl_mount(data_partition, wl_handle);
}


static void removeFiles(void){
  fclose(file_path);
  remove(file_path);
}


void initSettings(char * readmeStr, char * initialDataStr){
  wl_handle_t wl_handle = WL_INVALID_HANDLE;
  ESP_ERROR_CHECK(storage_init_spiflash(&wl_handle));
  const tinyusb_msc_spiflash_config_t config_spi = {
    .wl_handle = wl_handle
  };
  ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&config_spi));
  ESP_ERROR_CHECK(tinyusb_msc_storage_mount(BASE_PATH));

  struct stat s = {0};
  if(!exists(directory)){
    if (mkdir(directory, 0775) != 0) {
      ESP_LOGE(TAG, "mkdir failed with errno: %s", strerror(errno));
    }else{
      ESP_LOGI(TAG, "directory created: %s", directory);
    }
  }

  if (!exists(file_path)) {
    FILE *f = fopen(file_path, "w");
    if(f){
      fputs(initialDataStr, f);
      // f_sync(fileno(f));
      fclose(f);
    }else{
      ESP_LOGE(TAG, "Failed to open file for writing");
    }
  }

  if (!exists(file_path_readme)) {
    FILE *f2 = fopen(file_path_readme, "w");
    if(f2){
      fputs(readmeStr, f2);
      // f_sync(fileno(f2));
      fclose(f2);
    }else{
      ESP_LOGE(TAG, "Failed to open file for writing");
    }
  }
}

cJSON * getSettings(){
  FILE *f;
  ESP_LOGI(TAG, "Reading file");
  f = fopen(file_path, "r");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return NULL;
  }

  char line[128];
  if(fgets(line, sizeof(line), f) == NULL){
    fclose(f);
    return NULL;
  }
  fclose(f);
  // strip newline
  char *pos = strchr(line, '\n');
  if (pos) {
      *pos = '\0';
  }
  ESP_LOGI(TAG, "Read from file: '%s'", line);

  char * str = strdup(line);

  ESP_LOGI(TAG, "json_str '%s'", str);
  cJSON * obj = cJSON_Parse(str);
  free(str);

  return obj;
}

char * getSettingByKey(char * targetkey){
  ESP_LOGI(TAG, "in getSettingByKey0");
  cJSON * obj = getSettings();
  ESP_LOGI(TAG, "in getSettingByKey1");
  char * value = "";
  if(obj != NULL){
    // Iteratively check for existing keys
    cJSON *currentItem = obj->child;
    while (currentItem != NULL) {
      const char *key = currentItem->string;
      cJSON *value = currentItem;
      if (cJSON_IsString(value) && (value->valuestring != NULL)) {
        ESP_LOGI(TAG, "Key: %s, Value: %s\n", key, value->valuestring);
      } else {
        ESP_LOGE(TAG, "Error getting value for key: %s\n", key);
      }
      currentItem = currentItem->next;
    }
    cJSON *target_elm = cJSON_GetObjectItemCaseSensitive(obj, targetkey);
    if(target_elm){
      value = target_elm->valuestring;
    }
  }

  return value;
}

cJSON * getSettingArrayAsJSONByKey(char * targetkey) {
  cJSON * obj = getSettings();
  if (obj == NULL) return NULL;
  cJSON *target_elm = cJSON_GetObjectItemCaseSensitive(obj, targetkey);

  cJSON *result = NULL;
  if (cJSON_IsArray(target_elm)) {
    result = cJSON_Duplicate(target_elm, true);
  }

  cJSON_Delete(obj);
  return result;
}

void startSettingsMode(){
  cJSON * obj = getSettings();

  char * settings_mode = "storage";
  if(obj != NULL){
    // Iteratively check for existing keys
    cJSON *currentItem = obj->child;
    while (currentItem != NULL) {
      const char *key = currentItem->string;
      cJSON *value = currentItem;
      if (cJSON_IsString(value) && (value->valuestring != NULL)) {
        ESP_LOGI(TAG, "Key: %s, Value: %s\n", key, value->valuestring);
      } else {
        ESP_LOGE(TAG, "Error getting value for key: %s\n", key);
      }
      currentItem = currentItem->next;
    }
    cJSON *settings_mode_elm = cJSON_GetObjectItemCaseSensitive(obj, "settings_mode");
    if(settings_mode_elm){
      settings_mode = settings_mode_elm->valuestring;
    }
  }

  ESP_LOGI(TAG, "Settings Mode is %s\n", settings_mode);

  if(strcmp("storage", settings_mode) == 0){
    showColorWithBrightness("white", 0.1);
    ESP_LOGI(TAG, "USB Composite initialization");
    const tinyusb_config_t tusb_cfg = {
      .device_descriptor = &msc_device_descriptor,
      .string_descriptor = NULL,
      .string_descriptor_count = 0,
      .external_phy = false,
      .configuration_descriptor = msc_configuration_descriptor,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
      .usb_dev = TINYUSB_USBDEV_0,
      .cdc_port = TINYUSB_CDC_ACM_0,
      .rx_unread_buf_sz = 64,
      .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = NULL,
      .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    /* the second way to register a callback */
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_0,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));

    ESP_LOGI(TAG, "USB Composite initialization DONE");
  }

  cJSON_Delete(obj);

}

#endif // STORAGE_H