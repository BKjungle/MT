#ifndef _Enc_H_
#define _Enc_H_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#define AES_BITS 128
#define MSG_LEN 128

bool aess_encrypt( char* in,  char* key,  char* out,int olen);

bool aes_decrypt( char* in,  char* key,  char* out);

#endif