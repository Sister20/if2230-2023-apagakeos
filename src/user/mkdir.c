// File : mkdir.c
// Contains the implementation of functions needed to process mkdir command

#include "mkdir.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void mkdir(char* args_val, char (*args_info)[2], int args_count) {
    // Variables to keep track the currently visited directory
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    int temp_directory_number = 0;

    // Variables for parsing the arguments
    int posName = (*(args_info + 1))[0];
    int lenName = 0;
    int index = posName;

    int posEndArgs1 = (*(args_info + 1))[0] + (*(args_info + 1))[1];
    bool endOfArgs1 = (posName+lenName-1 == posEndArgs1);
    bool endWord = TRUE;
    bool newDirectoryFound = FALSE;

    int errorCode = 0;

    // If there are more than 2 arguments, reject the mkdir command
    if (args_count != 2) {
        put("mkdir: too many arguments\n", BIOS_RED);
    }
    else {
        // If path is not absolute, set the currently visited directory to current working directory
        if (!isPathAbsolute(args_val, args_info)) {
            search_directory_number = current_directory;
        }

        // Get the directory table of the visited directory
        updateDirectoryTable(search_directory_number);

        // Start searching for the directory to make 
        while (!endOfArgs1) {
            // If current char is not '/', process the information of word. Else, process the word itself
            if (memcmp(args_val + index, "/", 1) != 0 && index != posEndArgs1) {
                // If word already started, increment the length. Else, start new word
                if (!endWord) {
                    lenName++;
                }
                else {
                    // If there is a new word after non-existent directory, set an error code and stop parsing
                    if (newDirectoryFound) {
                        newDirectoryFound = FALSE;
                        errorCode = 4;
                        endOfArgs1 = TRUE;
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
                        endOfArgs1 = TRUE;
                    }
                    else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                        search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                        updateDirectoryTable(search_directory_number);
                    }
                    else {
                        temp_directory_number = findDirectoryNumber(args_val, posName, lenName);
                        if (temp_directory_number == -1) {
                            newDirectoryFound = TRUE;
                        }
                        else {
                            search_directory_number = temp_directory_number;
                            updateDirectoryTable(search_directory_number);
                        }
                    }
                    endWord = TRUE;
                }
            }

            if (!endOfArgs1) {
                if (index == posEndArgs1) {
                    endOfArgs1 = TRUE;
                }
                else {
                    index++;
                }
            }
        }

        if (!newDirectoryFound) {
            put("mkdir: cannot create directory '", BIOS_RED);
            putn(args_val + (*(args_info + 1))[0], BIOS_RED, (*(args_info + 1))[1]); 
            switch (errorCode) {
            case 0:
                put("': Folder exist\n", BIOS_RED);
                break;
            case 3:
                put("': Folder name is too long\n", BIOS_RED);
                break;
            case 4:
                put("': No such file or directory\n", BIOS_RED);
                break;
            }
        }
        else {
            struct FAT32DriverRequest request = {
                .buf = 0,
                .name = "\0\0\0\0\0\0\0",
                .ext = "\0\0\0",
                .parent_cluster_number = search_directory_number,
                .buffer_size = 0
            };
            memcpy(&(request.name), args_val + posName, lenName);

            interrupt(2, (uint32_t) &request, (uint32_t) &errorCode, 0x0);
            if (errorCode != 0) {
                put("mkdir: cannot create directory '", BIOS_RED);
                putn(args_val + (*(args_info + 1))[0], BIOS_RED, (*(args_info + 1))[1]); 
                switch (errorCode) {
                case 1:
                    put("': Folder exist\n", BIOS_RED);
                    break;
                case -1:
                    put("': Unknown error occured\n", BIOS_RED);
                    break;
                }
            }
        }
    }
}

