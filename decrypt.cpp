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
        if (numRows == keylen){
            // check if there is only 1 line left
            long fpos = ftell(infile);
            fseek(infile, 0, SEEK_END);
            //fprintf(stderr, "fpos=%d, ftell(infile)==%d\n", fpos, ftell(infile));
            if (ftell(infile) == fpos + keylen){
                // this has the padding length, add to the current buffer
                //fprintf(stderr, "One line remaining\n");
                numRows++;
                fseek(infile, fpos, SEEK_SET);
                int br = fread(buffer[numRows-1], sizeof(char), keylen, infile);
                done = true;
            } else if (fpos == ftell(infile)){
                done = true;
            } else {
                fseek(infile, fpos, SEEK_SET);
            }
        }
        if (done && numRows == 0)
            break;

        // copy the buffer so we can reference it later
        for (int i = 0; i < numRows; ++i)
            for (int j = 0; j < keylen; ++j)
                buffer_clean[i][j] = buffer[i][j];

        // horizontal pass
        for (int j = 0; j < keylen; ++j){
            int f = f1(0, j, row1, numRows);
            int g = g1(0, j, row1, numRows);

            int x1i = (j-f)%numRows;
            int x2i = (j-g)%numRows;

            x1i = (x1i < 0) ? -1*x1i : x1i;
            x2i = (x2i < 0) ? -1*x2i : x2i;

            int x1 = row1[x1i];
            int x2 = row1[x2i];

            int x = (x1^x2)^buffer[j][0];
            buffer[j][0] = (char)x;
        }
        for (int i = 1; i < keylen; ++i){
            for (int j = 0; j < numRows; ++j){
                char* prevcol = new char[numRows];
                getCol(buffer_clean, prevcol, numRows, i-1);
                int f = f1(i, j, prevcol, numRows);
                int g = g1(i, j, prevcol, numRows);

                int x1i = (j-f)%numRows;
                int x2i = (j-g)%numRows;

                x1i = (x1i < 0) ? -1*x1i : x1i;
                x2i = (x2i < 0) ? -1*x2i : x2i;

                int x1 = prevcol[x1i];
                int x2 = prevcol[x2i];

                int x = (x1^x2)^(buffer[j][i]);
                buffer[j][i] = (char)x;
            }
        }
        // update the buffer so we can reference it later
        for (int i = 0; i < numRows; ++i)
            for (int j = 0; j < keylen; ++j)
                buffer_clean[i][j] = buffer[i][j];

        // Vertical pass
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
            }
        }

        // set up new row1
        //memcpy(row1, buffer_clean[numRows-1], keylen);


        // Remove padding and paddingLength
        int pad = 0;
        if (done){
            //fprintf(stderr, "Done\n");
            pad = buffer[numRows-1][0] << 8;
            pad |= buffer[numRows-1][1];
            for (int i = 0; i < numRows-2; ++i){
                for (int j = 0; j < keylen; ++j)
                    printf("%c", buffer[i][j]);
            }
            for (int i = 0; i < keylen-pad; ++i)
                printf("%c", buffer[numRows-2][i]);
        } else {
            for (int i = 0; i < numRows; ++i){
                for (int j = 0; j < keylen; ++j)
                    printf("%c", buffer[i][j]);
            }
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
