#ifndef AESCRYPT_H
#define AESCRYPT_H

#include <stdlib.h>
#include <stdio.h>

char key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7 };

void XOREncrypt(char *pw, char count);
void XORDecrypt(char *encryptPw, char count);

#endif