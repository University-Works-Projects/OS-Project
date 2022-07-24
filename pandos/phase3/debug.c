/*
 * @file klog.c
 * @author Maldus512 
 * @brief Small library that implements a circular log buffer. When properly traced (with ASCII representation),
 *          `klog_buffer` displays a series of printed lines.
 */

#include "../h/debug.h"

// Print str to klog
void klog_print(char *str) {
    while (*str != '\0') {
        // If there is a newline skip to the next one
        if (*str == '\n') {
            next_line();
            str++;
        } 
        // Otherwise just fill the current one
        else {
            klog_buffer[klog_line_index][klog_char_index] = *str++;
            next_char();
        }
    }
}


// Princ a number in hexadecimal format (best for addresses)
void klog_print_hex(unsigned int num) {
    const char digits[] = "0123456789ABCDEF";

    do {
        klog_buffer[klog_line_index][klog_char_index] = digits[num % 16];
        num /= 16;
        next_char();
    } while (num > 0);
}


// Move onto the next character (and into the next line if the current one overflows)
static void next_char(void) {
    if (++klog_char_index >= KLOG_LINE_SIZE) {
        klog_char_index = 0;
        next_line();
    }
}


// Skip to next line
static void next_line(void) {
    klog_line_index = (klog_line_index + 1) % KLOG_LINES;
    klog_char_index = 0;
    // Clean out the rest of the line for aesthetic purposes
    for (unsigned int i = 0; i < KLOG_LINE_SIZE; i++) {
        klog_buffer[klog_line_index][i] = ' ';
    }
}