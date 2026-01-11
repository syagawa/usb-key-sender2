#include <stdio.h>
#include <stdlib.h>
#include "esp_random.h"
#include "esp_system.h"

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



bool getRandomStrFromRange(const char *range_str, char *out_str, size_t out_size) {
  int min, max;

  if (sscanf(range_str, "%d-%d", &min, &max) != 2 || min > max) {
    return false;
  }

  uint32_t span = (uint32_t)(max - min + 1);
  uint32_t r, limit = (UINT32_MAX / span) * span;

  do {
      r = esp_random();
  } while (r >= limit);

  int value = min + (r % span);
  snprintf(out_str, out_size, "%d", value);

  return true;
}

// char result[8];

// if (getRandomStrFromRange("0-13", result, sizeof(result))) {
//   printf("random string = %s\n", result);
// }