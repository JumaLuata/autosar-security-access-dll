#ifndef SEC_ACC_INTERFACE_H
#define SEC_ACC_INTERFACE_H

#include <stdint.h>
#include <string.h>  // Added for memcmp
#include "Crypto_Types.h"

#define SECACC_MAX_SEED_LENGTH  16
#define SECACC_MAX_KEY_LENGTH   16

typedef enum {
    SECACC_LEVEL_01 = 0x01,  // Basic security level
    SECACC_LEVEL_11 = 0x11    // Advanced security level
} SecAcc_SecurityLevelType;

typedef struct {
    uint8_t vendorCode;
    uint8_t reserved[3];
} SecAcc_ConfigType;

#ifdef __cplusplus
extern "C" {
#endif

// DLL export macros
#if defined(_WIN32) || defined(_WIN64)
    #ifdef BUILD_DLL
        #define SECACC_API __declspec(dllexport)
    #else
        #define SECACC_API __declspec(dllimport)
    #endif
#else
    #define SECACC_API
#endif

SECACC_API Std_ReturnType SecAcc_Init(const SecAcc_ConfigType* ConfigPtr);
SECACC_API Std_ReturnType SecAcc_GenerateKey(uint8_t securityLevel, 
                                           const uint8_t* seed, 
                                           uint8_t seedLength,
                                           uint8_t* key, 
                                           uint8_t* keyLength);
SECACC_API Std_ReturnType SecAcc_VerifyKey(uint8_t securityLevel,
                                         const uint8_t* key,
                                         uint8_t keyLength);

#ifdef __cplusplus
}
#endif

#endif /* SEC_ACC_INTERFACE_H */