#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int f1(int i, int j, char* key, int keylen){
    return (key[i] ^ key[j]) % keylen;
}

int g1(int i, int j, char* key, int keylen){
    return (key[i] * key[j]) % keylen;
}

char* getCol( char** buf, char* out, int numrows, int colidx){
    for (int i = 0; i < numrows; ++i){
        out[i] = buf[i][colidx];
    }
}

int encrypt(char* key, int keylen, char* filename){
    char** buffer = new char*[keylen+1];
    for (int i = 0; i < keylen+1; ++i)
        buffer[i] = new char[keylen];

    char** buffer_clean = new char*[keylen+1];
    for (int i = 0; i < keylen+1; ++i)
        buffer_clean[i] = new char[keylen];

    FILE* infile = fopen(filename, "rb");
    if (!infile){
        perror("fopen");
        return -1;
    }

    // prepare the first row
    char* row1 = new char[keylen+1];
    strcpy(row1, key);

    bool done = false;
    
    int padding = 0;
    while(!done){
        int numRows = 0;
        for (int i = 0; i < keylen; ++i){
            int br = fread(buffer[i], sizeof(char), keylen, infile);
            if (br == 0){
                // no data on the row
                done = true;
                
                break;
            } else if (br == keylen){
                // we got a full row!
                numRows++;
                continue;
            } else {
                // pad to full row
                int pad = keylen - br;
                memset(&(buffer[i][br]), 0x00, pad);
                padding += pad;
                numRows++;
                done = true;

                break;
            }
        }
        // check if we're at the end
        long fpos = ftell(infile);
        fseek(infile, 0, SEEK_END);
        //fprintf(stderr, "fpos=%d, ftell(infile)==%d\n", fpos, ftell(infile));
        if (ftell(infile) == fpos){
            // this has the padding length, add to the current buffer
            done = true;
            
            // add padding_info
            numRows++;
            buffer[numRows-1][0] = padding / 256;
            buffer[numRows-1][1] = padding % 256;
            memset(&(buffer[numRows-1][2]), 0x0a, keylen-2);
        }
        fseek(infile, fpos, SEEK_SET);
        
        if (done && numRows == 0)
            break;

        // copy the buffer so we can reference it later
        for (int i = 0; i < numRows; ++i)
            for (int j = 0; j < keylen; ++j)
                buffer_clean[i][j] = buffer[i][j];

        // Vertical Pass
        for (int j = 0; j < keylen; ++j){
            int f = f1(0, j, row1, keylen);
            int g = g1(0, j, row1, keylen);

            int x1i = (j-f)%keylen;
            int x2i = (j-g)%keylen;

            x1i = (x1i < 0) ? -1*x1i : x1i;
            x2i = (x2i < 0) ? -1*x2i : x2i;

            int x1 = row1[x1i];
            int x2 = row1[x2i];

            int x = (x1^x2)^buffer[0][j];
            buffer[0][j] = (char)x;
            //printf("%c", (char)x); 
        }
        for (int i = 1; i < numRows; ++i){
            for (int j = 0; j < keylen; ++j){
                int f = f1(i, j, buffer_clean[i-1], keylen);
                int g = g1(i, j, buffer_clean[i-1], keylen);

                int x1i = (j-f)%keylen;
                int x2i = (j-g)%keylen;

                x1i = (x1i < 0) ? -1*x1i : x1i;
                x2i = (x2i < 0) ? -1*x2i : x2i;

                int x1 = buffer_clean[i-1][x1i];
                int x2 = buffer_clean[i-1][x2i];

                int x = (x1^x2)^(buffer[i][j]);
                buffer[i][j] = (char)x;
                //printf("%c", (char)x); 
            }
        }

        // copy the buffer so we can reference it later
        for (int i = 0; i < numRows; ++i)
            for (int j = 0; j < keylen; ++j)
                buffer_clean[i][j] = buffer[i][j];

        // horizontal pass
        for (int j = 0; j < keylen; ++j){
            int x1=0;
            // get the sum of the key characters
            for (int i = 0; i < numRows; ++i){
                x1 += row1[i];
            }

            int x = ((x1)+buffer[j][0]) % 256;
            buffer[j][0] = (char)x;
        }
        for (int i = 1; i < keylen; ++i){
            char* prevcol = new char[numRows];
            getCol(buffer_clean, prevcol, numRows, i-1);
            int x1=0;
            // Get the sum of the previous column
            for (int k = 0; k < numRows; ++k){
                x1 += prevcol[k];
            }

            for (int j = 0; j < numRows; ++j){

                int x = ((x1)+(buffer[j][i])) % 256;
                buffer[j][i] = (char)x;
            }

            delete[] prevcol;
        }

        // print the contents of the buffer
        for (int i = 0; i < numRows; ++i){
            for (int j = 0; j < keylen; ++j)
                printf("%c", buffer[i][j]);
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
