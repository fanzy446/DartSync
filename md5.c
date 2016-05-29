#include <stdio.h>
#include <openssl/md5.h>
// #include <string.h>
// #include "md5.h"

// //#define MD5_DIGEST_LENGTH 16

// const char *md5sum(const char *chaine){
//         struct md5_ctx ctx;
//         unsigned char digest[16];
//         md5_init(&ctx);
//         ctx.size = strlen(chaine);
//         strcpy(ctx.buf, chaine);
//         md5_update(&ctx);
//         md5_final(digest, &ctx);
//         return digest;
//     }

int main()
{
    unsigned char c[MD5_DIGEST_LENGTH];
    char *filename="md5.c";
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf (" %s\n", filename);
    fclose (inFile);
    return 0;
    

    //printf("%s\n", md5sum("123123123123fasdf"));
}