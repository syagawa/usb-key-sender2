#include <stdio.h>
#include "esp_random.h"

typedef struct {
  char out[37];
} uuid_string_t;

uuid_string_t generate_v4_uuid() {
  uint8_t u[16];
  uuid_string_t result;

  esp_fill_random(u, 16);
  u[6] = (u[6] & 0x0f) | 0x40; // Version 4
  u[8] = (u[8] & 0x3f) | 0x80; // Variant

  snprintf(result.out, sizeof(result.out),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
    u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);

  return result;
}

// void app_main(void) {
  // uuid_string_t my_id = generate_v4_uuid();
  // printf("ID: %s\n", my_id.out);
// }