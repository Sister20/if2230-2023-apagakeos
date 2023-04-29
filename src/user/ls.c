// File : ls.c
// Contains the implementation of functions needed to process ls command

#include "ls.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void printDirectoryTable() {
    for (int i = 1; i < 63; i++) {
        if (dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            put(dir_table.table[i].name, BIOS_LIGHT_BLUE);
            if (dir_table.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[i].ext) != 0) {
                put(".", BIOS_LIGHT_BLUE);
                putn(dir_table.table[i].ext, BIOS_LIGHT_BLUE, 3);
            }
            put("\n", BIOS_LIGHT_BLUE);
        }
    }
    if (dir_table.table[63].user_attribute == UATTR_NOT_EMPTY) {
        put(dir_table.table[63].name, BIOS_LIGHT_BLUE);
        if (dir_table.table[63].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[63].ext) != 0) {
            put(".", BIOS_LIGHT_BLUE);
            putn(dir_table.table[63].ext, BIOS_LIGHT_BLUE, 3);
        }
        put("\n", BIOS_LIGHT_BLUE);
    }
}

void access(char* args_val, int (*args_info)[2], int args_pos) {
    // Variables to keep track the currently visited directory
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    int oneArgFlag = args_pos;

    if (args_pos == -1) {
        search_directory_number = current_directory;
        updateDirectoryTable(search_directory_number);
        printDirectoryTable();
    }
    else {
        if (args_pos == 0) {
            args_pos++;
        }

        // Variables for parsing the arguments
        int posName = (*(args_info + args_pos))[0];
        int lenName = 0;
        int index = posName;
        int entry_index = -1;
        char* name = "\0\0\0\0\0\0\0\0";

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
                }
                else {
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
                            directoryNotFound = TRUE;
                            endOfArgs = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                search_directory_number =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(search_directory_number);
                            }
                            else {
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
            put("ls: cannot access '", BIOS_RED);
            putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
            switch (errorCode) {
            case 1:
                put("': Not a directory\n", BIOS_RED);
                break;
            case 3:
                put("': Directory name is too long\n", BIOS_RED);
                break;
            default:
                put("': No such file or directory\n", BIOS_RED);
                break;
            }
        }
        else {
            if (fileFound) {
                putn(args_val + (*(args_info + args_pos))[0], BIOS_WHITE, (*(args_info + args_pos))[1]);
                put("\n", BIOS_WHITE);
            }
            else { 
                if (oneArgFlag > 0) {
                    putn(args_val + (*(args_info + args_pos))[0], BIOS_WHITE, (*(args_info + args_pos))[1]);
                    put(":\n", BIOS_WHITE);
                }
                printDirectoryTable();
            }
        }
    }
}

void ls(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count > 2) {
        for (int i = 1; i < args_count-1; i++) {
            access(args_val, args_info, i);
            put("\n", BIOS_GREY);
        }
        access(args_val, args_info, args_count - 1);
        put("\n", BIOS_GREY);
    }
    else if (args_count == 2) {
        access(args_val, args_info, 0);
    }
    else {
        access(args_val, args_info, -1);
    }
}