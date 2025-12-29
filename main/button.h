#ifndef BUTTON_H
#define BUTTON_H
#include "iot_button.h"
#include "storage.h"


void (*singleClickAction)(void);
void (*pressUpAction)(void);
void (*longPressedAction)(void);


static void button_long_cb(void *arg, void *data) {
  longPressedAction();
}

static void button_press_up_cb(void *arg, void *data) {
  pressUpAction();
}

static void button_single_click_cb(void *arg,void *usr_data) {
  singleClickAction();
}


static void setButtonColor(char * color) {
  buttonColor = color;
}

char * getButtonColor(){
  if(strcmp(buttonColor, "") != 0) {
    return buttonColor;
  }
  return defaultButtonColor;
}

bool isButtonPressed(void){
  ESP_LOGI(TAG, "in isButtonPressed");
  return gpio_get_level(GPIOButtonNumber) == 0;
}

static void initButtonForKeyboard(void) {
  const gpio_config_t boot_button_config = {
    .pin_bit_mask = BIT64(GPIOButtonNumber),
    .mode = GPIO_MODE_INPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_up_en = true,
    .pull_down_en = false,
  };
  ESP_ERROR_CHECK(gpio_config(&boot_button_config));

  ESP_LOGI(TAG, "USB initialization");
  const tinyusb_config_t tusb_cfg = {
    .device_descriptor = NULL,
    .string_descriptor = hid_string_descriptor,
    .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
    .external_phy = false,
    .configuration_descriptor = hid_configuration_descriptor,
  };

  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
  ESP_LOGI(TAG, "USB initialization DONE");

  // create gpio button
  button_config_t gpio_btn_cfg = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = waitingMS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = GPIOButtonNumber,
        .active_level = 0,
    },
  };

  button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);

  if (gpio_btn == NULL) {
    ESP_LOGE(TAG, "Button create failed");
  }
  iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);
  iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, button_long_cb,NULL);
  iot_button_register_cb(gpio_btn, BUTTON_PRESS_UP, button_press_up_cb,NULL);
}

#endif // BUTTON_H
