#ifndef XORCRYPT_H
#define XORCRYPT_H

#include <stdlib.h>
#include <stdio.h>

extern char key[10];

void XOREncrypt(char *pw, char count);
void XORDecrypt(char *encryptPw, char count);

#endif
