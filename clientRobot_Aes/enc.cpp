#include "enc.h"

using namespace std;

bool aess_encrypt( char* in,  char* key,  char* out, int olen)//可能会设置buf长度
{
    if(!in || !key || !out) return 0;
    unsigned char iv[AES_BLOCK_SIZE] = {0};//加密的初始化向量
    //iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    	
    AES_KEY aes;
    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return false;
    }
   // int len=strlen(in);//这里的长度是char*in的长度，但是如果in中间包含'\0'字符的话
    //那么就只会加密前面'\0'前面的一段，所以，这个len可以作为参数传进来，记录in的长度
    //至于解密也是一个道理，光以'\0'来判断字符串长度，确有不妥，后面都是一个道理。
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, olen, &aes, iv, AES_ENCRYPT);
    return true;
}
bool aes_decrypt( char* in,  char* key,  char* out)
{
    if(!in || !key || !out) return false;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    for(int i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    	iv[i]=0;
    AES_KEY aes;
    if(AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return false;
    }
    int len=strlen(in);
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_DECRYPT);
    return true;
}
/*
int main(int argc,char *argv[])
{
    char sourceStringTemp[MSG_LEN];
    char dstStringTemp[MSG_LEN];
    memset((char*)sourceStringTemp, 0 ,MSG_LEN);
    memset((char*)dstStringTemp, 0 ,MSG_LEN);
   
    strcpy((char*)sourceStringTemp, "{\"info\":\"你好\"}");
    //strcpy((char*)sourceStringTemp, argv[1]);
	//char kkey[] = "912194e51267870e";
	char kkey[] = "912194e51267870e9283e9a035360a78";
    char key[32] ;
    int i = 0;
    for(i = 0;i<32;i++)
    {
    	key[i] = kkey[i];
    }
    
    if(!aes_encrypt(sourceStringTemp,key,dstStringTemp))
    {
    	printf("encrypt error\n");
    	return -1;
    }
    printf("enc %d:",strlen((char*)dstStringTemp));
    for(i= 0;dstStringTemp[i];i+=1){
        printf("%x",(unsigned char)dstStringTemp[i]);
    }
    memset((char*)sourceStringTemp, 0 ,MSG_LEN);
    if(!aes_decrypt(dstStringTemp,key,sourceStringTemp))
    {
    	printf("decrypt error\n");
    	return -1;
    }
    printf("\n");
    printf("dec %d:",strlen((char*)sourceStringTemp));
    printf("%s\n",sourceStringTemp);
    for(i= 0;sourceStringTemp[i];i+=1){
        printf("%x",(unsigned char)sourceStringTemp[i]);
    }
    printf("\n");
    return 0;
}
*/