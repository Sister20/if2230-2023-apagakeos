#include "../std/stdtype.h"
#include "fat32.h"
#include "../std/stdmem.h"

/*-----------------------------------------------------------------------------------*/
/*-------------------------------------CONSTANT--------------------------------------*/

/* signature of file system */
const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '3', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

/* struct to save the file system driver state */
struct FAT32DriverState driverState;

/*-----------------------------------------------------------------------------------*/
/*-------------------------------------HELPER----------------------------------------*/

/* return the logical block address where cluster start */
uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster * CLUSTER_BLOCK_COUNT;
}

/* write the the content of ptr to cluster from cluster_number to cluster_number+cluster_count */
void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    write_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

/* read and put the content of cluster from cluster_number to cluster_number+cluster_count into ptr */
void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    read_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

/* put the first entry of this directory table with the directory name and its parent cluster number */
void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
    // create new entry of directory table, cluster number refer to the parent of this directory
    struct FAT32DirectoryEntry dirEntry = {
        .name = {name[0], name[1], name[2], name[3], name[4], name[5], name[6], name[7]},
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,
        .cluster_high = parent_dir_cluster >> 16,
        .cluster_low = parent_dir_cluster,
        .filesize = 0,
    };

    // put the entry as the first entry
    dir_table->table[0] = dirEntry;
}

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------INITIALIZER-------------------------------------*/

/* check if current storage is empty (do not have the filesystem signature) */
bool is_empty_storage(void) {
    // initiate buffer to contain boot sector content
    uint8_t temp[BLOCK_SIZE];
    
    // read the content of boot sector and put it to temp
    read_blocks(temp,BOOT_SECTOR,1);

    // compare the buffer with filesystem signature, return true if equal
    return memcmp(fs_signature, temp, BLOCK_SIZE) != 0;
}

/* create new file system if storage is empty */
void create_fat32(void) {
    // write the file system signature to the boot sector (cluster 0)
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    // initialize and write FAT to cluster 1
    driverState.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driverState.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driverState.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    write_clusters(driverState.fat_table.cluster_map, 1, 1);
    
    // initialize root directory and write it to cluster 2
    struct FAT32DirectoryTable rootDir = {
        .table = {
            {
            .name = {'r','o','o','t'},
            .attribute = ATTR_SUBDIRECTORY,
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = 0x00,
            .cluster_low = 0x02,
            .filesize = 0 
            }
        }
    };
    write_clusters(rootDir.table, 2, 1);
}

/* initialize file system */
void initialize_filesystem_fat32(void) {
    // if storage empty then create new file system
    // else load the FAT to driverState
    if (is_empty_storage()) {
        create_fat32();
    } 
    else {
        read_clusters(&driverState.fat_table, 1, 1);
    }
}

/*-----------------------------------------------------------------------------------*/
/*---------------------------------CRUD OPERATION------------------------------------*/

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