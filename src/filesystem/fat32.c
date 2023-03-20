#include "std/stdtype.h"
#include "std/stdmem.h"
#include "fat32.h"
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

uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster * CLUSTER_BLOCK_COUNT;
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    write_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    read_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

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

bool is_empty_storage(void) {
    // initiate buffer to contain boot sector content
    uint8_t temp[BLOCK_SIZE];
    
    // read the content of boot sector and put it to temp
    read_blocks(temp,BOOT_SECTOR,1);

    // compare the buffer with filesystem signature, return true if equal
    return memcmp(fs_signature, temp, BLOCK_SIZE) != 0;
}

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
int8_t write(struct FAT32DriverRequest request) {
    // read entries of directory from the parent cluster
    read_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

    // if parent cluster is not a directory, return with error code 2
    if (driverState.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY) { return 2;}

    // initialize value needed for subsequent checking
    int entryRow = 0;
    int entryChecked = 1;
    bool valid = 1;
    bool full = 1;

    // check the total entry in the directory and if there is entry with same name and extension
    while (entryChecked <= 64 && valid) {
        if (driverState.dir_table_buf.table[entryChecked-1].user_attribute != UATTR_NOT_EMPTY && full) {
            full = 0;
            entryRow = entryChecked-1;
        }
        if (memcmp(driverState.dir_table_buf.table[entryChecked-1].name, request.name, 8) == 0 && 
            memcmp(driverState.dir_table_buf.table[entryChecked-1].ext, request.ext, 3) == 0) {
            valid = 0;
        }
        entryChecked++;
    }

    // if there is entry with same name and extension, return with error code 1
    if (!valid) { return 1;}

    // if the directory is full, return with error code -1
    if (full) { return -1; }

    // if the request buffer size is 0, then create a subdirectory, else write the file 
    if (request.buffer_size == 0) {
        // find empty cluster in fat table
        uint32_t clusterNumber = 0x0;
        while (driverState.fat_table.cluster_map[clusterNumber] != 0x0 && clusterNumber < 0x800) {
            clusterNumber++;
        }

        // if there is no cluster that can be allocated, return with error code -1
        if (clusterNumber == 0x800) { return -1; }

        // update the FAT table and write it to FAT cluster
        driverState.fat_table.cluster_map[clusterNumber] = FAT32_FAT_END_OF_FILE;
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // update parent directory table and write it to parent cluster
        struct FAT32DirectoryEntry dirEntry = {
            .name = {request.name[0], request.name[1], request.name[2], request.name[3], request.name[4], request.name[5], request.name[6], request.name[7]},
            .attribute = ATTR_SUBDIRECTORY,
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = clusterNumber >> 16,
            .cluster_low = clusterNumber,
            .filesize = request.buffer_size
        };
        driverState.dir_table_buf.table[entryRow] = dirEntry;
        write_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

        // create directory table for new directory and write it to directory cluster
        read_clusters(driverState.dir_table_buf.table, clusterNumber, 1);
        dirEntry.cluster_high = request.parent_cluster_number >> 16;
        dirEntry.cluster_low = request.parent_cluster_number;
        driverState.dir_table_buf.table[0] = dirEntry;
        write_clusters(driverState.dir_table_buf.table, clusterNumber, 1);
    }
    else {
        // determine number of cluster needed
        int modulo = request.buffer_size % CLUSTER_SIZE;
        int clusterNeeded = request.buffer_size / CLUSTER_SIZE;
        if (modulo != 0) { clusterNeeded++; } 

        // get all cluster_number that is still empty
        int clusterAvailable = 0;
        uint32_t currClusterNumber = 0x0;
        uint32_t clusterNumber[clusterNeeded];
        while (clusterAvailable < clusterNeeded && currClusterNumber != 0x800) {
            if (driverState.fat_table.cluster_map[currClusterNumber] == 0x0) {
                clusterNumber[clusterAvailable] = currClusterNumber;
                clusterAvailable++;
            }
            currClusterNumber++;
        }

        // if there is not enough cluster to contain the file, return with error code -1
        if (clusterAvailable != clusterNeeded) { return -1;}
        
        // update the FAT table and write it to FAT cluster
        for (int i = 0; i < clusterNeeded-1; i++) {
            driverState.fat_table.cluster_map[clusterNumber[i]] = clusterNumber[i+1];
        }
        driverState.fat_table.cluster_map[clusterNumber[clusterNeeded-1]] = FAT32_FAT_END_OF_FILE;
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // update parent directory table and write it to parent cluster
        struct FAT32DirectoryEntry dirEntry = {
            .name = {request.name[0], request.name[1], request.name[2], request.name[3], request.name[4], request.name[5], request.name[6], request.name[7]},
            .ext = {request.ext[0], request.ext[1], request.ext[2]},
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = clusterNumber[0] >> 16,
            .cluster_low = clusterNumber[0],
            .filesize = request.buffer_size
        };
        driverState.dir_table_buf.table[entryRow] = dirEntry;
        write_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

        // write the file to all of the cluster selected
        for (int i = 0; i < clusterNeeded; i++) {
            write_clusters((uint8_t*) request.buf + CLUSTER_SIZE*i, clusterNumber[i], 1);
        }
    }

    return 0;
}


/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request);