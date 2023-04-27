// File : rm.c
// Contains the implementation of functions needed to process rm command

#include "rm.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"
#include "std/string.h"

void remove(char* args_val, int (*args_info)[2], int args_count) {
    // Variables to keep track the currently visited directory
    uint32_t dest_search_directory_number = ROOT_CLUSTER_NUMBER;
    char* name = "\0\0\0\0\0\0\0\0";

    // Variables for parsing the arguments
    int posName = (*(args_info + args_count))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + 1))[0] + (*(args_info + 1))[1];
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
                // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
                if (lenName > 8) {
                    errorCode = 2;
                    directoryNotFound = TRUE;
                    endOfArgs = TRUE;
                } else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                } else {
                    clear(name, 8);
                    memcpy(name, args_val + posName, lenName);
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

        if (directoryNotFound) {
            put("rm: cannot access '", BIOS_RED);
            putn(args_val + (*(args_info + args_count))[0], BIOS_RED, (*(args_info + args_count))[1]); 
            switch (errorCode) {
            case 1:
                put("': Not a directory\n", BIOS_RED);
                break;
            case 2:
                put("': Directory name is too long\n", BIOS_RED);
                break;
            default:
                put("': No such file or directory\n", BIOS_RED);
                break;
            }
        }
        else {
            // File 
            struct FAT32DriverRequest destReq = {
                .buf = 0,
                .name = "\0\0\0\0\0\0\0\0",
                .ext = "\0\0\0",
                .parent_cluster_number = dest_search_directory_number,
                .buffer_size = 0
            };
            memcpy(&(destReq.name), name, 8);
            uint32_t retCode;
            interrupt(3, (uint32_t) &destReq, (uint32_t) &retCode, 0x0);
            
            if (retCode == 2) {
                put("rm: Cannot remove, folder is not empty\n", BIOS_RED);
            }
        }
    }

}

void rm(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count >= 2) {
        for(int i=1; i<args_count; i++) {
            remove(args_val, args_info, i);
        }
    }  else {
        put("rm: missing file operand\n", BIOS_RED);
    }
}