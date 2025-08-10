#ifndef nerdSHA256_H_HWCRYPT_
#define nerdSHA256_H_HWCRYPT_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



IRAM_ATTR int nerd_double_sha2_hw(uint8_t* dataIn, uint8_t* doubleHash);

#endif /* nerdSHA256_H_HW_ */