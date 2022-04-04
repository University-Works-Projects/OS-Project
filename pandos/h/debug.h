/*
 * @file klog.c
 * @author Maldus512 
 * @brief Small library that implements a circular log buffer. When properly traced (with ASCII representation),
 *          `klog_buffer` displays a series of printed lines.
 */

#define KLOG_LINES     64     // Number of lines in the buffer. Adjustable, only limited by available memory
#define KLOG_LINE_SIZE 42     // Length of a single line in characters


static void next_line(void);
static void next_char(void);


unsigned int klog_line_index                         = 0;       // Index of the next line to fill
unsigned int klog_char_index                         = 0;       // Index of the current character in the line
char         klog_buffer[KLOG_LINES][KLOG_LINE_SIZE] = {0};     // Actual buffer, to be traced in uMPS3

void klog_print(char *str);

void klog_print_hex(unsigned int num);

// Move onto the next character (and into the next line if the current one overflows)
static void next_char(void);

// Skip to next line
static void next_line(void);