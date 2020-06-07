#include "XORCrypt.h"

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