#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int f1(int i, int j, char* key){
    return (key[i] ^ key[j]) % strlen(key);
}

int g1(int i, int j, char* key){
    return (key[i] * key[j]) % strlen(key);
}

int encrypt(char* key, int keylen, char* filename){

    char** buffer = new char*[keylen];
    for (int i = 0; i < keylen; ++i)
        buffer[i] = new char[keylen];

    FILE* infile = fopen(filename, "r");
    if (!infile){
        perror("fopen");
        return -1;
    }
    
    int numRows = 0;
    int padding = 0;
    for (int i = 0; i < keylen; ++i){
        int br = fread(buffer[i], sizeof(char), keylen, infile);
        if (br == 0){
            // no data on the row
            break;
        } else if (br == keylen){
            // we got a full row!
            numRows++;
            continue;
        } else {
            // pad to full row
            int pad = keylen - br;
            memset(&buffer[i][br], 0x00, pad);
            padding += pad;
            numRows++;
        }
    }

    for (int j = 0; j < keylen; ++j){
        int f = f1(0, j, key);
        int g = g1(0, j, key);

        int x1 = key[(j-f)%keylen];
        int x2 = key[(j-g)%keylen];

        int x = (x1^x2)^buffer[0][j];
        buffer[0][j] = (char)x;
        printf("%c", (char)x); 
    }
    for (int i = 1; i < numRows; ++i){
        for (int j = 0; j < keylen; ++j){
            int f = f1(i, j, key);
            int g = g1(i, j, key);

            int x1 = buffer[i-1][(j-f)%keylen];
            int x2 = buffer[i-1][(j-g)%keylen];

            int x = (x1^x2)^(buffer[i][j]);
            buffer[i][j] = (char)x;
            printf("%c", (char)x); 
        }
    }



}


int main(int argc, char** argv){
    if (argc < 3){
        printf("Usage: %s <Key> <Plaintext file>\n", argv[0]);
        return 1;
    }

    char* key = new char[strlen(argv[1])];
    strcpy(key, argv[1]);

    char* filename = new char[strlen(argv[2])];
    strcpy(filename, argv[2]);


    encrypt(key, strlen(key), filename);

    delete[] key;
    delete[] filename;
}
