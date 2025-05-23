#include <windows.h>
#include <stdio.h>
#define KEYGENALGO_EXPORTS
#include "KeyGenAlgoInterfaceEx.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum AES128
{
    AES_ROUNDS = 10,
    AES_BLOCK_SIZE = 16,
    AES_KEY_LEN = 16,
    CMAC_KEY_LEN = 16,
};

typedef struct aes128_t
{
    uint8_t round_key[AES_BLOCK_SIZE * (AES_ROUNDS + 1)];
    uint8_t iv[AES_BLOCK_SIZE];
} aes128_t;
/**
 * @brief Initiate a new aes128_t context for encryption / decryption
 *
 * @param[inout] ctx aes128 instance
 * @param[in] iv initialization vector for the context
 * @param[in] key 128-bit symmetric key
 */

void aes128_init(aes128_t* ctx, const uint8_t* iv, const uint8_t* key);

/**
 * @brief Initiate a new aes128_t context for CMAC
 *
 * @param[inout] ctx aes128 instance
 * @param[in] key 128-bit key
 */

void aes128_init_cmac(aes128_t* ctx, const uint8_t* key);

/**
 * @brief Encrypt contents in-place
 *
 * @param[inout] ctx aes128 instance
 * @param[inout] chunk pointer to plaintext/ciphertext
 * @param[in] length number of bytes to encrypt
 */

void aes128_encrypt(aes128_t* ctx, uint8_t* chunk, size_t length);

/**
 * @brief Decrypt contents in-place
 *
 * @param[inout] ctx aes128 instance
 * @param[inout] chunk pointer to ciphertext/plaintext
 * @param[in] length number of bytes to decrypt
 */

void aes128_decrypt(aes128_t* ctx, uint8_t* chunk, size_t length);

/**
 * @brief Cipher-based Message Authentication Code (OMAC1)
 *
 * @param[inout] ctx CMAC-specific aes128 instance
 * @param[in] msg pointer to ciphertext/plaintext message
 * @param[in] length number of bytes to process
 * @param[out] mac 16-byte generated tag
 */

void aes128_cmac(const aes128_t* ctx, const uint8_t* msg, size_t length, uint8_t* mac);


typedef unsigned char uint8;

typedef union state_t
{
    uint8_t s[4][4];
} state_t;


// Precalculated lookup table instead of recursive polynomial multiplication
static uint8_t galois_multiply(uint8_t x, uint8_t y)
{
    static const uint8_t polynomial_mask[8] = { 0x00, 0x1b, 0x36, 0x2d, 0x6c, 0x77, 0x5a, 0x41 };
    uint16_t r = ((x * (y & 0x01)) ^
        (x * (y & 0x02)) ^
        (x * (y & 0x04)) ^
        (x * (y & 0x08)));
    return (uint8_t)(r ^ polynomial_mask[r >> 8]);
}

static void xor128(uint8_t* a, const uint8_t* b)
{
    size_t i = 0;
    for (; i < AES_BLOCK_SIZE; i++)
    {
        a[i] ^= b[i];
    }
}

// Performs a substitution non-linear substitution table used to perform one-to-one byte substitutions
static uint8_t sbox_lookup(uint8_t index, bool invert)
{
    static const uint8_t sbox[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };
    static const uint8_t rsbox[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

    return invert ? rsbox[index] : sbox[index];
}

// Generate a key schedule using the cipher key
static void aes_key_expansion(uint8_t* round_key, const uint8_t* key)
{
    static const uint8_t round_constants[11] = { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };
    // First round key
    memcpy(round_key, key, AES_KEY_LEN);
    uint8_t word[4];
    size_t i = 4;
    for (; i < 44; i++)
    {
        size_t j = 0;
        for (; j < 4; j++)
        {
            word[j] = round_key[4 * (i - 1) + j];
        }
        // Rotate and substitute
        if (!(i % 4))
        {
            const uint8_t _word = word[0];
            word[0] = word[1];
            word[1] = word[2];
            word[2] = word[3];
            word[3] = _word;
            size_t j = 0;
            for (; j < 4; j++)
            {
                word[j] = sbox_lookup(word[j], false);
            }
            word[0] ^= round_constants[i / 4];
        }
        for (j = 0; j < 4; j++)
        {
            round_key[(4 * i) + j] = round_key[(4 * (i - 4)) + j] ^ word[j];
        }
    }
}
// AES-CMAC subkey generation algorithm
static void aes_generate_subkey(uint8_t* key)
{
    uint8_t msb = key[0] & 0x80;
    size_t i = 0;
    for (; i < 15; i++)
    {
        key[i] = (uint8_t)(key[i] << 1U) | (key[i + 1] >> 7U);
    }
    key[15] = (uint8_t)(key[15] << 1U);
    if (msb)
    {
        key[15] ^= 0x87; // const_Rb
    }
}
// XOR a Round Key into the state
static void aes_add_round_key(state_t* ctx, uint8_t round, const uint8_t* round_key)
{
    size_t i = 0;
    for (; i < 4; i++)
    {
        size_t j = 0;
        for (; j < 4; j++)
        {
            ctx->s[i][j] ^= round_key[(AES_BLOCK_SIZE * round) + (4 * i) + j];
        }
    }
}
// Substitute bytes in the state using sbox / rsbox
static void aes_substitute_bytes(state_t* ctx, bool invert)
{
    size_t i = 0;
    for (; i < 4; i++)
    {
        size_t j = 0;
        for (; j < 4; j++)
        {
            ctx->s[j][i] = sbox_lookup(ctx->s[j][i], invert);
        }
    }
}
// Cyclically shift the last three rows of the State with different offsets
static void aes_shift_rows(state_t* ctx, bool invert)
{
    // Cycle the first row 1 column to the left or right
    uint8_t s = ctx->s[invert ? 3 : 0][1];
    ctx->s[invert ? 3 : 0][1] = ctx->s[invert ? 2 : 1][1];
    ctx->s[invert ? 2 : 1][1] = ctx->s[invert ? 1 : 2][1];
    ctx->s[invert ? 1 : 2][1] = ctx->s[invert ? 0 : 3][1];
    ctx->s[invert ? 0 : 3][1] = s;
    // Cycle the second row 2 columns
    s = ctx->s[0][2];
    ctx->s[0][2] = ctx->s[2][2];
    ctx->s[2][2] = s;
    s = ctx->s[1][2];
    ctx->s[1][2] = ctx->s[3][2];
    ctx->s[3][2] = s;
    // Cycle the third row 3 columns to the left or right
    s = ctx->s[0][3];
    ctx->s[invert ? 0 : 0][3] = ctx->s[invert ? 1 : 3][3];
    ctx->s[invert ? 1 : 3][3] = ctx->s[invert ? 2 : 2][3];
    ctx->s[invert ? 2 : 2][3] = ctx->s[invert ? 3 : 1][3];
    ctx->s[invert ? 3 : 1][3] = s;
}

// Mix the contents of the state's columns to produce new columns
// s`(x) = a(x) ⊗ s(x)
static void aes_mix_columns(state_t* ctx, bool invert)
{
    if (invert)
    {
        size_t i = 0;
        for (; i < 4; i++)
        {
            uint8_t s[4] = {
                ctx->s[i][0],
                ctx->s[i][1],
                ctx->s[i][2],
                ctx->s[i][3] };
            ctx->s[i][0] = (galois_multiply(s[0], 0x0e) ^
                galois_multiply(s[1], 0x0b) ^
                galois_multiply(s[2], 0x0d) ^
                galois_multiply(s[3], 0x09));
            ctx->s[i][1] = (galois_multiply(s[0], 0x09) ^
                galois_multiply(s[1], 0x0e) ^
                galois_multiply(s[2], 0x0b) ^
                galois_multiply(s[3], 0x0d));
            ctx->s[i][2] = (galois_multiply(s[0], 0x0d) ^
                galois_multiply(s[1], 0x09) ^
                galois_multiply(s[2], 0x0e) ^
                galois_multiply(s[3], 0x0b));
            ctx->s[i][3] = (galois_multiply(s[0], 0x0b) ^
                galois_multiply(s[1], 0x0d) ^
                galois_multiply(s[2], 0x09) ^
                galois_multiply(s[3], 0x0e));
        }
    }
    else
    {
        size_t i = 0;
        for (; i < 4; i++)
        {
            uint8_t s = (ctx->s[i][0] ^
                ctx->s[i][1] ^
                ctx->s[i][2] ^
                ctx->s[i][3]);
            uint8_t c0 = ctx->s[i][0];
            size_t j = 0;
            for (; j < 4; j++)
            {
                // XOR s[i][3] with the previous s[i][0]
                uint8_t c = ctx->s[i][j] ^ ((j == 3) ? c0 : ctx->s[i][j + 1]);
                c = ((uint8_t)(c << 1U) ^ (((c >> 7) & 1) * 0x1b));
                ctx->s[i][j] ^= c ^ s;
            }
        }
    }
}

// Main cipher function
static void aes_xcrypt(state_t* state, const uint8_t* round_key, bool decrypt)
{
    if (decrypt)
    {
        aes_add_round_key(state, AES_ROUNDS, round_key);
        size_t i = 0;
        for (;; i++)
        {
            aes_shift_rows(state, true);
            aes_substitute_bytes(state, true);
            aes_add_round_key(state, AES_ROUNDS - 1 - i, round_key);
            if (i == AES_ROUNDS - 1)
            {
                break;
            }
            aes_mix_columns(state, true);
        }
    }
    else
    {
        aes_add_round_key(state, 0, round_key);
        size_t i = 1;
        for (;; i++)
        {
            aes_substitute_bytes(state, false);
            aes_shift_rows(state, false);
            if (i == AES_ROUNDS)
            {
                break;
            }
            aes_mix_columns(state, false);
            aes_add_round_key(state, i, round_key);
        }
        aes_add_round_key(state, AES_ROUNDS, round_key);
    }
}

void aes128_cmac(const aes128_t* ctx, const uint8_t* msg, size_t length, uint8_t* mac)
{
    // Subkey generation (pg 5 RFC 4493)
    uint8_t L[AES_BLOCK_SIZE] = { 0 }; // Output of AES(0)
    aes_xcrypt((state_t*)L, ctx->round_key, false);
    aes_generate_subkey(L);
    // MAC generation (pg 7 RFC 4493)
    uint8_t block[AES_BLOCK_SIZE] = { 0 };
    for (; length; length -= AES_BLOCK_SIZE)
    {
        if (length < AES_BLOCK_SIZE)
        {
            aes_generate_subkey(L);
            L[length] ^= 128;
        }
        if (length <= AES_BLOCK_SIZE)
        {
            size_t i = 0;
            for (; i < length; i++)
            {
                L[i] ^= msg[i];
            }
            length = AES_BLOCK_SIZE;
            msg = L;
        }
        xor128(block, msg);
        aes_xcrypt((state_t*)block, ctx->round_key, false);
        msg += AES_BLOCK_SIZE;
    }
    memcpy(mac, block, AES_BLOCK_SIZE);
}

void aes128_init_cmac(aes128_t* ctx, const uint8_t* key)
{
    memset(ctx, 0, sizeof(*ctx));
    aes_key_expansion(ctx->round_key, key);
}

static void Generate_derived_key(uint8* l_df_ub, uint8* l_SK_ub)
{
    uint8 masterKey[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

    aes128_t ctx;
    aes128_init_cmac(&ctx, masterKey);
    aes128_cmac(&ctx, l_df_ub, 16, l_SK_ub);
}

static void SecurityAccess_CalcKey(const uint8* l_Seed_ub, uint8* l_Key_ub)
{
    uint8 skey[16];
    uint8 cmac_result[16];
    uint8 DF[] = { 0x22, 0x97, 0x62, 0x12, 0xbb, 0x16, 0xe3, 0x0c, 0x82, 0x01, 0x81, 0xe7, 0x1e, 0x11, 0x22, 0x99 };

    Generate_derived_key(DF, skey);

    aes128_t ctx;
    aes128_init_cmac(&ctx, skey);
    aes128_cmac(&ctx, l_Seed_ub, 16, cmac_result);

    // 32 bytes key = DF+cmac
    memcpy(l_Key_ub, DF, 16);
    memcpy(l_Key_ub + 16, cmac_result, 16);
}


BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return TRUE;
}


#define SecM_Level1_Cycle_value 32
#define SecM_Level2_Cycle_value 32
#define SecM_Level3_Cycle_value 32
#define SecM_Level4_Cycle_value 32

KEYGENALGO_API VKeyGenResultEx GenerateKeyEx(
    const unsigned char*    iSeedArray,     /* Array for the seed [in] */
    unsigned int            iSeedArraySize, /* Length of the array for the seed [in] */
    const unsigned int      iSecurityLevel, /* Security level [in] */
    const char*             iVariant,       /* Name of the active variant [in] */
    unsigned char*          ioKeyArray,     /* Array for the key [in, out] */
    unsigned int            iKeyArraySize,  /* Maximum length of the array for the key [in] */
    unsigned int&           oSize           /* Length of the key [out] */
)
{

    SecurityAccess_CalcKey(iSeedArray, ioKeyArray);
    oSize = 32;
    return KGRE_Ok;

}

int main()
{

    unsigned char   iSeedArray[16] = { 0x12,0x34,0x56,0x12,0x34,0x56 ,0x12,0x34,0x56 ,0x12,0x34,0x56 ,0x12,0x34,0x56,0x12 };/* Array for the seed [in] */
    unsigned int    iSeedArraySize = 16; /* Length of the array for the seed [in] */
    unsigned int    iSecurityLevel = 0x01; /* Security level 01 or 11 */
    char            iVariant[4] = { 0,0,0,0 };    /* Name of the active variant [in] */
    unsigned char   ioKeyArray[16] = { 0,0,0,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0 };    /* Array for the key [in, out] */
    unsigned int     iKeyArraySize = 16; /* Maximum length of the array for the key [in] */
    unsigned int     oSize = 0;         /* Length of the key [out] */


    printf("oSize %x \n", oSize);

    GenerateKeyEx(iSeedArray, iSeedArraySize, 5, iVariant, ioKeyArray, iKeyArraySize, (unsigned int&)oSize);
    printf("27 05/06 \n");
    for (int i = 0; i < 16; ++i) {
        printf("key0 %x \n", ioKeyArray[i]);
        printf("%d", i);
        printf("\n");
    }

    for (int i = 0; i < 16; ++i) {
        printf("%x ", ioKeyArray[i]);
        // printf("%d", i);
        // printf("\n");
    }
    printf("oSize %x \n", oSize);
    int a, b;
    scanf("%d%d", a, b);
    return 0;
}
