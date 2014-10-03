#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "elf.h"

#define error() fprintf(stderr, \
    "Error in %s on line %d : %s\n", \
    __FILE__, \
    __LINE__, \
    strerror(errno))

#define HEADER_FILE 0

typedef unsigned char t_byte;
typedef long t_length;

typedef struct {
    t_byte *buffer;
    t_length length;
} t_buffer;

char workspace[0x100] = { 0 };

t_buffer *readfile(int fnumber);
int buildelf();
int information(t_byte *buffer);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage:\t%s workspace\n", argv[0]);
        return -1;
    }

    strncpy(workspace, argv[1], sizeof(workspace));
	if (access(workspace, 0x04) != 00) {
		error();
        return -1;
	}

    return buildelf();
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
    fclose(fp);
    return buffer;
}

int buildelf() {
    t_buffer *bufheader = readfile(HEADER_FILE);
    if (bufheader == NULL) {
        error();
        return -1;
    }

	if (information(bufheader->buffer) != 0) {
		fprintf(stderr, "Bad file format!\n");
		return -1;
	}
	return 0;
}

int information(t_byte *buffer) {
	Elf32_Ehdr *elfhdr;
	Elf32_Phdr *prohdr;
	int i;
	elfhdr = (Elf32_Ehdr*)buffer;
	if (elfhdr->e_ident[EI_MAG0] != ELFMAG0
		|| elfhdr->e_ident[EI_MAG1] != ELFMAG1
		|| elfhdr->e_ident[EI_MAG2] != ELFMAG2
		|| elfhdr->e_ident[EI_MAG3] != ELFMAG3) {
		return -1;
	}
	printf("ELF header:\n");

	printf("\tMagic: ");
	for (i = 0;i < EI_NIDENT;i++) {
		printf("%02X ", elfhdr->e_ident[i]);
	}
	printf("\n");

	switch (elfhdr->e_ident[EI_CLASS]) {
	case ELFCLASSNONE:
		printf("\tClass: Invalid class\n");
		break;
	case ELFCLASS32:
		printf("\tClass: 32 bit ELF class\n");
		break;
	case ELFCLASS64:
		printf("\tClass: 64 bit ELF class\n");
		break;
	default:
		return -1;
	}

	switch (elfhdr->e_ident[EI_DATA]) {
	case ELFDATANONE:
		printf("\tData format: Unknown data format\n");
		break;
	case ELFDATA2LSB:
		printf("\tData format: Little endian\n");
		break;
	case ELFDATA2MSB:
		printf("\tData format: Big endian\n");
		break;
	default:
		return -1;
	}

	switch (elfhdr->e_type) {
	case ET_NONE:
		printf("\tType: Unknown type\n");
		break;
	case ET_REL:
		printf("\tType: Relocatable file\n");
		break;
	case ET_EXEC:
		printf("\tType: Executable file\n");
		break;
	case ET_DYN:
		printf("\tType: Shared file\n");
		break;
	case ET_CORE:
		printf("\tType: Core file\n");
		break;
	default:
		return -1;
	}

	printf("\tEntry: %08X\n", elfhdr->e_entry);
	printf("\tOffset of program header: %08X\n", elfhdr->e_phoff);
	printf("\tSize of program header: %08X\n", elfhdr->e_phentsize);
	printf("\tNumber of programs headers: %d\n", elfhdr->e_phnum);

	printf("\n");
	printf("Program Headers:\n");
	prohdr = (Elf32_Phdr*)(buffer + elfhdr->e_phoff);
	printf("\t\tType\tOffset\t\tFile size\tVirtual address\tMemory size\tFlags\tAlignment\n");
	for (i = 0;i < elfhdr->e_phnum;i++) {
		printf("\t[%d]\t%s", i, prohdr[i].p_type == 1 ? "LOAD" : "NULL");
		printf("\t%08X", prohdr[i].p_offset);
		printf("\t%08X", prohdr[i].p_filesz);
		printf("\t%08X", prohdr[i].p_vaddr);
		//printf("\t[%d]Physical address: %08X\n", i, prohdr[i].p_paddr);
		printf("\t%08X", prohdr[i].p_memsz);
		printf("\t%s%s%s", 
			(prohdr[i].p_flags & PF_R ? "R" : ""),
			(prohdr[i].p_flags & PF_W ? "W" : ""),
			(prohdr[i].p_flags & PF_X ? "X" : "")
			);
		printf("\t%08X\n", prohdr[i].p_align);
	}

	return 0;
}
