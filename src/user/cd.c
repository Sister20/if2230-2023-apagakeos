// File : cd.c
// Contains the implementation of functions needed to process cd command

#include "cd.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void cd(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count > 2) {
        put("cd: too many arguments\n", BIOS_RED);
    }
    else if (args_count == 2) {
        uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
        char* name = "\0\0\0\0\0\0\0\0";

        // Variables for parsing the arguments
        int posName = (*(args_info + 1))[0];
        int lenName = 0;
        int index = posName;
        int entry_index = -1;

        int posEndArgs = (*(args_info + 1))[0] + (*(args_info + 1))[1];
        bool endOfArgs = (posName+lenName-1 == posEndArgs);
        bool endWord = TRUE;

        int errorCode = 0;

        // If path is not absolute, set the currently visited directory to current working directory
        if (!isPathAbsolute(args_val, args_info, 1)) {
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
                    endWord = FALSE;
                    posName = index;
                    lenName = 1;
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
                            errorCode = 1;
                            endOfArgs = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(search_directory_number);
                            }
                            else {
                                errorCode = 2;
                                endOfArgs = TRUE;
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

        if (errorCode != 0) {
            put("cd: ", BIOS_RED);
            putn(args_val + (*(args_info + 1))[0], BIOS_RED, (*(args_info + 1))[1]); 
            switch (errorCode) {
            case 1:
                put(": No such file or directory\n", BIOS_RED);
                break;
            case 2:
                put(": Not a directory\n", BIOS_RED);
                break;
            case 3:
                put(": Argument name is too long\n", BIOS_RED);
            }
        }
        else {
            current_directory = search_directory_number;
        }
    }
}