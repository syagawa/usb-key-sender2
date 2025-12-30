#include "class/hid/hid_device.h"

#ifndef HID_KEY_JIS_RO
#define HID_KEY_JIS_RO 0x87  // JIS配列の「ろ」 / アンダースコア
#endif

#ifndef REPORT_ID_KEYBOARD
#define REPORT_ID_KEYBOARD 1
#endif

static const char *TAG_KEYBOARD = "hid-keyboard";

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/**
 * @brief HID report descriptor
 *
 * In this example we implement Keyboard + Mouse HID device,
 * so we must define both report descriptors
 */
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
    // TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE))
};

/**
 * @brief String descriptor
 */
const char* hid_string_descriptor[5] = {
  // array of pointer to string descriptors
  (char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
  "TinyUSB",             // 1: Manufacturer
  "TinyUSB Device",      // 2: Product
  "123456",              // 3: Serials, should use chip ID
  "USE Key Sender2",  // 4: HID
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
  // Configuration number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
  TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

/********* Application ***************/

typedef enum {
    MOUSE_DIR_RIGHT,
    MOUSE_DIR_DOWN,
    MOUSE_DIR_LEFT,
    MOUSE_DIR_UP,
    MOUSE_DIR_MAX,
} mouse_dir_t;

#define DISTANCE_MAX        125
#define DELTA_SCALAR        5

// sample string User: ESP32-S3!\nPassword: Admin_123_|\\\n12345^~-=/?/.>,<_,______


const char* keyboard_layout_mode = "jis"; //jis, us
static void ascii_to_hid_with_modifier(char c, uint8_t *keycode, uint8_t *modifier) {
  *modifier = 0;
  *keycode = 0;

  if (c >= 'a' && c <= 'z') {
    *keycode = (c - 'a') + HID_KEY_A;
  } else if (c >= 'A' && c <= 'Z') {
    *keycode = (c - 'A') + HID_KEY_A;
    *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  } else if (c >= '1' && c <= '9') {
    *keycode = (c - '1') + HID_KEY_1;
  } else if (c == '0') {
    *keycode = HID_KEY_0;
  // 記号と特殊キー US
  } else {
    // US
    if(strcmp(keyboard_layout_mode, "us") == 0){
      switch (c) {
        case ' ':  *keycode = HID_KEY_SPACE; break;
        case '\n': *keycode = HID_KEY_ENTER; break;
        case '\t': *keycode = HID_KEY_TAB; break;
        case '-':  *keycode = HID_KEY_MINUS; break;
        case '=':  *keycode = HID_KEY_EQUAL; break;
        case '[':  *keycode = HID_KEY_BRACKET_LEFT; break;
        case ']':  *keycode = HID_KEY_BRACKET_RIGHT; break;
        case '\\': *keycode = HID_KEY_BACKSLASH; break;
        case ';':  *keycode = HID_KEY_SEMICOLON; break;
        case '\'': *keycode = HID_KEY_APOSTROPHE; break;
        case '`':  *keycode = HID_KEY_GRAVE; break;
        case ',':  *keycode = HID_KEY_COMMA; break;
        case '.':  *keycode = HID_KEY_PERIOD; break;
        case '/':  *keycode = HID_KEY_SLASH; break;

        // Shift
        case '!':  *keycode = HID_KEY_1; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '@':  *keycode = HID_KEY_2; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '#':  *keycode = HID_KEY_3; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '$':  *keycode = HID_KEY_4; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '%':  *keycode = HID_KEY_5; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '^':  *keycode = HID_KEY_6; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '&':  *keycode = HID_KEY_7; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '*':  *keycode = HID_KEY_8; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '(':  *keycode = HID_KEY_9; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case ')':  *keycode = HID_KEY_0; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '_':  *keycode = HID_KEY_MINUS; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '+':  *keycode = HID_KEY_EQUAL; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '{':  *keycode = HID_KEY_BRACKET_LEFT; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '}':  *keycode = HID_KEY_BRACKET_RIGHT; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '|':  *keycode = HID_KEY_BACKSLASH; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case ':':  *keycode = HID_KEY_SEMICOLON; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '\"': *keycode = HID_KEY_APOSTROPHE; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '~':  *keycode = HID_KEY_GRAVE; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '<':  *keycode = HID_KEY_COMMA; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '>':  *keycode = HID_KEY_PERIOD; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '?':  *keycode = HID_KEY_SLASH; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;

        default:   *keycode = 0; break; // 未対応
      }
    // JIS
    }else{
      switch (c) {
        case ' ':  *keycode = HID_KEY_SPACE; break;
        case '\n': *keycode = HID_KEY_ENTER; break;
        case '\t': *keycode = HID_KEY_TAB; break;
        
        case '-':  *keycode = HID_KEY_MINUS; break;
        case '^':  *keycode = HID_KEY_EQUAL; break;         // JIS ^
        case '@':  *keycode = HID_KEY_BRACKET_LEFT; break;  // JIS @
        case '[':  *keycode = HID_KEY_BRACKET_RIGHT; break; // JIS [
        case ';':  *keycode = HID_KEY_SEMICOLON; break;
        case ':':  *keycode = HID_KEY_APOSTROPHE; break;    // JIS :
        case ']':  *keycode = HID_KEY_BACKSLASH; break;     // JIS ]
        case ',':  *keycode = HID_KEY_COMMA; break;
        case '.':  *keycode = HID_KEY_PERIOD; break;
        case '/':  *keycode = HID_KEY_SLASH; break;
        case '\\': *keycode = HID_KEY_JIS_RO; break; // JIS ￥(変換が必要な場合あり)

        // Shift
        case '!':  *keycode = HID_KEY_1; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '\"': *keycode = HID_KEY_2; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '#':  *keycode = HID_KEY_3; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '$':  *keycode = HID_KEY_4; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '%':  *keycode = HID_KEY_5; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '&':  *keycode = HID_KEY_6; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '\'': *keycode = HID_KEY_7; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '(':  *keycode = HID_KEY_8; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case ')':  *keycode = HID_KEY_9; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '=':  *keycode = HID_KEY_MINUS; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '~':  *keycode = HID_KEY_EQUAL; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '`':  *keycode = HID_KEY_BRACKET_LEFT; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '{':  *keycode = HID_KEY_BRACKET_RIGHT; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '+':  *keycode = HID_KEY_SEMICOLON; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '*':  *keycode = HID_KEY_APOSTROPHE; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '}':  *keycode = HID_KEY_BACKSLASH; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '<':  *keycode = HID_KEY_COMMA; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '>':  *keycode = HID_KEY_PERIOD; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '?':  *keycode = HID_KEY_SLASH; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
        case '_':  *keycode = HID_KEY_JIS_RO; *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break; // JIS アンダーバー
        default:   *keycode = 0; break;
      }
    }
  }
}


void usb_hid_print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    ESP_LOGI(TAG_KEYBOARD, "in usb_hid_print_string1");
    uint8_t keycode = 0;
    uint8_t modifier = 0;
    ascii_to_hid_with_modifier(str[i], &keycode, &modifier);
    if (keycode != 0) {
      uint8_t key_report[6] = {keycode, 0, 0, 0, 0, 0};
      uint8_t empty_report[6] = {0, 0, 0, 0, 0, 0};
      // キーを押す
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, key_report);
      vTaskDelay(pdMS_TO_TICKS(20));

      while (!tud_hid_ready()) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }

      // キーを離す
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, empty_report);
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  }
}
