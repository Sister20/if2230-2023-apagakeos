// File : user-shell.c
// Contains the implementation of functions needed to run shell program

#include "user-shell.h"
#include "mkdir.h"

uint32_t current_directory = ROOT_CLUSTER_NUMBER;
struct FAT32DirectoryTable dir_table;

/* ==================================================== SYSCALL INTERRUPT ==================================================== */

// Interrupt to main
void interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

/* ========================================================= PARSER ========================================================= */
int inputparse (char *args_val, char args_info[4][2]) {
    // Declare the vars
    int nums = 0;

    // Process to count the args, initialize 0
    int i = 0;
    int j = 0;
    int k = 0;

    bool endWord = TRUE;
    bool startWord = TRUE;
    int countchar = 0;

    // Iterate all over the chars
    // Ignore blanks at first
    while (args_val[i] == ' ' && args_val[i] != 0x0A) {
        i++;
    }

    // While belum eof
    while (args_val[i] != 0x0A) {
        // Ignore blanks
        while (args_val[i] == ' ' && args_val[i] != 0x0A) {
            if (!endWord) {
                k = 0;
                j++;
                endWord = TRUE;
            }
            startWord = TRUE;
            i++;
        }

        // Return the number of args
        if (args_val[i] == 0x0A) {
            return nums;
        }

        // Out then it is not the end of the word
        endWord = FALSE;

        // Process other chars
        if (startWord) {
            nums++;
            countchar = 0;
            args_info[j][k] = i;
            startWord = FALSE;
            k++;
        }

        countchar++;
        args_info[j][k] = countchar;
        i++; // Next char
    }

    return nums;
}

/* ========================================================= PRINTER ========================================================= */

// Wrapper for the base interrupt
void put(char* buf, uint8_t color) {
    interrupt(5, (uint32_t) buf, strlen(buf), color);
}

// Wrapper for the base interrupt, have additional argument n
void putn(char* buf, uint8_t color, int n) {
    interrupt(5, (uint32_t) buf, n, color);
}

// Print Current Working Directory
void printCWD(char* path_str, uint32_t current_dir) {
    // Biasakan untuk clear dulu
    clear(path_str, 128);

    // Intantiate length vars
    int pathlen = 0;

    if (current_dir == ROOT_CLUSTER_NUMBER) {
        path_str[pathlen++] = '/';
        put (path_str, BIOS_LIGHT_BLUE);
        return;
    }
}

/* ======================================================== PATHING ======================================================== */

// Check if the path argument is an absolute path or not
bool isPathAbsolute(char* args_val, char (*args_info)[2]) {
    return (memcmp(args_val + (*(args_info + 1))[0], "/", 1) == 0);
}

// Update the dir_table according to the cluster number
void updateDirectoryTable(uint32_t cluster_number) {
    interrupt(6, (uint32_t) &dir_table, cluster_number, 0x0);
}

// Find the name of directory in the dir_table and return its cluster number
int findDirectoryNumber(char* args_val, int position, int length) {
    int result = -1;

    int i = 1;
    bool found = FALSE;
    while (i < 64 && !found) {
        if (memcmp(dir_table.table[i].name, args_val + position, length) == 0 && 
            dir_table.table[i].user_attribute ==UATTR_NOT_EMPTY &&
            dir_table.table[i].attribute == ATTR_SUBDIRECTORY) {
            result = (int) ((dir_table.table[i].cluster_high << 16) | dir_table.table[i].cluster_low);
            found = TRUE;
        }
        else {
            i++;
        }
    }

    return result;
}

/* ======================================================= MAIN FUNCTION ======================================================= */

// the main function where shell run
int main(void) {
    // The buffers
    char args_val[2048];
    char args_info[4][2];
    char path_str[2048];

    // Request section
    struct ClusterBuffer cl           = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "ikanaide",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };

    while (TRUE) {
        // Always start by clearing the buffer
        clear(args_val, 2048);
        for (int i = 0; i < 4; i++) {
            clear(args_info[i], 2);
        }
        clear(path_str, 2048);

        // Initialize
        put("ApaGaKeOS@OS-IF2230", BIOS_LIGHT_GREEN);
        put(":", BIOS_GREY);
        printCWD(path_str, current_directory);
        put("$ ", BIOS_GREY);
        
        // Asking for inputs
        interrupt (4, (uint32_t) args_val, 2048, 0x0);

        // Get the numbers of input args
        int args_count = inputparse (args_val, args_info);
        
        // processing the command
        if ((memcmp(args_val + *(args_info)[0], "cd", 2) == 0) && ((*(args_info))[1] == 2)) {
            // TODO
        }
        else if ((memcmp(args_val + *(args_info)[0], "ls", 2) == 0) && ((*(args_info))[1] == 2)) {
            // TODO
        }
        else if ((memcmp(args_val + *(args_info)[0], "mkdir", 5) == 0)&& ((*(args_info))[1] == 5)) {
            mkdir(args_val, args_info, args_count);
        }
        else if ((memcmp(args_val + *(args_info)[0], "cat", 3) == 0)&& ((*(args_info))[1] == 3)) {
            int32_t retcode;
            if (args_count == 2) {
                memcpy(request.name, args_val + args_info[1][0], args_info[1][1]);
                request.buffer_size = CLUSTER_SIZE;
                interrupt(0, (uint32_t) &request, (uint32_t) &retcode, 0);
                if (retcode == 0) {
                    put((char *) request.buf, BIOS_BROWN);
                } else if (retcode == 1) {
                    put("cat: ", BIOS_RED);
                    for (char i = 0; i < args_info[1][1]; i++) {
                        putn((args_val + args_info[1][0] + i), BIOS_RED, 1);
                    }
                    put(": Can not open\n", BIOS_RED);
                } else if (retcode == 2) {
                    put("cat: Buffer size is not enough!\n", BIOS_RED);
                } else if (retcode == 3) {
                    put("cat: ", BIOS_RED);
                    for (char i = 0; i < args_info[1][1]; i++) {
                        putn((args_val + args_info[1][0] + i), BIOS_RED, 1);
                    }
                    put(": No such file or directory\n", BIOS_RED);
                }
            } else if (args_count > 2) {
                put("cat: too many arguments\n", BIOS_RED);
            }
        }
        else if ((memcmp(args_val + *(args_info)[0], "cp", 2) == 0)&& ((*(args_info))[1] == 2)) {
            // TODO
        }
        else if ((memcmp(args_val + *(args_info)[0], "rm", 2) == 0)&& ((*(args_info))[1] == 2)) {
            // TODO
        }
        else if ((memcmp(args_val + *(args_info)[0], "mv", 2) == 0)&& ((*(args_info))[1] == 2)) {
            // TODO
        }
        else if ((memcmp(args_val + *(args_info)[0], "whereis", 7) == 0)&& ((*(args_info))[1] == 7)) {
            // TODO
        }
        else {
            for (char i = 0; i < (*(args_info))[1]; i++) {
                putn((args_val + *(args_info)[0] + i), BIOS_RED, 1);
            }
            put(": command not found\n", BIOS_RED);
        }
    }

    return 0;
}