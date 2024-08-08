#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define RAM_SIZE 4096

bool ZFLAG = 0;

// Macro to swap bytes of a 16-bit word
#define SWAP_BYTES(word) (((word) >> 8) | ((word) << 8))
#define MAX_SYMBOLS 100
#define SET_ZERO_FLAG (ZFLAG = (A == 0) ? 1 : 0) 

typedef struct {
    uint16_t value;
    char name[50];
} Symbol;

static Symbol symbols[MAX_SYMBOLS];
static size_t symbol_count = 0;

// Function to load symbols from a file
int load_symbols(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open symbols file");
        return -1;
    }

    symbol_count = 0; // Reset symbol count

    while (fscanf(file, "%s = %hx", symbols[symbol_count].name, &symbols[symbol_count].value) == 2) {
        symbol_count++;
        if (symbol_count >= MAX_SYMBOLS) {
            fprintf(stderr, "Warning: Maximum symbol limit reached\n");
            break;
        }
    }

    fclose(file);
    return 0;
}

// Function to get the symbol name for a given value
const char* get_symbol_name(uint16_t value) {
    for (size_t i = 0; i < symbol_count; ++i) {
        if (symbols[i].value == value) {
            return symbols[i].name;
        }
    }
    return "Unknown";
}


// optional for single step (TODO add CLI option)
void wait_for_keypress() {
    struct termios oldt, newt;

    // Get the terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("Press any key to continue...\n");

    getchar();

    // Restore the old terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

}

    
void dumpBuffer(uint16_t* buffer)
{
    for (uint16_t i = 0; i < 48; i++) {
        if (i % 16 == 0) {
            printf("\n %04x | ", i);
        }
        printf("%04x ", buffer[i]);
    }
    printf("\n");
}


int main(int argc, char *argv[]) 
{
    // Ensure the correct number of command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file;
    uint16_t RAM[RAM_SIZE];
    size_t words_read;

    // Open the file in binary read mode
    file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    // Read up to 4096 words (each 16 bits) into the data array
    words_read = fread(RAM, sizeof(uint16_t), RAM_SIZE, file);
    if (words_read < RAM_SIZE) {
        if (feof(file)) {
            printf("End of file reached, read %zu words\n", words_read);
        } else {
            perror("Failed to read the expected amount");
            fclose(file);
            return EXIT_FAILURE;
        }
    } else {
        printf("Successfully read 4096 words\n");
    }

    // Close the file
    fclose(file);
    for (size_t i = 0; i < words_read; i++) {
        RAM[i] = SWAP_BYTES(RAM[i]);
    }

    load_symbols(argv[2]);
    
    uint32_t steps = 0;
    uint32_t PC = 0;
    #define IR          RAM[PC]
    #define MAX_STEPS   1024
    
    uint16_t A, i;
    uint8_t ins;
    uint16_t IMM;


    bool running = true;
    
    while (running && steps < MAX_STEPS) {
        printf("_______________________________________________________________________\n");
        
        ins = (IR >> 12) & 0xf;
        IMM = IR & 0xfff;
        printf("step = %i PC = %04X[%04X]  ins = %01X  IMM = %02X\n", steps, PC, RAM[PC], ins, IMM);

        switch (ins) {
            case 0x0:
                printf("loadm %s\n",get_symbol_name(IMM));
                A = RAM[IMM];
                break;
            case 0x1:
                printf("loadp %s\n",get_symbol_name(IMM));
                i = RAM[IMM];
                A = RAM[i];
                break;
            case 0x2:
                printf("storep %s\n",get_symbol_name(IMM));
                i = RAM[IMM];
                RAM[i] = A;
                break;
            case 0x3:
                printf("load %04x\n",IMM);
                A = IMM;
                break;
            case 0x4:
                printf("store %s\n",get_symbol_name(IMM));
                RAM[IMM] = A;
                break;
            case 0x5:
                printf("add %s\n",get_symbol_name(IMM));
                A += RAM[IMM];
                SET_ZERO_FLAG;
                break;
            case 0x6:
                printf("unrecognised instruction\n");
                break;
            case 0x7:
                printf("and %s\n",get_symbol_name(IMM));
                A = A & RAM[IMM];
                SET_ZERO_FLAG;
                break;
            case 0x8:
                printf("or %s\n",get_symbol_name(IMM));
                A = A | RAM[IMM];
                SET_ZERO_FLAG;
                break;
            case 0x9:
                printf("xor %s\n",get_symbol_name(IMM));
                A = A ^ RAM[IMM];
                SET_ZERO_FLAG;
                break;
            case 0xa:
                printf("lsr\n");
                A = A >> 1;
                SET_ZERO_FLAG;
                break;
            case 0xb:
                printf("jmp %s\n",get_symbol_name(IMM));
                if (PC == IMM) {
                    running = false;
                }
                PC = IMM - 1;
                break;
            case 0xc:
                printf("jmpZ %s\n",get_symbol_name(IMM));
                if (ZFLAG) PC = IMM - 1; // -1 because will get incremented
                break;
            case 0xd:
                printf("unrecognised instruction\n");
                break;
            case 0xe:
                printf("jsr %s\n",get_symbol_name(IMM));
                RAM[IMM] = PC;
                PC = IMM; // PC inc will get us past the return address
                break;
            case 0xf:
                printf("jmpi %s\n",get_symbol_name(IMM));
                PC = RAM[IMM]; // inc will get us to next instruction after jsr
                break;

        }
        printf("A = %04X Z%u \n", A, ZFLAG);
        dumpBuffer(&RAM[0]);
        //wait_for_keypress();
        PC += 1;
        PC = PC & 0xFFF;
        steps++;
        
    }

    return EXIT_SUCCESS;
}

