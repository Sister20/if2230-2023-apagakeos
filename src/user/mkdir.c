// File : mkdir.c
// Contains the implementation of functions needed to process mkdir command

#include "mkdir.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void createDirectory(char* args_val, int (*args_info)[2], int args_pos) {
// Variables to keep track the currently visited directory
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    char* name = "\0\0\0\0\0\0\0\0";

    // Variables for parsing the arguments
    int posName = (*(args_info + args_pos))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + args_pos))[0] + (*(args_info + args_pos))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool newFileFound = FALSE;

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
            }
            else {
                // If there is a new word after non-existent directory, set an error code and stop parsing
                if (newFileFound) {
                    newFileFound = FALSE;
                    if (errorCode == 5) {
                        errorCode = 1;
                    }
                    else {
                        errorCode = 4;
                    }
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
                    endOfArgs = TRUE;
                }
                else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(search_directory_number);
                }
                else {
                    clear(name, 8);
                    memcpy(name, args_val + posName, lenName);
                    entry_index = findEntryName(name);
                    if (entry_index == -1) {
                        newFileFound = TRUE;
                    }
                    else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                            updateDirectoryTable(search_directory_number);
                        }
                        else {
                            newFileFound = TRUE;
                            errorCode = 5;
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

    if (!newFileFound) {
        put("mkdir: cannot create directory '", BIOS_RED);
        putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
        switch (errorCode) {
        case 0:
        case 5:
            put("': File exist\n", BIOS_RED);
            break;
        case 1:
            put("': Not a directory\n", BIOS_RED);
            break;
        case 3:
            put("': Argument name is too long\n", BIOS_RED);
            break;
        case 4:
            put("': No such file or directory\n", BIOS_RED);
            break;
        }
    }
    else {
        struct FAT32DriverRequest request = {
            .buf = 0,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = search_directory_number,
            .buffer_size = 0
        };
        memcpy(&(request.name), args_val + posName, lenName);

        interrupt(2, (uint32_t) &request, (uint32_t) &errorCode, 0x0);
        if (errorCode != 0) {
            put("mkdir: cannot create directory '", BIOS_RED);
            putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
            switch (errorCode) {
            case 1:
                put("': File exist\n", BIOS_RED);
                break;
            case -1:
                put("': Unknown error occured\n", BIOS_RED);
                break;
            }
        }
    }
}

void mkdir(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count < 2) {
        put("mkdir: missing operand\n", BIOS_RED);
    }
    else {
        for (int i = 1; i < args_count; i++) {
            createDirectory(args_val, args_info, i);
        }
    }
}