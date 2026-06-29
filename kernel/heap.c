#include "heap.h"
#include "pmm.h"
#include "../drivers/vga.h"

/* Each block has a header */
struct heap_block {
    uint32_t size;              /* Size of data (not including header) */
    uint8_t  is_free;
    struct heap_block* next;
    struct heap_block* prev;
};

#define HEAP_PAGES   16                          /* 16 * 4KB = 64KB initial heap */
#define HEAP_SIZE    (HEAP_PAGES * PAGE_SIZE)
#define MIN_SPLIT    32                          /* Don't split if remainder < this */

static struct heap_block* heap_start = 0;
static uint8_t heap_memory[HEAP_SIZE];          /* Static heap region for now */

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    while (val > 0) {
        buf[--i] = '0' + (val % 10);
        val /= 10;
    }
    vga_print(&buf[i]);
}

void heap_init() {
    heap_start = (struct heap_block*)heap_memory;
    heap_start->size    = HEAP_SIZE - sizeof(struct heap_block);
    heap_start->is_free = 1;
    heap_start->next    = 0;
    heap_start->prev    = 0;

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Heap initialized | Size: ");
    print_int(HEAP_SIZE / 1024);
    vga_print(" KB\n");
}

void* kmalloc(uint32_t size) {
    if (size == 0) return 0;

    /* Align size to 4 bytes */
    size = (size + 3) & ~3;

    struct heap_block* block = heap_start;

    while (block) {
        if (block->is_free && block->size >= size) {
            /* Split block if large enough */
            if (block->size >= size + sizeof(struct heap_block) + MIN_SPLIT) {
                struct heap_block* new_block = (struct heap_block*)
                    ((uint8_t*)block + sizeof(struct heap_block) + size);

                new_block->size    = block->size - size - sizeof(struct heap_block);
                new_block->is_free = 1;
                new_block->next    = block->next;
                new_block->prev    = block;

                if (block->next) block->next->prev = new_block;
                block->next = new_block;
                block->size = size;
            }

            block->is_free = 0;
            return (void*)((uint8_t*)block + sizeof(struct heap_block));
        }
        block = block->next;
    }

    return 0;  /* Out of heap memory */
}

void kfree(void* ptr) {
    if (!ptr) return;

    struct heap_block* block = (struct heap_block*)
        ((uint8_t*)ptr - sizeof(struct heap_block));

    block->is_free = 1;

    /* Coalesce with next block if free */
    if (block->next && block->next->is_free) {
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next  = block->next->next;
        if (block->next) block->next->prev = block;
    }

    /* Coalesce with previous block if free */
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(struct heap_block) + block->size;
        block->prev->next  = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void heap_dump() {
    vga_print_color("\n--- Heap Dump ---\n", COLOR_CYAN, COLOR_BLACK);
    struct heap_block* block = heap_start;
    int i = 0;
    while (block) {
        vga_print("  Block ");
        print_int(i++);
        vga_print(": size=");
        print_int(block->size);
        vga_print(" ");
        if (block->is_free)
            vga_print_color("[FREE]\n", COLOR_GREEN, COLOR_BLACK);
        else
            vga_print_color("[USED]\n", COLOR_RED, COLOR_BLACK);
        block = block->next;
    }
    vga_print_color("-----------------\n", COLOR_CYAN, COLOR_BLACK);
}