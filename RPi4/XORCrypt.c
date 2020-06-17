#include "XORCrypt.h"

char key[10] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7 };

/*
XOR Encrypt Function
Encrypt result save param pw
*/
void XOREncrypt(char *pw, char count)
{
    char i;

    for (i = 0; i < count; ++i)
    {
        pw[i] = pw[i] ^ key[i];
    }
}

/*
XOR Decrypt Function
Decrypt result save param encryptPw
*/
void XORDecrypt(char *encryptPw, char count)
{
    char i;

    for (i = 0; i < count; ++i)
    {
        encryptPw[i] = encryptPw[i] ^ key[i];
    }
}
