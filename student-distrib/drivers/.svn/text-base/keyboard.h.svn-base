#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../lib.h"
#include "../pcb.h"

#define LEFT_SHIFT_PRESSED 0x2A
#define LEFT_SHIFT_RELEASED 0xAA
#define RIGHT_SHIFT_PRESSED 0x36
#define RIGHT_SHIFT_RELEASED 0xB6
#define CAPS_LOCK_PRESSED 0x3A
#define NUM_LOCK_PRESSED 0x45
#define CTRL_PRESSED  0x1D
#define CTRL_RELEASED 0x9D
#define BACKSPACE_PRESSED 0x0E
#define KEYPAD_7_PRESSED 0x47
#define KEYPAD_8_PRESSED 0x48
#define KEYPAD_9_PRESSED 0x49
#define KEYPAD_MINUS_PRESSED 0x4A
#define KEYPAD_4_PRESSED 0x4B
#define KEYPAD_5_PRESSED 0x4C
#define KEYPAD_6_PRESSED 0x4D
#define KEYPAD_PLUS_PRESSED 0x4E
#define KEYPAD_1_PRESSED 0x4F
#define KEYPAD_2_PRESSED 0x50
#define KEYPAD_3_PRESSED 0x51
#define KEYPAD_0_PRESSED 0x52
#define KEYPAD_DOT_PRESSED 0x53
#define ENTER_PRESSED 0x1C
#define LEFT_ALT_PRESSED 0x38
#define RIGHT_ALT_PRESSED 0x38
#define LEFT_ALT_RELEASED 0xB8
#define RIGHT_ALT_RELEASED 0xB8
#define F1_PRESSED 0x3B
#define F1_RELEASED 0xBB
#define F2_PRESSED 0x3C
#define F2_RELEASED 0xBC
#define F3_PRESSED 0x3D
#define F3_RELEASED 0xBD
#define F4_PRESSED 0x3E
#define F4_RELEASED 0xBE

#define N_PAGES_HISTORY 2
#define TOTAL_ROWS N_PAGES_HISTORY*NUM_ROWS
#define MAX_SCROLL_CHANGE 10
#define NUM_OF_CHARAC 128
#define CASE_CHANGE 0x20

#define LEFT_SHIFT_STATUS_BIT 7
#define RIGHT_SHIFT_STATUS_BIT 6
#define CAPS_LOCK_STATUS_BIT 5
#define NUM_LOCK_STATUS_BIT 4
#define CTRL_STATUS_BIT 3
#define ALT_STATUS_BIT 2
#define MAX_HOLD_BUFFER_SIZE 128
#define MAX_TERMINALS MAX_N_PROCESSES
#define MAX_CONCURRENT_TERMINALS 3

typedef struct terminal_struct{

  uint8_t special_key_status;                      // Stores current status of Shift, CapsLock and NumLock Buttons.
  uint32_t terminal_no;
  char Terminal_Display_Buffer[TOTAL_ROWS][NUM_COLS];  // Stores current stdin data
  uint32_t cursor_x;                               // Current location of cursor_x
  uint32_t cursor_y;                               // Current location of cursor_y
  uint32_t disp_start_row;                         // Current display start row in Terminal_Display_Buffer
  uint32_t disp_end_row;                           // Current display end row in Terminal_Display_Buffer
  int n_lines_buffered;                            // Number of Buffered lines
  int n_ups;                                       // Number of scroll ups that would be allowed from the current state.
  int n_characters_entered;
  int is_data_available_for_read;
  int enter_pressed;
  char line_buffer[MAX_HOLD_BUFFER_SIZE];


}terminal_t;

/* Calculate a % b */
int mod(uint32_t a, uint32_t b);

/* Check if the scancode is prinatble character */
int is_printable(uint8_t scancode);

/* Terminal Scroll functions. Show the screen starting from the disp start window */
void show_screen(terminal_t * t);

/* Clear the screen - fill with spaces. Happens on pressing CTRL-L */
void clear_screen(terminal_t * t);

/* For Page down - Scroll Down from history */
void handle_page_down(terminal_t * t);

/* For Page UP - Scroll upwards through history */
void handle_page_up(terminal_t * t);

void handle_end_press(terminal_t * t);
void handle_home_press(terminal_t * t);
void handle_insert_press(terminal_t * t);
void handle_delete(terminal_t * t);
void handle_up_arrow(terminal_t * t);
void handle_down_arrow(terminal_t * t);
void handle_left_arrow(terminal_t * t);
void handle_right_arrow(terminal_t * t);

/*  Returns the Decoded character - if it is printable, or takes the apropriate actions  by setting bits in the status byte */
/* KeyPad is Handled as well */
char get_decoded_character(terminal_t * t, uint8_t scancode);

/* Handles received Keyboard Interrupts - gets decoded character - if prinatble, prints it to current cursor location
 * Also Handles maintaining History, vertical scrolling, maintaining cursor position as well as backspaces and Enter.
 */
void keyboard_irq_handler();

// Write character at current cursor position
void write_character(terminal_t * t,char pressed);

// write new line character at current cursor position
void write_new_line(terminal_t * t);

/* Set the Terminal Display Buffer to all Spaces */
int terminal_open(terminal_t * t);

// Read the current line buffer
uint32_t terminal_read(terminal_t * t, char * buf, int size);

// Write to the terminal. It Flushes the current line buffer.
void terminal_write(terminal_t * t, char * buf, int num_butes_to_write);

// Removes it from array of active terminals
void terminal_close(terminal_t * t);

#endif


