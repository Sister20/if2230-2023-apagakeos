#include "user-shell.h"

// Some string and pointer processing libs
size_t strlen(char *string) {
    size_t i = 0;
    while (string[i] != '\0')
        i++;
    return i;
}

void clear(void *pointer, size_t n) {
    uint8_t *ptr       = (uint8_t*) pointer;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = 0x00;
    }
}

void strcpy(char *dst, char *src, int type) {
    size_t i = 0;
    if (type == 1) {
        while (src[i] != '\0') {
            dst[i] = src[i];
            i++;
        }
    } else {
        while (src[i] != 0xA) {
            dst[i] = src[i];
            i++;
        }
    }
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *buf1 = (const uint8_t*) s1;
    const uint8_t *buf2 = (const uint8_t*) s2;
    for (size_t i = 0; i < n; i++) {
        if (buf1[i] < buf2[i])
            return -1;
        else if (buf1[i] > buf2[i])
            return 1;
    }

    return 0;
}

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


int main(void) {
    // Start working directory
    // uint32_t current_directory = ROOT_CLUSTER_NUMBER;

    // The buffers
    char input_buff[2048];
    char input_split[4][2];
    // char path_str[256];
    char command[64];
    bool valid = FALSE;
    char args[64];

    while (TRUE) {
        clear(input_buff, 2048);
        for (int i = 0; i < 4; i++) {
            clear(input_split[i], 2);
        }
        clear(command, 64);
        clear(args, 64);
        // Initialize
        put("ApaGaKeOS@OS-IF2230", BIOS_LIGHT_GREEN);
        put(":", BIOS_GREY);
        // cwd - current woring directory
        put("/", BIOS_LIGHT_BLUE);
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