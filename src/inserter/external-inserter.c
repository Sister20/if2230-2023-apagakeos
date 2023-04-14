#include <stdio.h>
#include <stdlib.h>
// #include "std/stdtype.h"

// Usual gcc fixed width integer type 
typedef u_int32_t uint32_t;
typedef u_int8_t  uint8_t;

// Manual import from fat32.h, disk.h, & stdmem.h due some issue with size_t
#define BLOCK_SIZE      512

struct FAT32DriverRequest {
    void     *buf;
    char      name[8];
    char      ext[3];
    uint32_t  parent_cluster_number;
    uint32_t  buffer_size;
} __attribute__((packed));

void* memcpy(void* restrict dest, const void* restrict src, size_t n);

void create_fat32(void);

void initialize_filesystem_fat32(void);
/**
 *  FAT32 Folder / Directory read
 *
 * @param request buf point to struct FAT32DirectoryTable,
 *                name is directory name,
 *                ext is unused,
 *                parent_cluster_number is target directory table to read,
 *                buffer_size must be exactly sizeof(struct FAT32DirectoryTable)
 * @return Error code: 0 success - 1 not a folder - 2 not found - -1 unknown
 */
int8_t read_directory(struct FAT32DriverRequest request);

/**
 * FAT32 read, read a file from file system.
 *
 * @param request All attribute will be used for read, buffer_size will limit reading count
 * @return Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
 */
int8_t read(struct FAT32DriverRequest request);

/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request);

/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request);

// Global variable
uint8_t *image_storage;
uint8_t *file_buffer;

void read_blocks(void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    for (int i = 0; i < block_count; i++)
        memcpy((uint8_t*) ptr + BLOCK_SIZE*i, image_storage + BLOCK_SIZE*(logical_block_address+i), BLOCK_SIZE);
}

void write_blocks(const void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    for (int i = 0; i < block_count; i++)
        memcpy(image_storage + BLOCK_SIZE*(logical_block_address+i), (uint8_t*) ptr + BLOCK_SIZE*i, BLOCK_SIZE);
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "inserter: ./inserter <file to insert> <parent cluster index> <storage>\n");
        exit(1);
    }

    // Read storage into memory, requiring 4 MB memory
    image_storage     = malloc(4*1024*1024);
    file_buffer       = malloc(4*1024*1024);
    FILE *fptr        = fopen(argv[3], "r");
    fread(image_storage, 4*1024*1024, 1, fptr);
    fclose(fptr);

    // Read target file, assuming file is less than 4 MiB
    FILE *fptr_target = fopen(argv[1], "r");
    size_t filesize   = 0;
    if (fptr_target == NULL)
        filesize = 0;
    else {
        fread(file_buffer, 4*1024*1024, 1, fptr_target);
        fseek(fptr_target, 0, SEEK_END);
        filesize = ftell(fptr_target);
        fclose(fptr_target);
    }

    printf("Filename : %s\n",  argv[1]);
    printf("Filesize : %ld bytes\n", filesize);

    // FAT32 operations
    initialize_filesystem_fat32();
    struct FAT32DriverRequest request = {
        .buf         = file_buffer,
        .ext         = "\0\0\0",
        .buffer_size = filesize,
    };
    sscanf(argv[2], "%u",  &request.parent_cluster_number);
    sscanf(argv[1], "%8s", request.name);
    int retcode = write(request);
    if (retcode == 0)
        puts("Write success");
    else if (retcode == 1)
        puts("Error: File/folder name already exist");
    else if (retcode == 2)
        puts("Error: Invalid parent cluster");
    else
        puts("Error: Unknown error");

    // Write image in memory into original, overwrite them
    fptr              = fopen(argv[3], "w");
    fwrite(image_storage, 4*1024*1024, 1, fptr);
    fclose(fptr);

    return 0;
}