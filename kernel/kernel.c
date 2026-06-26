/* kernel.c - Our kernel entry point */

/* VGA text mode buffer is at memory address 0xB8000
   Each character = 2 bytes: [ASCII code] [color] */

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define WHITE_ON_BLACK 0x0F

unsigned short* vga = (unsigned short*)VGA_ADDRESS;
int cursor = 0;

void putchar(char c) {
    if (c == '\n') {
        cursor += VGA_WIDTH - (cursor % VGA_WIDTH);
        return;
    }
    vga[cursor++] = (WHITE_ON_BLACK << 8) | c;
}

void print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (WHITE_ON_BLACK << 8) | ' ';
    }
    cursor = 0;
}

void kernel_main() {
    clear_screen();
    print("myOS kernel loaded!\n");
    print("Welcome to myOS v0.1\n");
    print("\nHello from 32-bit Protected Mode!");
}