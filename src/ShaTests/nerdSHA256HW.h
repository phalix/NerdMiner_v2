#ifndef nerdSHA256_H_HWCRYPT_
#define nerdSHA256_H_HWCRYPT_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* Calculate midstate */
IRAM_ATTR int nerd_midstate_hwcrypt(uint8_t* data, uint32_t len);

//IRAM_ATTR int nerd_double_sha(nerd_sha256* midstate, uint8_t* data, uint8_t* doubleHash);

IRAM_ATTR int nerd_double_sha2_hwcrypt(uint8_t* dataIn, uint8_t* doubleHash);

#endif /* nerdSHA256_H_HWCRYPT_ */