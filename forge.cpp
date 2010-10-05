#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SZ 1024
#define FILESZ 8192
int cpy(char* src, int offset, int srclen, char* dst, int cpylen){
    if (offset > srclen)
        return 0;

    int count = cpylen;
    if (offset + count > srclen){
        count = srclen - offset;
        //printf("reducing copy amount from %d to %d\n", cpylen, count);
    }

    memcpy(dst, &src[offset], count);

    return count;
}

int encrypt(int max_value, char* file, int filelen)
{
    int i;
    int count;
    unsigned int in_buf[BUF_SZ], temp, sum;
    char in[BUF_SZ];
    int fd;
    int last;

/* open files */
/*  
    if ((fd = open(plaintext_file, O_RDONLY)) < 0) {
        printf("Error: open for input file\n");
        return -1;
    }
*/

    last = 13;
    sum = 0;
    //while ((count = read(fd, in, BUF_SZ)) > 0) {
    int bufloc=0;
    while( (count = cpy(file, bufloc, filelen, in, BUF_SZ)) > 0 ){

        if (count == 0) {
            //close(fd);
            return -1;
        }
        in_buf[0] = (unsigned int)in[0];
        bufloc++;
        sum = (sum + in_buf[0]) % UINT_MAX;
        in_buf[0] = (last + in_buf[0] + 11) % UINT_MAX;
        for (i = 1; i < count; i++) {
            temp = (unsigned int)in[i];
            bufloc++;
            sum = (sum + temp) % UINT_MAX;
            in_buf[i] = (((in_buf[i - 1] * 7 + temp) * 13571) %
                 UINT_MAX + temp + i) % UINT_MAX;
            /*  printf(" %u ",in_buf[i]); */
        }
        last = (in_buf[i - 1] + sum) % max_value;

    }
    //fprintf(stderr, "bufloc==%d, filelen==%d\n", bufloc, filelen);

    if (count < 0) {
        printf("Error: read input file\n");
        return -1;
    }
    //printf("%d", last);
    //close(fd);
    return last;
}

int main(int argc, char** argv){
    if (argc < 4){
        printf("Usage: %s <MaxValue> <DesiredResult> <ForgedFile>\n", argv[0]);
        return 1;
    }
    int MAXVAL = atoi(argv[1]);
    int WANT = atoi(argv[2]);
    char* FILENAME = argv[3];

    char buffer[FILESZ];
    memset(buffer, 0, FILESZ);

    int filelen = 0;
    FILE *f = fopen(FILENAME, "r");
    if (f == NULL) perror("fopen");
    else {
        filelen = fread(buffer, sizeof(char), BUF_SZ, f);
        fclose(f);
    }


    int current = encrypt(MAXVAL, buffer, filelen);
    fprintf(stderr, "Currently have: %d, need: %d\n", current, WANT);
    int tmplen = filelen;
    while(current != WANT){
        buffer[tmplen] = ' ';
        tmplen++;
        current = encrypt(MAXVAL, buffer, tmplen);
        fprintf(stderr, "Currently have: %d, need: %d\n", current, WANT);
    }

    printf("%s", buffer);

    return 0;
}