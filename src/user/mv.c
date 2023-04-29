// File : mv.c
// Contains the implementation of functions needed to process mv command

#include "mv.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

int parse(char* args_val, int (*args_info)[2], int args_count, int cluster, char* _name, char* _ext) {
    // Variables to keep track the currently visited directory
    uint32_t dest_search_directory_number = ROOT_CLUSTER_NUMBER;
    char* name = "\0\0\0\0\0\0\0\0";
    char* extension = "\0\0\0";

    // Variables for parsing the arguments
    int posName = (*(args_info + args_count))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + args_count))[0] + (*(args_info + args_count))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool fileFound = FALSE;
    bool directoryNotFound = FALSE;

    int errorCode = 0;

    // If path is not absolute, set the currently visited directory to current working directory
    if (!isPathAbsolute(args_val, args_info, args_count-1)) {
        dest_search_directory_number = current_directory;
    }

    // Get the directory table of the visited directory
    updateDirectoryTable(dest_search_directory_number);

    // Start searching for the directory to make 
    while (!endOfArgs) {
        // If current char is not '/', process the information of word. Else, process the word itself
        if (memcmp(args_val + index, "/", 1) != 0 && index != posEndArgs) {
            // If word already started, increment the length. Else, start new word
            if (!endWord) {
                lenName++;
            } else {
                if (fileFound && index != posEndArgs) {
                    errorCode = 1;
                    directoryNotFound = TRUE;
                    fileFound = FALSE;
                    endOfArgs = TRUE;
                } else {
                    endWord = FALSE;
                    posName = index;
                    lenName = 1;
                }
            }
        } else {
            // Process the word
            if (!endWord) {
                // If word length more than 8, set an error code and stop parsing. Else, check if the word exists as directory
                if (lenName > 8) {
                    errorCode = 2;
                    directoryNotFound = TRUE;
                    endOfArgs = TRUE;
                } else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                } else {
                    clear(name, 8);
                    clear(extension,3);
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
                        memcpy(name, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(extension, args_val + posName + i + 1, lenName-i-1);
                        }
                    } else {
                        memcpy(name, args_val + posName, lenName);
                    }
                    entry_index = findEntryName(name);
                    if (entry_index == -1) {
                        directoryNotFound = TRUE;
                        endOfArgs = TRUE;
                    } else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                            updateDirectoryTable(dest_search_directory_number);
                        } else {
                            fileFound = TRUE;
                        }
                        cluster = ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    }
                }
                endWord = TRUE;
            }
        }

        if (!endOfArgs) {
            if (index == posEndArgs) {
                endOfArgs = TRUE;
            } else {
                index++;
            }
        }
    }
    struct ClusterBuffer cbuf = {0};
    struct FAT32DriverRequest srcReq = {
        .buf = &cbuf,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = dest_search_directory_number,
        .buffer_size = CLUSTER_SIZE
    };
    memcpy(&(srcReq.name), name, 8);
    memcpy(&(srcReq.ext), extension,3);
    uint32_t retCode;

    interrupt(0, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);

    _name = name;
    _ext = extension;
    if (retCode != 0) {
        interrupt(2, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);
        if (retCode != 0) {
            return -1;
        } else {
            return 2; // Folder
        }
    } else {
        return 1; // File
    }
}

void rename(char* args_val, int (*args_info)[2], int args_count, int cluster_1, int cluster_2) {
    // read - delete - write pake nama baru
    interrupt(0, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0); // Read

    // delete
    interrupt(3, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);

    // write
    interrupt(2, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);

}

void move(char* args_val, int (*args_info)[2], int args_count, int cluster_1, int cluster_2) {
    // 
}

void mv(char* args_val, int (*args_info)[2], int args_count) {
    int cluster_1;
    int cluster_2;
    char* name_1;
    char* name_2;
    char* ext_1;
    char* ext_2;
    if (args_count == 3) {
        if (parse(args_val, args_info, 1, cluster_1, name_1, name_2, ext_1, ext_2) == 1) {
            if (parse(args_val, args_info, 2, cluster_2, name_1, name_2, ext_1, ext_2) == 1) {
                rename(args_val, args_info, args_count, cluster_1, cluster_2);
            } else if (parse(args_val, args_info, 2, cluster_2), name_1, name_2, ext_1, ext_2 == 2) {
                move(args_val, args_info, args_count, cluster_1, cluster_2);
            }
        } else {
            put("mv: cannot overwrite non-directory with directory\n", BIOS_RED);
        }
    }  else {
        put("mv: missing file operand\n", BIOS_RED);
    }
}