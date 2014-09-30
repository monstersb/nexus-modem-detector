#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <errno.h>
#include <string.h>

#define error() fprintf(stderr, \
    "Error in %s on line %d : %s\n", \
    __FILE__, \
    __LINE__, \
    strerror(errno))

typedef unsigned char t_byte;
typedef long t_length;

typedef struct {
    t_byte *buffer;
    t_length length;
} t_buffer;

char workspace[0x100] = { 0 };

t_buffer *readfile(int fnumber);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage:\t%s workspace\n", argv[0]);
        return -1;
    }

    char *s = "a";
    strcpy(workspace, (const char *)"");
    return 0;
    strncpy(workspace, argv[1], sizeof(workspace));
    t_buffer *buf00 = readfile(0);
    if (buf00 == NULL) {
        error();
        return -1;
    }

    return 0;
}

t_buffer *readfile(int fnumber) {
    char fname[0x100];
    FILE *fp;
    t_length flength, bytesread;
    t_buffer *buffer;
    sprintf(fname, "%s/modem.b%02d", workspace, fnumber);
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    flength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (flength == -1) {
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    buffer = (t_buffer*)malloc(sizeof(t_buffer));
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }
    buffer->length = flength;
    buffer->buffer = (t_byte*)malloc(flength);
    if (buffer->buffer == NULL) {
        free(buffer);
        fclose(fp);
        return NULL;
    }
    bytesread = fread(buffer->buffer, sizeof(t_byte), flength, fp);
    if (bytesread != flength) {
        free(buffer->buffer);
        free(buffer);
        fclose(fp);
        return NULL;
    }
    return buffer;
}
