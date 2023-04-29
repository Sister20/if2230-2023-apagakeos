// File : mv.c
// Contains the implementation of functions needed to process mv command

#include "mv.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

bool remove_mv(char* args_val, int (*args_info)[2], int args_count) {
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
                    directoryNotFound = TRUE;
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i >= lenName) {
                        endOfArgs = TRUE;
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
                            fileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(dest_search_directory_number);
                            }
                            else {
                                fileFound = TRUE;
                            }
                        }
                    }
                    endWord = TRUE;
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

    struct FAT32DriverRequest destReq = {
            .buf = 0,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = dest_search_directory_number,
            .buffer_size = 0
    };
    memcpy(&(destReq.name), name, 8);
    uint32_t retCode;

    if (directoryNotFound) {
        // Check if it is a file or it doesnt exist
        interrupt(3, (uint32_t) &destReq, (uint32_t) &retCode, 0x0);
        if (retCode != 0) {
            put("mv: cannot delete '", BIOS_RED);
            putn(args_val + (*(args_info + args_count))[0], BIOS_RED, (*(args_info + args_count))[1]); 
            switch (retCode) {
            case 1:
                put("': File not found\n", BIOS_RED);
                break;
            default:
                put("': No such file or directory\n", BIOS_RED);
                break;
            }
            return FALSE;
        }
        return TRUE;
    }
    else {
        // Directory
        destReq.parent_cluster_number = ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
        interrupt(3, (uint32_t) &destReq, (uint32_t) &retCode, 0x0);
        if (retCode != 0) {
            put("mv: cannot delete '", BIOS_RED);
            putn(args_val + (*(args_info + args_count))[0], BIOS_RED, (*(args_info + args_count))[1]); 
            switch (retCode) {
            case 1:
                put("': Directory not found\n", BIOS_RED);
                break;
            case 2:
                put("': Directory not empty\n", BIOS_RED);
                break;
            default:
                put("': No such file or directory\n", BIOS_RED);
                break;
            }
            return FALSE;
        }
        return TRUE;
    }
}

bool copy_mv(char* args_val, int (*args_info)[2], int args_count) {
    /* Searches if the destination exists and if it is a file or directory.
       Returns 1 if it is a file, 0 if it is a directory, -1 if it is not found */

    // Variables to keep track the currently visited directory
    uint32_t dest_search_directory_number = ROOT_CLUSTER_NUMBER;
    char destName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char destExt[3] = {'\0','\0','\0'};

    // Variables for parsing the arguments
    int posName = (*(args_info + args_count-1))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + args_count-1))[0] + (*(args_info + args_count-1))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool newFileFound = FALSE;

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
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i >= lenName) {
                        errorCode = 3;
                        endOfArgs = TRUE;
                    } else if (lenName-i-1 > 3) {
                        errorCode = 3;
                        endOfArgs = TRUE;
                    } else {
                        clear(destName, 8);
                        clear(destExt,3);
                        memcpy(destName, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(destExt, args_val + posName + i + 1, lenName-i-1);
                        }
                        entry_index = findEntryName(destName);
                        if (entry_index == -1) {
                            newFileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(dest_search_directory_number);
                            }
                            else {
                                newFileFound = TRUE;
                                errorCode = 5;
                            }
                        }
                    }
                    endWord = TRUE;
                }
                else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                }
                else {
                    clear(destName, 8);
                    clear(destExt,3);
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
                        if (lenName-i-1 > 3) { // Jika extension lebih dari 3 karakter
                            errorCode = 3;
                            break;
                        }   
                        memcpy(destName, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(destExt, args_val + posName + i + 1, lenName-i-1);
                        }
                    } else {
                        memcpy(destName, args_val + posName, lenName);
                    }
                    entry_index = findEntryName(destName);
                    if (entry_index == -1) {
                        newFileFound = TRUE;
                    }
                    else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                            updateDirectoryTable(dest_search_directory_number);
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

    if (errorCode == 3 || errorCode ==  4) {
        put("mv : Destination not valid\n", BIOS_RED);
        return FALSE;
    }
    else if (args_count > 3 && newFileFound) { // Jika lebih dari satu file yang dicopy, maka harus dimasukkan ke dalam folder
        put("mv: target '", BIOS_RED);
        putn(args_val + (*(args_info + args_count-1))[0], BIOS_RED, (*(args_info + args_count-1))[1]);
        put("' is not a directory\n", BIOS_RED);
        return FALSE;
    }

    // Read each files that need to be copied
    for (int j=1; j<args_count-1; j++) {
        // Variables to keep track the currently visited directory
        uint32_t src_search_directory_number = ROOT_CLUSTER_NUMBER;
        char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
        char srcExt[3] = {'\0','\0','\0'};

        // Variables for parsing the arguments
        posName = (*(args_info + j))[0];
        lenName = 0;
        index = posName;
        entry_index = -1;

        posEndArgs = (*(args_info + j))[0] + (*(args_info + j))[1];
        endOfArgs = (posName+lenName-1 == posEndArgs);
        endWord = TRUE;
        bool srcNewFileFound = FALSE;

        errorCode = 0;

        // If path is not absolute, set the currently visited directory to current working directory
        if (!isPathAbsolute(args_val, args_info, j)) {
            src_search_directory_number = current_directory;
        }

        // Get the directory table of the visited directory
        updateDirectoryTable(src_search_directory_number);

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
                    if (srcNewFileFound) {
                        srcNewFileFound = FALSE;
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
                        // Periksa extension
                        int i = 0;
                        while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                            i++;
                        }
                        clear(srcName, 8);
                        clear(srcExt,3);
                        if (i >= lenName) {
                            errorCode = 3;
                            endOfArgs = TRUE;
                        } 
                        else if (lenName-i-1 > 3) {
                            errorCode = 3;
                            endOfArgs = TRUE;
                        } 
                        else {
                            memcpy(srcName, args_val + posName, i);
                            if (*(args_val + posName + i + 1) != 0x0A) {
                                memcpy(srcExt, args_val + posName + i + 1, lenName-i-1);
                            }
                            entry_index = findEntryName(srcName);
                            if (entry_index == -1) {
                                srcNewFileFound = TRUE;
                            }
                            else {
                                if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                    src_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                    updateDirectoryTable(src_search_directory_number);
                                }
                                else {
                                    srcNewFileFound = TRUE;
                                    errorCode = 5;
                                }
                            }
                        }
                        endWord = TRUE;
                    }
                    else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                        src_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                        updateDirectoryTable(src_search_directory_number);
                    }
                    else {
                        clear(srcName, 8);
                        clear(srcExt,3);
                        // Periksa extension
                        int i = 0;
                        while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                            i++;
                        }
                        if (i < lenName) { // Jika ada extension
                            if (lenName-i-1 > 3) { // Jika extension lebih dari 3 karakter
                                errorCode = 3;
                                break;
                            }
                            memcpy(srcName, args_val + posName, i);
                            if (*(args_val + posName + i + 1) != 0x0A) {
                                memcpy(srcExt, args_val + posName + i + 1, lenName-i-1);
                            }
                        } else {
                            memcpy(srcName, args_val + posName, lenName);
                        }
                        entry_index = findEntryName(srcName);
                        if (entry_index == -1) {
                            srcNewFileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                src_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(src_search_directory_number);
                            }
                            else {
                                srcNewFileFound = TRUE;
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

        if (errorCode == 3 || errorCode ==  4) {
            put("mv: cannot stat '", BIOS_RED);
            putn(args_val + (*(args_info + j))[0], BIOS_RED, (*(args_info + j))[1]);
            put("': No such file or directory\n", BIOS_RED);
            return FALSE;
        }
        else if (!srcNewFileFound) {
            put("mv: '", BIOS_RED);
            putn(args_val + (*(args_info + j))[0], BIOS_RED, (*(args_info + j))[1]);
            put("' is a directory\n", BIOS_RED);
            return FALSE;
        }

        struct ClusterBuffer cbuf = {0};
        struct FAT32DriverRequest srcReq = {
            .buf = &cbuf,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = src_search_directory_number,
            .buffer_size = CLUSTER_SIZE
        };
        memcpy(&(srcReq.name), srcName, 8);
        memcpy(&(srcReq.ext), srcExt, 3);
        uint32_t retCode;

        interrupt(0, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);

        if (retCode != 0) {
            put("mv: cannot stat '", BIOS_RED);
            putn(args_val + (*(args_info + j))[0], BIOS_RED, (*(args_info + j))[1]);
            switch (retCode) {
                case 1:
                    put("': Is a directory\n", BIOS_RED);
                    break;
                case 2:
                    put("': Buffer size is not enough\n", BIOS_RED);
                    break;
                case 3:
                    put("': No such file or directory\n", BIOS_RED);
                    break;
                case -1:
                    put("': Unknown error\n", BIOS_RED);
                    break;
            }
            return FALSE;
        } else {
            if (!newFileFound) {
                // Tujuan berupa direktori
                srcReq.parent_cluster_number = dest_search_directory_number;
                interrupt(3, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);
                interrupt(2, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);
                if (retCode != 0) {
                    put("cp : cannot copy '", BIOS_RED);
                    putn(args_val + (*(args_info + j))[0], BIOS_RED, (*(args_info + j))[1]);
                    switch (retCode) {
                    case 1:
                        put("': File exist\n", BIOS_RED);
                        break;
                    case -1:
                        put("': Unknown error occured\n", BIOS_RED);
                        break;
                    }
                    return FALSE;
                }
            } else {
                // Tujuan berupa file
                struct FAT32DriverRequest destReq = {
                    .buf = &cbuf,
                    .name = "\0\0\0\0\0\0\0\0",
                    .ext = "\0\0\0",
                    .parent_cluster_number = dest_search_directory_number,
                    .buffer_size = CLUSTER_SIZE
                };
                memcpy(&(destReq.name), destName, 8);
                memcpy(&(destReq.ext), destExt, 3);
                interrupt(3, (uint32_t) &destReq, (uint32_t) &retCode, 0x0);
                interrupt(2, (uint32_t) &destReq, (uint32_t) &retCode, 0x0);
                if (retCode != 0) {
                    put("cp : cannot copy '", BIOS_RED);
                    putn(args_val + (*(args_info + j))[0], BIOS_RED, (*(args_info + j))[1]);
                    switch (retCode) {
                    case 1:
                        put("': File exist\n", BIOS_RED);
                        break;
                    case -1:
                        put("': Unknown error occured\n", BIOS_RED);
                        break;
                    }
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

int parse(char* args_val, int (*args_info)[2], int args_count) {
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
            }
            else {
                // If there is a new word after non-existent directory, set an error code and stop parsing
                if (fileFound) {
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
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i >= lenName) {
                        endOfArgs = TRUE;
                    } else if (lenName-i-1 > 3) {
                        endOfArgs = TRUE;
                    } else {
                        clear(name, 8);
                        clear(extension,3);
                        memcpy(name, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(extension, args_val + posName + i + 1, lenName-i-1);
                        }
                        entry_index = findEntryName(name);
                        if (entry_index == -1) {
                            fileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(dest_search_directory_number);
                            }
                            else {
                                fileFound = TRUE;
                            }
                        }
                    }
                    endWord = TRUE;
                }
                else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                }
                else {
                    clear(name, 8);
                    clear(extension,3);
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
                        if (lenName-i-1 > 3) { // Jika extension lebih dari 3 karakter
                            break;
                        }   
                        memcpy(name, args_val + posName, i);
                        if (*(args_val + posName + i + 1) != 0x0A) {
                            memcpy(extension, args_val + posName + i + 1, lenName-i-1);
                        }
                    } else {
                        memcpy(name, args_val + posName, lenName);
                    }
                    entry_index = findEntryName(name);
                    if (entry_index == -1) {
                        fileFound = TRUE;
                    }
                    else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                            updateDirectoryTable(dest_search_directory_number);
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

void rename(char* args_val, int (*args_info)[2], int args_count) {
    copy_mv(args_val, args_info, args_count);
    remove_mv(args_val, args_info, 2);
}

void move(char* args_val, int (*args_info)[2], int args_count) {
    copy_mv(args_val, args_info, args_count);
}

void mv(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count == 3) {
        if (parse(args_val, args_info, 1) == 1) {
            int parseRes = parse(args_val, args_info, 2);
            if (parseRes == 1) {
                rename(args_val, args_info, args_count);
            } else if (parseRes == 2) {
                move(args_val, args_info, args_count);
            }
        } else {
            put("mv: cannot overwrite non-directory with directory\n", BIOS_RED);
        }
    }  else {
        put("mv: missing file operand\n", BIOS_RED);
    }
}