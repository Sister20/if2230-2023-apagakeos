// File : whereis.c
// Contains the implementation of functions needed to process whereis command

#include "whereis.h"
#include "user-shell.h"
#include "std/stdtype.h"
#include "std/stdmem.h"

void processDFS (char srcName[8], uint32_t search_directory_number, int v, bool visited[63]) {
    char path_list[2048];
    clear(path_list, 2048);
    // Kunjungi dulu simpulnya
    visited[v - 1] = TRUE;

    // define bool visied yang baru
    bool visitedNew [63];
    clear(visitedNew, 63);

    // Melakukan traversal terhadap dir table sekarang ke tetangganya
    for (int i = 1; i < 64; i++) {
        // Memastikan ada isinya, tidak kosong
        if (dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            // Kalau folder, salin trus traverse dalamnya
            if (dir_table.table[i].attribute == ATTR_SUBDIRECTORY) {
                // Cek apakah namanya sama, kalo sama cetak
                if (memcmp(dir_table.table[i].name, srcName, 8) == 0) {
                    printCWD(path_list, current_directory);
                    updateDirectoryTable(search_directory_number);
                    put("/", BIOS_LIGHT_BLUE);
                    put(srcName, BIOS_LIGHT_BLUE);
                    put("  ", BIOS_BLACK);
                }
                // Sama maupun tidak, proses pencarian tetap dilakukan
                if (!visitedNew[i - 1]) {
                    search_directory_number = (int) ((dir_table.table[i].cluster_high << 16) | dir_table.table[i].cluster_low);
                    current_directory = search_directory_number;
                    updateDirectoryTable(search_directory_number);
                    processDFS (srcName, search_directory_number, i, visitedNew);

                    // NAIK
                    visitedNew[i - 1] = TRUE;
                    search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    current_directory = search_directory_number;
                    updateDirectoryTable(search_directory_number);
                }
            }
            // Kalo bukan folder, cek, namanya sama apa ga
            else {
                // Cek apakah namanya sama, kalo sama cetak
                if (memcmp(dir_table.table[i].name, srcName, 8) == 0) {
                    printCWD(path_list, current_directory);
                    updateDirectoryTable(search_directory_number);
                    put("/", BIOS_LIGHT_BLUE);
                    put(srcName, BIOS_LIGHT_BLUE);
                    put(".", BIOS_LIGHT_BLUE);
                    put(dir_table.table[i].ext, BIOS_LIGHT_BLUE);
                    put("  ", BIOS_BLACK);
                }
                visited[i - 1] = TRUE;
                // kalo ga do nothing, set aja visitednya jad true
            }
        }
    }
}

void doWhereis (char* args_val, int (*args_info)[2], int args_pos) {
    // Variables to keep track the currently visited directory
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    bool visited[63];

    // Variables for parsing the arguments
    int posName = (*(args_info + args_pos))[0];
    int lenName = 0;
    int index = posName;

    int posEndArgs = (*(args_info + args_pos))[0] + (*(args_info + args_pos))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool fileFound = FALSE;

    int errorCode = 0;

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
                if (fileFound) {
                    fileFound = FALSE;
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
        } else {
            // Process the word
            if (!endWord) {
                // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
                if (lenName > 8) {
                    errorCode = 3;
                    endOfArgs = TRUE;
                } else if (lenName == 2 && memcmp(args_val + posName, "..", 2) == 0) {
                    lenName += 2;
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

    put(srcName, BIOS_WHITE);
    put(": ", BIOS_WHITE);

    char path_list[2048];
    clear(path_list, 2048);
    clear(visited, 64);

    for (int i = 1; i < 64; i++) {
        // Pastiin tidak kosong
        if (dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            // Kalau folder, salin trus traverse dalamnya
            if (dir_table.table[i].attribute == ATTR_SUBDIRECTORY) {
                // Cek apakah namanya sama, kalo sama cetak
                if (memcmp(dir_table.table[i].name, srcName, 8) == 0) {
                    printCWD(path_list, current_directory);
                    updateDirectoryTable(search_directory_number);
                    put(srcName, BIOS_LIGHT_BLUE);
                    put("  ", BIOS_BLACK);
                }
                // Sama maupun tidak, proses pencarian tetap dilakukan
                if (!visited[i - 1]) {
                    search_directory_number = (int) ((dir_table.table[i].cluster_high << 16) | dir_table.table[i].cluster_low);
                    current_directory = search_directory_number;
                    updateDirectoryTable(search_directory_number);
                    processDFS (srcName, search_directory_number, i, visited);

                    // NAIK
                    visited[i - 1] = TRUE;
                    search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    current_directory = search_directory_number;
                    updateDirectoryTable(search_directory_number);
                }
            }
            // Kalo bukan folder, cek, namanya sama apa ga
            else {
                // Cek apakah namanya sama, kalo sama cetak
                if (memcmp(dir_table.table[i].name, srcName, 8) == 0) {
                    printCWD(path_list, current_directory);
                    updateDirectoryTable(search_directory_number);
                    put(srcName, BIOS_LIGHT_BLUE);
                    put(".", BIOS_LIGHT_BLUE);
                    put(dir_table.table[i].ext, BIOS_LIGHT_BLUE);
                    put("  ", BIOS_BLACK);
                }
                visited[i - 1] = TRUE;
                // kalo ga do nothing, set aja visitednya jad true
            }
        }
    }

    put("\n\n", BIOS_WHITE);
}

void whereis (char* args_val, int (*args_info)[2], int args_count) {
    if (args_count < 2) {
        put ("whereis: missing operand\n", BIOS_RED);
    } else {
        for (int i = 1; i < args_count; i++) {
            doWhereis (args_val, args_info, i);
        }
    }
}