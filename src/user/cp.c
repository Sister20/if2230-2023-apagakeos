// File : cp.c
// Contains the implementation of functions needed to process cp command

#include "cp.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"
#include "std/string.h"

void copy(char* args_val, int (*args_info)[2], int args_count) {
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
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i >= lenName) {
                        errorCode = 3;
                        endOfArgs = TRUE;
                    } else {
                        clear(destName, 8);
                        clear(destExt,3);
                        int i = 0;
                        while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                            i++;
                        }
                        if (i < lenName) { // Jika ada extension
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
                else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                }
                else {
                    clear(destName, 8);
                    clear(destExt,3);
                    int i = 0;
                    while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
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
        put("Destination not valid\n", BIOS_RED);
        return;
    }
    else if (args_count > 3 && newFileFound) { // Jika lebih dari satu file yang dicopy, maka harus dimasukkan ke dalam folder
        put("cp: target '", BIOS_RED);
        put(destName, BIOS_RED);
        put(".", BIOS_RED);
        put(destExt, BIOS_RED);
        put("' is not a directory\n", BIOS_RED);
        return;
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
                        int i = 0;
                        while (i < lenName && memcmp(".", args_val + posName + i, 1) != 0) {
                            i++;
                        }
                        if (i >= lenName) {
                            errorCode = 3;
                            endOfArgs = TRUE;
                        } else {
                            clear(srcName, 8);
                            clear(srcExt,3);
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

        if (errorCode == 3 || errorCode ==  4 || !srcNewFileFound) {
            put("cp: cannot stat '", BIOS_RED);
            put(srcName, BIOS_RED);
            if (srcExt[0] != 0x00) {
                put(".", BIOS_RED);
                put(srcExt, BIOS_RED);
            }
            put("': No such file or directory\n", BIOS_RED);
            return;
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
        memcpy(&(srcReq.ext), srcExt,3);
        uint32_t retCode;

        interrupt(0, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);

        if (retCode != 0) {
            put("cp: cannot stat '", BIOS_RED);
            put(srcName, BIOS_RED);
            if (srcExt[0] != 0x00) {
                put(".", BIOS_RED);
                put(srcExt, BIOS_RED);
            }
            switch (retCode) {
                case 1:
                    put("': Is a directory\n", BIOS_RED);
                    return;
                case 2:
                    put("': Buffer size is not enough\n", BIOS_RED);
                    return;
                case 3:
                    put("': No such file or directory\n", BIOS_RED);
                    return;
                case -1:
                    put("': Unknown error\n", BIOS_RED);
                    return;
            }
        } else {
            if (!newFileFound) {
                // Direktori
                srcReq.parent_cluster_number = dest_search_directory_number;
                interrupt(3, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);
                interrupt(2, (uint32_t) &srcReq, (uint32_t) &retCode, 0x0);
                if (retCode != 0) {
                    put("cp : cannot copy '", BIOS_RED);
                    put(srcName, BIOS_RED);
                    if (srcExt[0] != 0x00) {
                        put(".", BIOS_RED);
                        put(srcExt, BIOS_RED);
                    }
                    switch (retCode) {
                    case 1:
                        put("': File exist\n", BIOS_RED);
                        return;
                    case -1:
                        put("': Unknown error occured\n", BIOS_RED);
                        return;
                    }
                }
            } else {
                // File
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
                    put(srcName, BIOS_RED);
                    put(".", BIOS_RED);
                    put(srcExt, BIOS_RED);
                    switch (retCode) {
                    case 1:
                        put("': File exist\n", BIOS_RED);
                        return;
                    case -1:
                        put("': Unknown error occured\n", BIOS_RED);
                        return;
                    }
                }
            }
        }
    }
}

// int copy(char* args_val, int (*args_info)[2], char* destName, int args_count) {
//     // Variables to keep track the currently visited directory
//     uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
//     char* name = "\0\0\0\0\0\0\0\0";

//     // Variables for parsing the arguments
//     int posName = (*(args_info + args_pos))[0];
//     int lenName = 0;
//     int index = posName;
//     int entry_index = -1;

//     int posEndArgs = (*(args_info + args_pos))[0] + (*(args_info + args_pos))[1];
//     bool endOfArgs = (posName+lenName-1 == posEndArgs);
//     bool endWord = TRUE;
//     bool newFileFound = FALSE;

//     int errorCode = 0;

//     // If path is not absolute, set the currently visited directory to current working directory
//     if (!isPathAbsolute(args_val, args_info, args_pos)) {
//         search_directory_number = current_directory;
//     }

//     // Get the directory table of the visited directory
//     updateDirectoryTable(search_directory_number);

//     // Start searching for the directory to make 
//     while (!endOfArgs) {
//         // If current char is not '/', process the information of word. Else, process the word itself
//         if (memcmp(args_val + index, "/", 1) != 0 && index != posEndArgs) {
//             // If word already started, increment the length. Else, start new word
//             if (!endWord) {
//                 lenName++;
//             }
//             else {
//                 // If there is a new word after non-existent directory, set an error code and stop parsing
//                 if (newFileFound) {
//                     newFileFound = FALSE;
//                     if (errorCode == 5) {
//                         errorCode = 1;
//                     }
//                     else {
//                         errorCode = 4;
//                     }
//                     endOfArgs = TRUE;
//                 }
//                 else {
//                     endWord = FALSE;
//                     posName = index;
//                     lenName = 1;
//                 }
//             }
//         }
//         else {
//             // Process the word
//             if (!endWord) {
//                 // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
//                 if (lenName > 8) {
//                     errorCode = 3;
//                     endOfArgs = TRUE;
//                 }
//                 else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
//                     search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
//                     updateDirectoryTable(search_directory_number);
//                 }
//                 else {
//                     clear(name, 8);
//                     memcpy(name, args_val + posName, lenName);
//                     entry_index = findEntryName(name);
//                     if (entry_index == -1) {
//                         newFileFound = TRUE;
//                     }
//                     else {
//                         if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
//                             search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
//                             updateDirectoryTable(search_directory_number);
//                         }
//                         else {
//                             newFileFound = TRUE;
//                             errorCode = 5;
//                         }
//                     }
//                 }
//                 endWord = TRUE;
//             }
//         }

//         if (!endOfArgs) {
//             if (index == posEndArgs) {
//                 endOfArgs = TRUE;
//             }
//             else {
//                 index++;
//             }
//         }
//     }

//     if (!newFileFound) {
//         put("mkdir: cannot create directory '", BIOS_RED);
//         putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
//         switch (errorCode) {
//         case 0:
//         case 5:
//             put("': File exist\n", BIOS_RED);
//             break;
//         case 1:
//             put("': Not a directory\n", BIOS_RED);
//             break;
//         case 3:
//             put("': Argument name is too long\n", BIOS_RED);
//             break;
//         case 4:
//             put("': No such file or directory\n", BIOS_RED);
//             break;
//         }
//     }
//     else {
//         struct FAT32DriverRequest request = {
//             .buf = 0,
//             .name = "\0\0\0\0\0\0\0\0",
//             .ext = "\0\0\0",
//             .parent_cluster_number = search_directory_number,
//             .buffer_size = 0
//         };
//         memcpy(&(request.name), args_val + posName, lenName);

//         interrupt(2, (uint32_t) &request, (uint32_t) &errorCode, 0x0);
//         if (errorCode != 0) {
//             put("mkdir: cannot create directory '", BIOS_RED);
//             putn(args_val + (*(args_info + args_pos))[0], BIOS_RED, (*(args_info + args_pos))[1]); 
//             switch (errorCode) {
//             case 1:
//                 put("': File exist\n", BIOS_RED);
//                 break;
//             case -1:
//                 put("': Unknown error occured\n", BIOS_RED);
//                 break;
//             }
//         }
//     }
// }

// void copyMany(char* args_val, int (*args_info)[2], int args_count) {
//     uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
//     if (!isPathAbsolute(args_val, args_info, args_count-1)) {
//         search_directory_number = current_directory;
//     }
//     updateDirectoryTable(search_directory_number);
//     char* destDir = 
//     for (int i=1; i < args_count-1; i++) {

//     }
// }

void cp(char* args_val, int (*args_info)[2], int args_count) {
    if (args_count > 2) {
        copy(args_val, args_info, args_count);
    }
    else if (args_count == 2) {
        put("cp: missing destination file operand after '", BIOS_RED);
        putn(args_val + (*(args_info + 1))[0], BIOS_RED, (*(args_info + 1))[1]);
        put("'\n", BIOS_RED); 
    }
    else {
        put("cp: missing file operand\n", BIOS_RED);
    }
}