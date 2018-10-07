#include <stdint.h>

#define SZ(type,size) (sizeof(type) == size ? 1 : -1)

char __sanity_check_int8[SZ(int8_t, 1)];
char __sanity_check_int16[SZ(int16_t, 2)];
char __sanity_check_int32[SZ(int32_t, 4)];
char __sanity_check_int64[SZ(int64_t, 8)];

char __sanity_check_uint8[SZ(uint8_t, 1)];
char __sanity_check_uint16[SZ(uint16_t, 2)];
char __sanity_check_uint32[SZ(uint32_t, 4)];
char __sanity_check_uint64[SZ(uint64_t, 8)];

char __sanity_check_intptr[SZ(intptr_t, sizeof(void *))];
char __sanity_check_uintptr[SZ(uintptr_t, sizeof(void *))];
