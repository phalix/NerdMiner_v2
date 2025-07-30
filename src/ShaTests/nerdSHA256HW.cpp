#define NDEBUG


#include <stdio.h>
#include <string.h>
#include <Arduino.h>


#include "nerdSHA256HW.h"


#if SOC_SHA_SUPPORT_PARALLEL_ENG
#include "sha/sha_parallel_engine.h"
#elif SOC_SHA_SUPPORT_DMA
#include "sha/sha_dma.h"
#endif


#include <mbedtls/sha256.h>

#include "hal/sha_hal.h"
#include "hal/sha_types.h"
#include "hal/sha_ll.h"

int counter = 0;
void esp_sha_unlock_engine1(esp_sha_type sha_type)
{
    //esp_sha_unlock_engine(sha_type);
}

bool esp_sha_try_lock_engine1(esp_sha_type sha_type)
{
    //esp_sha_try_lock_engine(sha_type);
    return true;
}


IRAM_ATTR int nerd_midstate_hwcrypt(uint8_t* data, uint32_t len)
{
    return 0;
}


void print_hash(const uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i++) {
        Serial.printf("%02x", hash[i]);
    }
    Serial.println();
}


static SemaphoreHandle_t engine_states[3];


static SemaphoreHandle_t sha_get_engine_state(esp_sha_type sha_type)
{
    unsigned idx = 1; //SHA2_256
    volatile SemaphoreHandle_t *engine = &engine_states[idx];
    SemaphoreHandle_t result = *engine;
    uint32_t set_engine = 0;

    if (result == NULL) {
        // Create a new semaphore for 'in use' flag
        SemaphoreHandle_t new_engine = xSemaphoreCreateBinary();
        assert(new_engine != NULL);
        xSemaphoreGive(new_engine); // start available

        // try to atomically set the previously NULL *engine to new_engine
        set_engine = (uint32_t)new_engine;
        uxPortCompareSet((volatile uint32_t *)engine, 0, &set_engine);

        if (set_engine != 0) { // we lost a race setting *engine
            vSemaphoreDelete(new_engine);
        }
        result = *engine;
    }
    return result;
}



void esp_sha_block1(esp_sha_type sha_type, const void *data_block, bool first_block)
{
    
    #ifndef NDEBUG
        {
            SemaphoreHandle_t engine_state = sha_get_engine_state(sha_type);
            assert(uxSemaphoreGetCount(engine_state) == 0 &&
                "SHA engine should be locked" );
        }
    #endif

        // preemptively do this before entering the critical section, then re-check once in it
        sha_hal_wait_idle();
        //esp_sha_lock_memory_block();

        //sha_hal_hash_block(sha_type, data_block, 64 / 4, first_block);
        sha_ll_fill_text_block(data_block, 64 / 4);
        if (first_block) {
            sha_ll_start_block(sha_type);
        } else {
            sha_ll_continue_block(sha_type);
        }

        //esp_sha_unlock_memory_block();

}



int mbedtls_internal_sha256_process1( mbedtls_sha256_context *ctx, const unsigned char data[64] )
{
    bool first_block = false;
    //esp_sha_block1(SHA2_256, data, first_block);
    
    if (ctx->mode == ESP_MBEDTLS_SHA256_UNUSED) {
        ctx->mode = ESP_MBEDTLS_SHA256_HARDWARE;
        first_block = true;
        /*if (!ctx->is224 && esp_sha_try_lock_engine1(SHA2_256)) {
            ctx->mode = ESP_MBEDTLS_SHA256_HARDWARE;
            first_block = true;
        } else {
            ctx->mode = ESP_MBEDTLS_SHA256_SOFTWARE;
        }*/
    }
    esp_sha_block1(SHA2_256, data, first_block);
    /*if (ctx->mode == ESP_MBEDTLS_SHA256_HARDWARE) {
        esp_sha_block1(SHA2_256, data, first_block);
    } else {
        //mbedtls_sha256_software_process(ctx, data);
        return -1;
    }*/

    return 0;

}


int mbedtls_sha256_update_ret1( mbedtls_sha256_context *ctx, const unsigned char *input,
                               size_t ilen )
{
    int ret;
    size_t fill;
    uint32_t left;

    if ( ilen == 0 ) {
        return 0;
    }

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (uint32_t) ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if ( ctx->total[0] < (uint32_t) ilen ) {
        ctx->total[1]++;
    }

    if ( left && ilen >= fill ) {
        memcpy( (void *) (ctx->buffer + left), input, fill );

        if ( ( ret = mbedtls_internal_sha256_process1( ctx, ctx->buffer ) ) != 0 ) {
            return ret;
        }

        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while ( ilen >= 64 ) {
        if ( ( ret = mbedtls_internal_sha256_process1( ctx, input ) ) != 0 ) {
            return ret;
        }

        input += 64;
        ilen  -= 64;
    }

    if ( ilen > 0 ) {
        memcpy( (void *) (ctx->buffer + left), input, ilen );
    }

    return 0;
}

boolean initialized = false;



int mbedtls_sha256_starts_ret1( mbedtls_sha256_context *ctx, int is224 )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if ( is224 == 0 ) {
        /* SHA-256 */
        ctx->state[0] = 0x6A09E667;
        ctx->state[1] = 0xBB67AE85;
        ctx->state[2] = 0x3C6EF372;
        ctx->state[3] = 0xA54FF53A;
        ctx->state[4] = 0x510E527F;
        ctx->state[5] = 0x9B05688C;
        ctx->state[6] = 0x1F83D9AB;
        ctx->state[7] = 0x5BE0CD19;
    } else {
        /* SHA-224 */
        ctx->state[0] = 0xC1059ED8;
        ctx->state[1] = 0x367CD507;
        ctx->state[2] = 0x3070DD17;
        ctx->state[3] = 0xF70E5939;
        ctx->state[4] = 0xFFC00B31;
        ctx->state[5] = 0x68581511;
        ctx->state[6] = 0x64F98FA7;
        ctx->state[7] = 0xBEFA4FA4;
    }

    ctx->is224 = is224;
    if (ctx->mode == ESP_MBEDTLS_SHA256_HARDWARE) {
        esp_sha_unlock_engine1(SHA2_256);
    }
    ctx->mode = ESP_MBEDTLS_SHA256_UNUSED;

    
    return 0;
}

static const unsigned char sha256_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
do {                                                    \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
} while( 0 )
#endif


void esp_sha_read_digest_state1(esp_sha_type sha_type, void *digest_state)
{
#ifndef NDEBUG
    {
        SemaphoreHandle_t engine_state = sha_get_engine_state(sha_type);
        assert(uxSemaphoreGetCount(engine_state) == 0 &&
               "SHA engine should be locked" );
    }
#endif

    // preemptively do this before entering the critical section, then re-check once in it
    sha_hal_wait_idle();

    //esp_sha_lock_memory_block();

    //sha_hal_read_digest(sha_type, digest_state); //replaced implementation with this!
    uint32_t *digest_state_words = (uint32_t *)digest_state;

    sha_ll_load(sha_type);
    uint32_t word_len = (256 / 32);

    sha_hal_wait_idle();
    sha_ll_read_digest(sha_type, digest_state, word_len);

    /* Fault injection check: verify SHA engine actually ran,
       state is not all zeroes.
    */
    
    for (size_t i = 0; i < word_len; i++) {
        if (digest_state_words[i] != 0) {
            return;
        }
    }
    Serial.println(counter);
    abort(); // SHA peripheral returned all zero state, probably due to fault injection


    //esp_sha_unlock_memory_block();
}


int mbedtls_sha256_finish_ret1( mbedtls_sha256_context *ctx, unsigned char output[32] )
{
    int ret;
    uint32_t last, padn;
    uint32_t high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
           | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32_BE( high, msglen, 0 );
    PUT_UINT32_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    if ( ( ret = mbedtls_sha256_update_ret1( ctx, sha256_padding, padn ) ) != 0 ) {
        goto out;
    }

    if ( ( ret = mbedtls_sha256_update_ret1( ctx, msglen, 8 ) ) != 0 ) {
        goto out;
    }

    /* if state is in hardware, read it out */
    if (ctx->mode == ESP_MBEDTLS_SHA256_HARDWARE) {
        esp_sha_read_digest_state1(SHA2_256, ctx->state);
    }

    PUT_UINT32_BE( ctx->state[0], output,  0 );
    PUT_UINT32_BE( ctx->state[1], output,  4 );
    PUT_UINT32_BE( ctx->state[2], output,  8 );
    PUT_UINT32_BE( ctx->state[3], output, 12 );
    PUT_UINT32_BE( ctx->state[4], output, 16 );
    PUT_UINT32_BE( ctx->state[5], output, 20 );
    PUT_UINT32_BE( ctx->state[6], output, 24 );

    if ( ctx->is224 == 0 ) {
        PUT_UINT32_BE( ctx->state[7], output, 28 );
    }

out:
    if (ctx->mode == ESP_MBEDTLS_SHA256_HARDWARE) {
        esp_sha_unlock_engine1(SHA2_256);
        ctx->mode = ESP_MBEDTLS_SHA256_SOFTWARE;
    }

    return ret;

}


void mbedtls_sha256_free1( mbedtls_sha256_context *ctx )
{
    if ( ctx == NULL ) {
        return;
    }

    if (ctx->mode == ESP_MBEDTLS_SHA256_HARDWARE) {
        esp_sha_unlock_engine1(SHA2_256);
    }
    //mbedtls_zeroize( ctx, sizeof( mbedtls_sha256_context ) );
}

void mbedtls_sha256_init1( mbedtls_sha256_context *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_sha256_context ) );
}


mbedtls_sha256_context ctx;

IRAM_ATTR int nerd_double_sha2_hwcrypt(uint8_t* dataIn, uint8_t* doubleHash)
{

  unsigned char *payload = dataIn;
  
  unsigned char first_hash[32];
  
  int ret __attribute__((unused));
  
  if(!initialized){
    
    initialized=true;

    mbedtls_sha256_init1(&ctx);
    esp_sha_try_lock_engine(SHA2_256);
  }
  counter += 1;
  //Serial.println(counter);

   
   //esp_sha_lock_memory_block();
   

  ctx.mode = ESP_MBEDTLS_SHA256_UNUSED;
  mbedtls_sha256_starts_ret1(&ctx, 0);
  
  
  ret = mbedtls_sha256_update_ret1(&ctx, dataIn, sizeof(uint8_t)*80);
  assert(ret == 0);
  
  ret = mbedtls_sha256_finish_ret1(&ctx, first_hash);
  assert(ret == 0);
  
  mbedtls_sha256_starts_ret1(&ctx, 0);
  ctx.mode = ESP_MBEDTLS_SHA256_UNUSED;

  ret = mbedtls_sha256_update_ret1(&ctx, first_hash, 32);
  assert(ret == 0);

  ret = mbedtls_sha256_finish_ret1(&ctx, doubleHash);
  assert(ret == 0);
  
  mbedtls_sha256_free1(&ctx);
  

  
  
  return 0;
  
}

