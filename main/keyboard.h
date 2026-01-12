#include "class/hid/hid_device.h"
#include "random.h"

#ifndef HID_KEY_JIS_RO
#define HID_KEY_JIS_RO 0x87  // JIS配列の「ろ」 / アンダースコア
#endif

#ifndef REPORT_ID_KEYBOARD
#define REPORT_ID_KEYBOARD 1
#endif

// keyboard mappings
// https://github.com/espressif/tinyusb/blob/c64589db1a2af04a6726f03744999d00fa70c892/src/class/hid/hid.h

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

static void send_hid_report_and_wait(uint8_t modifier, uint8_t keycode) {
  if (keycode == 0 && modifier == 0) return;

  uint8_t key_report[6] = {keycode, 0, 0, 0, 0, 0};
  uint8_t empty_report[6] = {0, 0, 0, 0, 0, 0};

  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, key_report);
  vTaskDelay(pdMS_TO_TICKS(20));

  while (!tud_hid_ready()) {
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, empty_report);
  vTaskDelay(pdMS_TO_TICKS(20));
  while (!tud_hid_ready()) {
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}


void usb_hid_print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    uint8_t keycode = 0;
    uint8_t modifier = 0;
    ascii_to_hid_with_modifier(str[i], &keycode, &modifier);
    if (keycode != 0) {
      send_hid_report_and_wait(modifier, keycode);
    }
  }
}



uint8_t getHidKeycodeFromStr(const char *str) {
  if(str == NULL) {
    return 0;
  }

  if(strcmp(str, "ENTER") == 0){
    return HID_KEY_ENTER;
  }else if(strcmp(str, "ESCAPE") == 0){
    return HID_KEY_ESCAPE;
  }else if(strcmp(str, "BACKSPACE") == 0){
    return HID_KEY_BACKSPACE;
  }else if(strcmp(str, "TAB") == 0){
    return HID_KEY_TAB;
  }else if(strcmp(str, "SPACE") == 0){
    return HID_KEY_SPACE;
  }else if(strcmp(str, "MINUS") == 0){
    return HID_KEY_MINUS;
  }else if(strcmp(str, "EQUAL") == 0){
    return HID_KEY_EQUAL;
  }else if(strcmp(str, "BRACKET_LEFT") == 0){
    return HID_KEY_BRACKET_LEFT;
  }else if(strcmp(str, "BRACKET_RIGHT") == 0){
    return HID_KEY_BRACKET_RIGHT;
  }else if(strcmp(str, "BACKSLASH") == 0){
    return HID_KEY_BACKSLASH;
  }else if(strcmp(str, "EUROPE_1") == 0){
    return HID_KEY_EUROPE_1;
  }else if(strcmp(str, "SEMICOLON") == 0){
    return HID_KEY_SEMICOLON;
  }else if(strcmp(str, "APOSTROPHE") == 0){
    return HID_KEY_APOSTROPHE;
  }else if(strcmp(str, "GRAVE") == 0){
    return HID_KEY_GRAVE;
  }else if(strcmp(str, "COMMA") == 0){
    return HID_KEY_COMMA;
  }else if(strcmp(str, "PERIOD") == 0){
    return HID_KEY_PERIOD;
  }else if(strcmp(str, "SLASH") == 0){
    return HID_KEY_SLASH;
  }else if(strcmp(str, "CAPS_LOCK") == 0){
    return HID_KEY_CAPS_LOCK;
  }else if(strcmp(str, "F1") == 0){
    return HID_KEY_F1;
  }else if(strcmp(str, "F2") == 0){
    return HID_KEY_F2;
  }else if(strcmp(str, "F3") == 0){
    return HID_KEY_F3;
  }else if(strcmp(str, "F4") == 0){
    return HID_KEY_F4;
  }else if(strcmp(str, "F5") == 0){
    return HID_KEY_F5;
  }else if(strcmp(str, "F6") == 0){
    return HID_KEY_F6;
  }else if(strcmp(str, "F7") == 0){
    return HID_KEY_F7;
  }else if(strcmp(str, "F8") == 0){
    return HID_KEY_F8;
  }else if(strcmp(str, "F9") == 0){
    return HID_KEY_F9;
  }else if(strcmp(str, "F10") == 0){
    return HID_KEY_F10;
  }else if(strcmp(str, "F11") == 0){
    return HID_KEY_F11;
  }else if(strcmp(str, "F12") == 0){
    return HID_KEY_F12;
  }else if(strcmp(str, "PRINT_SCREEN") == 0){
    return HID_KEY_PRINT_SCREEN;
  }else if(strcmp(str, "SCROLL_LOCK") == 0){
    return HID_KEY_SCROLL_LOCK;
  }else if(strcmp(str, "PAUSE") == 0){
    return HID_KEY_PAUSE;
  }else if(strcmp(str, "INSERT") == 0){
    return HID_KEY_INSERT;
  }else if(strcmp(str, "HOME") == 0){
    return HID_KEY_HOME;
  }else if(strcmp(str, "PAGE_UP") == 0){
    return HID_KEY_PAGE_UP;
  }else if(strcmp(str, "DELETE") == 0){
    return HID_KEY_DELETE;
  }else if(strcmp(str, "END") == 0){
    return HID_KEY_END;
  }else if(strcmp(str, "PAGE_DOWN") == 0){
    return HID_KEY_PAGE_DOWN;
  }else if(strcmp(str, "ARROW_RIGHT") == 0){
    return HID_KEY_ARROW_RIGHT;
  }else if(strcmp(str, "ARROW_LEFT") == 0){
    return HID_KEY_ARROW_LEFT;
  }else if(strcmp(str, "ARROW_DOWN") == 0){
    return HID_KEY_ARROW_DOWN;
  }else if(strcmp(str, "ARROW_UP") == 0){
    return HID_KEY_ARROW_UP;
  }else if(strcmp(str, "NUM_LOCK") == 0){
    return HID_KEY_NUM_LOCK;
  }else if(strcmp(str, "KEYPAD_DIVIDE") == 0){
    return HID_KEY_KEYPAD_DIVIDE;
  }else if(strcmp(str, "KEYPAD_MULTIPLY") == 0){
    return HID_KEY_KEYPAD_MULTIPLY;
  }else if(strcmp(str, "KEYPAD_SUBTRACT") == 0){
    return HID_KEY_KEYPAD_SUBTRACT;
  }else if(strcmp(str, "KEYPAD_ADD") == 0){
    return HID_KEY_KEYPAD_ADD;
  }else if(strcmp(str, "KEYPAD_ENTER") == 0){
    return HID_KEY_KEYPAD_ENTER;
  }else if(strcmp(str, "KEYPAD_1") == 0){
    return HID_KEY_KEYPAD_1;
  }else if(strcmp(str, "KEYPAD_2") == 0){
    return HID_KEY_KEYPAD_2;
  }else if(strcmp(str, "KEYPAD_3") == 0){
    return HID_KEY_KEYPAD_3;
  }else if(strcmp(str, "KEYPAD_4") == 0){
    return HID_KEY_KEYPAD_4;
  }else if(strcmp(str, "KEYPAD_5") == 0){
    return HID_KEY_KEYPAD_5;
  }else if(strcmp(str, "KEYPAD_6") == 0){
    return HID_KEY_KEYPAD_6;
  }else if(strcmp(str, "KEYPAD_7") == 0){
    return HID_KEY_KEYPAD_7;
  }else if(strcmp(str, "KEYPAD_8") == 0){
    return HID_KEY_KEYPAD_8;
  }else if(strcmp(str, "KEYPAD_9") == 0){
    return HID_KEY_KEYPAD_9;
  }else if(strcmp(str, "KEYPAD_0") == 0){
    return HID_KEY_KEYPAD_0;
  }else if(strcmp(str, "KEYPAD_DECIMAL") == 0){
    return HID_KEY_KEYPAD_DECIMAL;
  }else if(strcmp(str, "EUROPE_2") == 0){
    return HID_KEY_EUROPE_2;
  }else if(strcmp(str, "APPLICATION") == 0){
    return HID_KEY_APPLICATION;
  }else if(strcmp(str, "POWER") == 0){
    return HID_KEY_POWER;
  }else if(strcmp(str, "KEYPAD_EQUAL") == 0){
    return HID_KEY_KEYPAD_EQUAL;
  }else if(strcmp(str, "F13") == 0){
    return HID_KEY_F13;
  }else if(strcmp(str, "F14") == 0){
    return HID_KEY_F14;
  }else if(strcmp(str, "F15") == 0){
    return HID_KEY_F15;
  }else if(strcmp(str, "F16") == 0){
    return HID_KEY_F16;
  }else if(strcmp(str, "F17") == 0){
    return HID_KEY_F17;
  }else if(strcmp(str, "F18") == 0){
    return HID_KEY_F18;
  }else if(strcmp(str, "F19") == 0){
    return HID_KEY_F19;
  }else if(strcmp(str, "F20") == 0){
    return HID_KEY_F20;
  }else if(strcmp(str, "F21") == 0){
    return HID_KEY_F21;
  }else if(strcmp(str, "F22") == 0){
    return HID_KEY_F22;
  }else if(strcmp(str, "F23") == 0){
    return HID_KEY_F23;
  }else if(strcmp(str, "F24") == 0){
    return HID_KEY_F24;
  }else if(strcmp(str, "EXECUTE") == 0){
    return HID_KEY_EXECUTE;
  }else if(strcmp(str, "HELP") == 0){
    return HID_KEY_HELP;
  }else if(strcmp(str, "MENU") == 0){
    return HID_KEY_MENU;
  }else if(strcmp(str, "SELECT") == 0){
    return HID_KEY_SELECT;
  }else if(strcmp(str, "STOP") == 0){
    return HID_KEY_STOP;
  }else if(strcmp(str, "AGAIN") == 0){
    return HID_KEY_AGAIN;
  }else if(strcmp(str, "UNDO") == 0){
    return HID_KEY_UNDO;
  }else if(strcmp(str, "CUT") == 0){
    return HID_KEY_CUT;
  }else if(strcmp(str, "COPY") == 0){
    return HID_KEY_COPY;
  }else if(strcmp(str, "PASTE") == 0){
    return HID_KEY_PASTE;
  }else if(strcmp(str, "FIND") == 0){
    return HID_KEY_FIND;
  }else if(strcmp(str, "MUTE") == 0){
    return HID_KEY_MUTE;
  }else if(strcmp(str, "VOLUME_UP") == 0){
    return HID_KEY_VOLUME_UP;
  }else if(strcmp(str, "VOLUME_DOWN") == 0){
    return HID_KEY_VOLUME_DOWN;
  }else if(strcmp(str, "LOCKING_CAPS_LOCK") == 0){
    return HID_KEY_LOCKING_CAPS_LOCK;
  }else if(strcmp(str, "LOCKING_NUM_LOCK") == 0){
    return HID_KEY_LOCKING_NUM_LOCK;
  }else if(strcmp(str, "LOCKING_SCROLL_LOCK") == 0){
    return HID_KEY_LOCKING_SCROLL_LOCK;
  }else if(strcmp(str, "KEYPAD_COMMA") == 0){
    return HID_KEY_KEYPAD_COMMA;
  }else if(strcmp(str, "KEYPAD_EQUAL_SIGN") == 0){
    return HID_KEY_KEYPAD_EQUAL_SIGN;
  }

  if (strlen(str) == 1) {
    uint8_t keycode = 0;
    uint8_t dummy_mod;
    ascii_to_hid_with_modifier(str[0], &keycode, &dummy_mod);
    return keycode;
  }

  return 0;
}

// [
//   "notepad", 
//   {"key": "ENTER"},
//   "Hello World",
//   {"mod": "L_CTRL", "key": "s"}
// ]

void executeAction(cJSON **keys, int index) {
  if (keys == NULL || keys[index] == NULL) {
    return;
  }
  cJSON *item = keys[index];
  if (cJSON_IsString(item)) {
    usb_hid_print_string(item->valuestring);
  } else if (cJSON_IsObject(item)) {
    cJSON *t_obj = cJSON_GetObjectItemCaseSensitive(item, "token");

    if(cJSON_IsString(t_obj)){
      const char *t = t_obj->valuestring;
      if(strcmp(t, "UUID") == 0){
        uuid_string_t id = generateV4UUID();
        usb_hid_print_string(id.out);
      }else if(strcmp(t, "NUMBER") == 0){
        cJSON *r_obj = cJSON_GetObjectItemCaseSensitive(item, "range");
        char result[8];
        if(cJSON_IsString(r_obj)){
          const char *range = r_obj->valuestring;
          getRandomStrFromRange(range, result, sizeof(result));
        }else{
          const char *range = "0-9";
          getRandomStrFromRange(range, result, sizeof(result));
        }
        usb_hid_print_string(result);
      }else{

      }
    }else{

      uint8_t keycode = 0, modifier = 0;
      cJSON *k_obj = cJSON_GetObjectItemCaseSensitive(item, "key");
      cJSON *m_obj = cJSON_GetObjectItemCaseSensitive(item, "mod");
  
      // 修飾キーの解析
      if (cJSON_IsString(m_obj)) {
        const char *m = m_obj->valuestring;
        if      (strcmp(m, "CTRL")  == 0) modifier = KEYBOARD_MODIFIER_LEFTCTRL;
        else if (strcmp(m, "SHIFT") == 0) modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        else if (strcmp(m, "ALT") == 0)   modifier = KEYBOARD_MODIFIER_LEFTALT;
        else if (strcmp(m, "GUI") == 0)   modifier = KEYBOARD_MODIFIER_LEFTGUI;
      }
  
      // キーコードの解析
      if (cJSON_IsString(k_obj)) {
        const char *k = k_obj->valuestring;
        keycode = getHidKeycodeFromStr(k);
      }
  
      // 共通関数を呼び出し
      send_hid_report_and_wait(modifier, keycode);
    }


  }
}