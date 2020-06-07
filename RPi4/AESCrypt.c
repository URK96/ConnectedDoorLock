#include "AESCrypt.h"

uint8_t* AESEncrypt(uint8_t *pw, uint8_t count)
{
    uint8_t *result;
    struct AES_ctx ctx;

    result = malloc(sizeof(uint8_t) * count);

    AES_init_ctx_iv(&ctx, key, ivector);
    AES_CBC_encrypt_bufferEx(&ctx, pw, result, count);

    return result;
}

void AESDecrypt(uint8_t *encryptPw, uint8_t count)
{
    uint8_t *result;
    struct AES_ctx ctx;

    result = malloc(sizeof(uint8_t) * count);

    AES_init_ctx_iv(&ctx, key, ivector);
    AES_CBC_decrypt_buffer(&ctx, encryptPw, count);
}