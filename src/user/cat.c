// File : cat.c
// Contains the implementation of functions needed to process cat command

#include "cat.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void showFiles (char* args_val, int (*args_info)[2], int args_pos) {
    // Variables to keep track the currently visited directory
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};

    // Variables for parsing the arguments
    int posName = (*(args_info + args_pos))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + args_pos))[0] + (*(args_info + args_pos))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool fileFound = FALSE;
    bool directoryNotFound = FALSE;

    int errorCode = 0;

    // If path is not absolute, set the currently visited directory to current working directory
    if (!isPathAbsolute(args_val, args_info, args_pos)) {
        search_directory_number = current_directory;
    }

    // Get the directory table of the visited directory
    updateDirectoryTable(search_directory_number);

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
                }
                else {
                    endWord = FALSE;
                    posName = index;
                    lenName = 1;
                }
            }
        }
        else {
            // Process the word
            if (!endWord) {
                // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
                if (lenName > 8) {
                    errorCode = 3;
                    directoryNotFound = TRUE;
                    endOfArgs = TRUE;
                } else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(search_directory_number);
                } else {
                    clear(srcName, 8);
                    clear(srcExt, 3);
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
                        memcpy(srcName, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(srcExt, args_val + posName + i + 1, lenName-i-1);
                        }
                    } else {
                        memcpy(srcName, args_val + posName, lenName);
                    }
                    entry_index = findEntryName(srcName);
                    if (entry_index == -1) {
                        directoryNotFound = TRUE;
                        endOfArgs = TRUE;
                    } else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            search_directory_number =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                            updateDirectoryTable(search_directory_number);
                        } else {
                            fileFound = TRUE;
                        }
                    }
                }
                endWord = TRUE;
            }
        }

        if (!endOfArgs) {
            if (index == posEndArgs) {
                endOfArgs = TRUE;
            }
            else {
                index++;
            }
        }
    }

    if (directoryNotFound) {
        put("cat: '", BIOS_RED);
        putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
        switch (errorCode) {
            case 1:
                put("': Not a directory\n", BIOS_RED);
                break;
            case 3:
                put("': Argument name is too long\n", BIOS_RED);
                break;
            default:
                put("': No such file or directory\n", BIOS_RED);
                break;
        }
    } else {
        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest request = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = search_directory_number,
            .buffer_size = 4 * CLUSTER_SIZE,
        };

        memcpy(&(request.name), srcName, 8);
        memcpy(&(request.ext), srcExt, 3);
        int32_t retcode;

        interrupt(0, (uint32_t) &request, (uint32_t) &retcode, 0x0);
        if (retcode != 0) {
            put("cat: '", BIOS_RED);
            putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
            switch (retcode) {
                case 1:
                    put("': Is a directory\n", BIOS_RED);
                    break;
                case 2:
                    put("': Buffer size is not enough\n", BIOS_RED);
                    break;
                case 3:
                    put("': No such file or directory\n", BIOS_RED);
                    break;
            }
        } else {
            put((char *) &cl, BIOS_BROWN);
        }
    }
}

void cat (char* args_val, int (*args_info)[2], int args_count) {
    if (args_count < 2) {
        put("cat: missing operand\n", BIOS_RED);
    } else {
        for (int i = 1; i < args_count; i++) {
            showFiles (args_val, args_info, i);
        }
    }
}