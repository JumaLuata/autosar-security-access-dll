#include "SecAcc_Interface.h"
#include "Crypto_Types.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>  // Added for memcmp

static SecAcc_ConfigType currentConfig;
static uint8_t level01Key[16] = {0};
static uint8_t level11Key[16] = {0};

// Forward declaration from AES128.c
extern Std_ReturnType AES128_GenerateKey(uint8_t securityLevel, 
                                       const uint8_t* seed, 
                                       uint8_t seedLength,
                                       uint8_t* key);

Std_ReturnType SecAcc_Init(const SecAcc_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL) return E_NOT_OK;
    
    currentConfig = *ConfigPtr;
    srand((unsigned int)time(NULL));  // Fixed cast warning
    
    // Generate random keys for both security levels
    for(int i = 0; i < 16; i++) {
        level01Key[i] = (uint8_t)(rand() % 256);
        level11Key[i] = (uint8_t)(rand() % 256);
    }
    
    return E_OK;
}

Std_ReturnType SecAcc_GenerateKey(uint8_t securityLevel, 
                                const uint8_t* seed, 
                                uint8_t seedLength,
                                uint8_t* key, 
                                uint8_t* keyLength) {
    if (seed == NULL || key == NULL || keyLength == NULL) {
        return E_NOT_OK;
    }
    
    if (seedLength != SECACC_MAX_SEED_LENGTH) {
        return E_SEED_INVALID;
    }
    
    *keyLength = SECACC_MAX_KEY_LENGTH;
    
    switch(securityLevel) {
        case SECACC_LEVEL_01:
        case SECACC_LEVEL_11:
            return AES128_GenerateKey(securityLevel, seed, seedLength, key);
        default:
            return E_NOT_OK;
    }
}

Std_ReturnType SecAcc_VerifyKey(uint8_t securityLevel,
                               const uint8_t* key,
                               uint8_t keyLength) {
    if (key == NULL || keyLength != SECACC_MAX_KEY_LENGTH) {
        return E_KEY_INVALID;
    }
    
    const uint8_t* expectedKey = NULL;
    
    switch(securityLevel) {
        case SECACC_LEVEL_01:
            expectedKey = level01Key;
            break;
        case SECACC_LEVEL_11:
            expectedKey = level11Key;
            break;
        default:
            return E_NOT_OK;
    }
    
    return (memcmp(key, expectedKey, (size_t)SECACC_MAX_KEY_LENGTH) == 0) ? E_OK : E_KEY_INVALID;
}