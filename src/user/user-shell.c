#include "user-shell.h"

int inputparse (char *input, char output[4][2], bool *valid) {
    // If empty, then return 0
    if (input[0] == 0x0A) {
        *valid = FALSE;
        return 0;
    }

    // Declare the vars
    int nums = 0;

    // Process to count the args, initialize 0
    int i = 0;
    int j = 0;
    int k = 0;
    *valid = TRUE;

    bool endWord = TRUE;
    bool startWord = TRUE;
    int countchar = 0;

    // Iterate all over the chars
    // Ignore blanks at first
    while (input[i] == ' ' && input[i] != 0x0A) {
        i++;
    }

    // While belum eof
    while (input[i] != 0x0A) {
        // Ignore blanks
        while (input[i] == ' ' && input[i] != 0x0A) {
            if (!endWord) {
                k = 0;
                j++;
                endWord = TRUE;
            }
            startWord = TRUE;
            i++;
        }

        // Return the number of args
        if (input[i] == 0x0A) {
            return nums;
        }

        // Out then it is not the end of the word
        endWord = FALSE;

        // Process other chars
        if (startWord) {
            nums++;
            countchar = 0;
            output[j][k] = i;
            startWord = FALSE;
            k++;
        }

        countchar++;
        output[j][k] = countchar;
        i++; // Next char
    }

    return nums;
}

// Interrupt to main
void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// Some function using the base interrupt
void put (char* buf, uint8_t color) {
    interrupt (5, (uint32_t) buf, strlen(buf), color);
}

// Print Current Working Directory
void printCWD (char* path_str, uint32_t current_dir) {
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

int main(void) {
    // The buffers
    char input_buff[2048];
    char input_split[4][2];
    char path_str[2048];
    bool valid = FALSE;

    while (TRUE) {
        // Always start by clearing the buffer
        clear(input_buff, 2048);
        for (int i = 0; i < 4; i++) {
            clear(input_split[i], 2);
        }
        clear(path_str, 2048);

        // Initialize
        put("ApaGaKeOS@OS-IF2230", BIOS_LIGHT_GREEN);
        put(":", BIOS_GREY);
        printCWD(path_str, current_directory);
        put("$ ", BIOS_GREY);
        
        // Asking for inputs
        interrupt (4, (uint32_t) input_buff, 2048, 0x0);

        // Get the numbers of input args
        int count = inputparse (input_buff, input_split, &valid);
        if (valid) {
            if (count == 1) {
                if (memcmp(input_buff + input_split[0][0], "ls", input_split[0][1]) == 0) {
                    // Process ls here
                }
            } else if (count == 2) {
                if (memcmp(input_buff + input_split[0][0], "cd", input_split[0][1]) == 0) {
                    // Process cd here
                } else if (memcmp(input_buff + input_split[0][0], "mkdir", input_split[0][1]) == 0) {
                    // Process mkdir here
                }
            } else {
                put("Your command is not valid!\n", BIOS_RED);
            }
        } else {
            put("Your command is not valid!\n", BIOS_RED);
        }
        
    }

    return 0;
}