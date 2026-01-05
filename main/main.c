/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_partition.h"
#include "esp_check.h"
#include "tinyusb.h"
#include "tusb_msc_storage.h"
#include "tusb_cdc_acm.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

// #include "iot_button.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

// #include "jsmn.h"
#include "cJSON.h"

#define VERSION "2.1.1"
#define waitingMS 1000
#define GPIOButtonNumber 41
#define MaxLength 10


#include "led.h"
#include "keyboard.h"
#include "button.h"
#include "storage.h"

const char * initialDataStr = "{\"keys\": [\"sample\", \"sample2\", \"example-111\"]}";
static const char * readmeStr = "usb-key-sender-"VERSION"\nReset Settings: Delete settings.txt and remove this USB from PC.";

int keyIndex = 0;
cJSON *keys[MaxLength];
int array_keys_count = 0;
const char *colors[] = {"RED", "BLUE", "MAGENTA", "GREEN", "PINK", "YELLOW", "SKYBLUE", "BROWN", "PURPLE"};
const int colorsLength = sizeof(colors) / sizeof(colors[0]);
int colorIndex = -1;

int pressedCount = 0;
bool buttonIsLongPressed = false;
TickType_t lastIncrementTime = 0;

static void setIndex(int c) {
  keyIndex = c;
  if(keyIndex >= array_keys_count){
    keyIndex = 0;
    pressedCount = 0;
    colorIndex = -1;
  }else{
    colorIndex = keyIndex % colorsLength;
  }

}

static void startCount() {
  buttonIsLongPressed = true;
  lastIncrementTime = xTaskGetTickCount();
  pressedCount++;
  setIndex(pressedCount);
}

static void incrementCount(){
  pressedCount++;
  setIndex(pressedCount);
}

static void checkAndIncrementCount() {
  TickType_t current = xTaskGetTickCount();
  if ((current - lastIncrementTime) >= pdMS_TO_TICKS(waitingMS)){
    incrementCount();
    lastIncrementTime = xTaskGetTickCount();
  }
}

static void checkAndSetColor() {

  if(colorIndex == -1){
    offLed();
    return;
  }

  char *s = colors[colorIndex];
  lightLed(s);
}




static void action1(void *arg,void *usr_data) {

  // char *str = keys[keyIndex];
  // usb_hid_print_string(str);
  executeAction(keys, keyIndex);

  incrementCount();
}

static void action2(void *arg, void *data) {
  buttonIsLongPressed = false;
}

static void action3(void *arg, void *data) {
  // ESP_LOGI(TAG, "button_long_cb %d", pressedCount);
  startCount();
}


void enterSettingsMode(){
  lightLed("white");
  startSettingsMode();
  while(1){
    vTaskDelay(pdMS_TO_TICKS(500));

    // for windows
    if(!tud_mounted()){
      lightLed("greenyellow");
      vTaskDelay(pdMS_TO_TICKS(500));
      offLed();
    }else if(tud_suspended()){
      lightLed("cyan");
      vTaskDelay(pdMS_TO_TICKS(500));
      offLed();
    }

  }
}

void enterMain(){

  singleClickAction = action1;
  pressUpAction = action2;
  longPressedAction = action3;

  initSettings(readmeStr, initialDataStr);

  char* layout = getSettingStrByKeyRequireFree("layout");
  if (layout != NULL) {
      if (strcmp(layout, "us") == 0) {
        keyboard_layout_mode = "us";
      }
      free(layout);
  }

  int parseError = 0;
  cJSON *json_arr = getSettingArrayAsJSONByKey("keys");
  if (cJSON_IsArray(json_arr)){
    int size = cJSON_GetArraySize(json_arr);
    for (int i = 0; i < size && i < MaxLength; i++) {
      cJSON *item = cJSON_GetArrayItem(json_arr, i);
      cJSON *dup = cJSON_Duplicate(item, true);
      if(dup == NULL){
        parseError = 2;
        break;
      }
      keys[array_keys_count] = dup;
      array_keys_count++;
    }
    cJSON_Delete(json_arr);
  }else{
    parseError = 1;
  }

  if(parseError > 0){
    for(int i = 0; i < parseError; i++){
      lightLed("red");
      vTaskDelay(pdMS_TO_TICKS(400));
      offLed();
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }else{
    lightLed("green");
  }

  vTaskDelay(pdMS_TO_TICKS(500));
  offLed();

  while(1){
    if(buttonIsLongPressed){
      checkAndIncrementCount();
    }
    checkAndSetColor();

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void app_main(void){

  esp_reset_reason_t reason = esp_reset_reason();

  if(reason == 3){
    // ESP_LOGI(TAG, "restarted esp");
    initLed();
    lightLed("green");
    initSettings(readmeStr, initialDataStr);
    enterSettingsMode();
    // settings mode
  }else{
    if(isButtonPressed()){
      esp_restart();
    }else{
      initButtonForKeyboard();
      initLed();
      // ESP_LOGI(TAG, "normal");
      enterMain();
    }
  }
}
