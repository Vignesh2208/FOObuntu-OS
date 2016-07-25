#include "keyboard.h"

static char * video_mem_ptr = (char *)VIDEO;

pcb* active_terminal_task = NULL;
static int n_active_terminals = 0;
static int curr_active_terminal_number = 0;
static terminal_t * active_terminals[MAX_TERMINALS] = {NULL,NULL,NULL,NULL,NULL,NULL};


// Tab will come after \b. Currently not supported
/* Declarations */
static char base_characters[NUM_OF_CHARAC] =
{
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',0,      /*Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,    /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,          /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

/* When CapsLock or Shift is pressed */
static char alternate_characters[NUM_OF_CHARAC] = {

  0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',0,      /*Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,    /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,          /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */

};

void handle_terminal_switch(int dest_terminal){

  if(dest_terminal < MAX_CONCURRENT_TERMINALS){
      // valid terminal switch
      pcb * next_base_shell = find_task_by_pid(dest_terminal);
      pcb * active_task = next_base_shell;
      if(active_task != NULL){
          while(active_task->child != NULL){
            active_task = active_task->child;
          }
          //next_task = active_task;
          //if(active_task != current){
            next_task = active_task;

            active_terminal_task = next_task;

            //printf("active_task pid : %d\n",active_task->pid);
            //printf("Entered this\n");
          // load new tss into GDT
        /*{
          seg_desc_t the_tss_desc;
          the_tss_desc.granularity    = 0;
          the_tss_desc.opsize         = 0;
          the_tss_desc.reserved       = 0;
          the_tss_desc.avail          = 0;
          the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
          the_tss_desc.present        = 1;
          the_tss_desc.dpl            = 0x0;
          the_tss_desc.sys            = 0;
          the_tss_desc.type           = 0x9;
          the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

          SET_TSS_PARAMS(the_tss_desc, &next_task->secondary_hardware_context, tss_size);
          tss_desc_ptr = the_tss_desc;    
          ltr(KERNEL_TSS);
        }*/
          //printf("next task pid : %d\n",active_task->pid);
          
          show_screen((terminal_t *)next_task->curr_terminal);

        //}
      }
  }



}

void write_new_line(terminal_t *t){

  int i = 0;
  int is_window_changed = 0;

  // Set current character to space. Then redraw cursor in next line.
  t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y,TOTAL_ROWS)][t->cursor_x] = ' ';  
  *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = ' ';
  *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
                      
  if(t->n_lines_buffered < (N_PAGES_HISTORY)*NUM_ROWS)
       t->n_lines_buffered++;

  if(t->cursor_y == NUM_ROWS - 1){
                          
     t->disp_end_row = mod(t->disp_end_row + 1, TOTAL_ROWS);
     t->disp_start_row = mod(t->disp_start_row + 1, TOTAL_ROWS);  
     is_window_changed = 1;                          

  }
  else
     t->cursor_y  = mod((t->cursor_y + 1), NUM_ROWS);
                      
  // Change to start of next line.
  t->cursor_x = 0;

                      // Redraw cursor
  t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y,TOTAL_ROWS)][t->cursor_x] = '_';
  if(is_window_changed){
       for(i = t->cursor_x + 1; i < NUM_COLS; i++)
          t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y,TOTAL_ROWS)][i] = ' ';
        show_screen(t);
  }
  else{
      *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = '_';
      *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
  }

}

void write_character(terminal_t * t,char pressed){

  int i = 0;
  int is_window_changed = 0;

  if(pressed != '\n'){
    t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][t->cursor_x] = pressed;
    *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = pressed;
    *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
    if(t->cursor_x == NUM_COLS - 1){
                        
      if(t->n_lines_buffered < (N_PAGES_HISTORY)*NUM_ROWS)
          t->n_lines_buffered++;                        

      if(t->cursor_y == NUM_ROWS - 1){
          t->disp_end_row = mod((t->disp_end_row + 1),TOTAL_ROWS);
          t->disp_start_row = mod((t->disp_start_row + 1),TOTAL_ROWS);
          is_window_changed = 1;                          
      }
      else
        t->cursor_y  = mod((t->cursor_y + 1), NUM_ROWS); // Cursor moves to next line
      }
      // Increment t->cursor_x
      t->cursor_x = mod((t->cursor_x + 1), NUM_COLS);

      // Redraw cursor at new location
      t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][t->cursor_x] = '_';

      // Redraw display window if scrolled down
      if(is_window_changed){
                        
        for(i = t->cursor_x + 1; i < NUM_COLS; i++)
            t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][i] = ' ';
        show_screen(t);

      }
      else{
         *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = '_';
         *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
      }
    }
    else{

      // character to write is '\n'
      write_new_line(t);

    }
}

uint32_t terminal_read(terminal_t * t, char * buf, int size){
  int i = 0;

  for(i = 0; i < size; i++)
    buf[i] = '\0';

  while(t->is_data_available_for_read == 0);

  //printf("Data available %d\n",t->is_data_available_for_read);
  
  int len = (size-1 > t->n_characters_entered ? t->n_characters_entered : size-1);  
  
  for(i = 0; i < len; i++){
    buf[i] = t->line_buffer[i];
  }


  t->is_data_available_for_read = 0;

  return len;

}

void terminal_write(terminal_t * t, char * buf, int num_bytes_to_write){

  int i = 0;

  // Flush current line buffer
  for(i = 0; i < MAX_HOLD_BUFFER_SIZE; i++)
    t->line_buffer[i] = '\0';
  t->n_characters_entered = 0;

  for(i = 0; i < num_bytes_to_write ; i++)
    write_character(t,buf[i]);



}

/* Utility functions */
int mod(uint32_t a, uint32_t b){


  while(a < 0){
    a = a + b;
  }
  return (a - (a/b)*b);
}

/* Check if the scancode is prinatble character */
int is_printable(uint8_t scancode){

  if(scancode >=NUM_OF_CHARAC)
    return 0;

  if(base_characters[scancode] != 0)
    return 1;
  return 0;
}


/* Terminal Scroll functions. Show the screen starting from the disp start window */
void show_screen(terminal_t * t){
  // if(current != active_terminal_task)
  //    return;

  int i = 0, k = 0, j = 0;
  k = t->disp_start_row;
  for(i = 0 ; i < NUM_ROWS; i++){
    for(j = 0; j < NUM_COLS ; j++){
      *(uint8_t *)(video_mem_ptr + ((i*NUM_COLS + j) << 1)) = t->Terminal_Display_Buffer[k][j];
      *(uint8_t *)(video_mem_ptr + ((i*NUM_COLS + j) << 1) + 1) = ATTRIB;
    }
    k = mod((k + 1),TOTAL_ROWS);

  }
}

/* Clear the screen - fill with spaces. Happens on pressing CTRL-L */
void clear_screen(terminal_t * t){

  t->is_data_available_for_read = 0;
  t->n_characters_entered = 0;

  if(t->n_lines_buffered > (N_PAGES_HISTORY - 1)*NUM_ROWS)
    t->n_lines_buffered = (N_PAGES_HISTORY - 1)*NUM_ROWS;


  t->disp_start_row = mod((t->disp_start_row + t->cursor_y), TOTAL_ROWS);  
  t->cursor_x = 0;
  t->cursor_y = 0;  
  t->disp_end_row = mod((t->disp_start_row + NUM_ROWS), TOTAL_ROWS);

  int i = 0, k = 0, j = 0;
  for(i = 0; i < MAX_HOLD_BUFFER_SIZE; i++)
    t->line_buffer[i] = '\0';


  for(i = 0, k = t->disp_start_row; i < NUM_ROWS; i++, k = mod((k + 1), TOTAL_ROWS)){
    for(j = 0; j < NUM_COLS ; j++){
      *(uint8_t *)(video_mem_ptr + ((i*NUM_COLS + j) << 1)) = ' ';
      t->Terminal_Display_Buffer[k][j] = ' ';
      *(uint8_t *)(video_mem_ptr + ((i*NUM_COLS + j) << 1) + 1) = ATTRIB;
    }
  }
}

/* Set the Terminal Display Buffer to all Spaces */
int terminal_open(terminal_t * t){

  if(n_active_terminals == MAX_TERMINALS)
    return -1;

  int i = 0;
  int j = 0;
  t->special_key_status = 0x0;
  t->cursor_x = 0;
  t->cursor_y = 0;
  t->disp_start_row = 0;
  t->disp_end_row = NUM_ROWS;
  t->n_lines_buffered = 1;
  t->n_ups = 0;
  t->n_characters_entered = 0;
  t->is_data_available_for_read = 0;
  t->enter_pressed = 0;

  for(i = 0; i < MAX_TERMINALS; i++){
    if(active_terminals[i] == NULL){
      t->terminal_no  = i;
      n_active_terminals ++;
      active_terminals[i] = t;
    }
  }

 

  for(i = 0 ; i < TOTAL_ROWS; i++){
    for(j = 0; j < NUM_COLS; j++){
      t->Terminal_Display_Buffer[i][j] = ' ';
    }
  }

  for(i = 0; i < MAX_HOLD_BUFFER_SIZE; i++){
    t->line_buffer[i] = '\0';    
  }

  return 0;
}

void terminal_close(terminal_t * t){

  int i = 0;
  
  if(t->terminal_no < MAX_TERMINALS){
    if(t->terminal_no == curr_active_terminal_number){
      curr_active_terminal_number = -1;
      for(i = 0; i < MAX_TERMINALS; i++){
        if(active_terminals[i] != NULL && i != t->terminal_no)
        {
          curr_active_terminal_number = i;
          break;
        }  
      }

    }
    active_terminals[t->terminal_no] = NULL;

    n_active_terminals --;
  }

}

/* KeyPad Special Keys */

/* For Page down - Scroll Down from history */
void handle_page_down(terminal_t * t){
  
  int change = 0;
  if(t->n_ups > 0){
    if(t->n_ups > MAX_SCROLL_CHANGE){
      t->n_ups -= MAX_SCROLL_CHANGE;
      change = MAX_SCROLL_CHANGE;
    }
    else{
      change = t->n_ups;
      t->n_ups = 0;
    }    
    t->disp_start_row = mod(t->disp_start_row + change, TOTAL_ROWS);
    show_screen(t);
  }
}

/* For Page UP - Scroll upwards through history */
void handle_page_up(terminal_t * t){
  
  
  int n_Available = 0;
  int change = 0;
  if(t->n_lines_buffered > MAX_SCROLL_CHANGE && t->n_ups < (t->n_lines_buffered - (t->cursor_y + 1))){
  
    /* Number of Available lines to Scroll */
    n_Available = (t->n_lines_buffered - (t->cursor_y + 1)) - t->n_ups;
    if(n_Available == 0)
      return;

    if(n_Available < MAX_SCROLL_CHANGE){
      t->n_ups += n_Available;
      change = n_Available;
    }
    else{
      t->n_ups += MAX_SCROLL_CHANGE;
      change = MAX_SCROLL_CHANGE;
    }
    t->disp_start_row = mod(t->disp_start_row - change, TOTAL_ROWS);
    show_screen(t);

  }

}

void handle_end_press(terminal_t * t){
//printf("End Press\n");
}

void handle_home_press(terminal_t * t){
//printf("Home Press\n");
}

void handle_insert_press(terminal_t * t){
//printf("Insert Press\n");
}

void handle_delete(terminal_t * t){
//printf("Delete Press\n") ;
}

/* Arrow Keys */
void handle_up_arrow(terminal_t * t){
//printf("Up Arrow\n");
 handle_page_up(t); 
}

void handle_down_arrow(terminal_t * t){
//printf("Down Arrow\n");
  handle_page_down(t);
}

void handle_left_arrow(terminal_t * t){
//printf("Left Arrow\n");
}

void handle_right_arrow(terminal_t *t){
//printf("Right Arrow\n");
}


/*  Returns the Decoded character - if it is printable, or takes the apropriate actions  by setting bits in the status byte */
/* KeyPad is Handled as well */
char get_decoded_character(terminal_t * t, uint8_t scancode){

  if(is_printable(scancode) == 0){
    

    switch(scancode){
      case LEFT_SHIFT_PRESSED :   t->special_key_status |= (1 << LEFT_SHIFT_STATUS_BIT);                                  
                                  break;

      case RIGHT_SHIFT_PRESSED :  t->special_key_status |= (1 << RIGHT_SHIFT_STATUS_BIT);
                                  break;                                
      
      case LEFT_SHIFT_RELEASED :  t->special_key_status &= ~(1 << LEFT_SHIFT_STATUS_BIT);                                   
                                  break;
      
      case RIGHT_SHIFT_RELEASED : t->special_key_status &= ~(1 << RIGHT_SHIFT_STATUS_BIT);      
                                  break;

      case LEFT_ALT_PRESSED     : t->special_key_status |= (1 << ALT_STATUS_BIT);
                                  break;

      //case RIGHT_ALT_PRESSED     : t->special_key_status |= (1 << ALT_STATUS_BIT);
      //                            break;                                  


      case LEFT_ALT_RELEASED     : t->special_key_status &= ~(1 << ALT_STATUS_BIT);
                                  break;

      //case RIGHT_ALT_RELEASED    : t->special_key_status &= ~(1 << ALT_STATUS_BIT);
      //                            break;                                                                    

      case CAPS_LOCK_PRESSED    : 
                                  if((t->special_key_status >> CAPS_LOCK_STATUS_BIT) & 0x01)
                                    t->special_key_status &= ~(1 << CAPS_LOCK_STATUS_BIT);  
                                  else
                                    t->special_key_status |= (1 << CAPS_LOCK_STATUS_BIT);      
                                  
                                  break;

      case CTRL_PRESSED         : t->special_key_status |= (1 << CTRL_STATUS_BIT);
                                  break;

      case CTRL_RELEASED        : t->special_key_status &= ~(1 << CTRL_STATUS_BIT);
                                  break;
      
      case NUM_LOCK_PRESSED     : if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    t->special_key_status &= ~(1 << NUM_LOCK_STATUS_BIT);  
                                  else
                                     t->special_key_status |= (1 << NUM_LOCK_STATUS_BIT);     
      
                                  break;                                                                    
      

      case KEYPAD_7_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '7';
                                  else{
                                    // Handle Home press here
                                    handle_home_press(t);
                                  }
                                  break;

      case KEYPAD_8_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '8';
                                  else{

                                    // Handle up arrow press here
                                    handle_up_arrow(t);
                                  }
                                  break;

      case KEYPAD_9_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '9';
                                  else{
                                      // Handle Pgup here
                                      handle_page_up(t);

                                  }
                                  break;                                                                                                       
      case KEYPAD_4_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '4';
                                  else{
                                      // Handle left arrow press here
                                      handle_left_arrow(t);
                                  }     
                                  break;                             

      case KEYPAD_5_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '5';
                                  break;               


      case KEYPAD_6_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '6';
                                  else{
                                      // Handle Right arrow press here
                                      handle_right_arrow(t);
                                  }     
                                  break;                                                                                                 

      case KEYPAD_1_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '1';
                                  else{
                                      // Handle End press here
                                      handle_end_press(t);
                                  }     
                                  break;                             
      case KEYPAD_2_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '2';
                                  else{
                                      // Handle Down arrow press here
                                      handle_down_arrow(t);

                                  }     
                                  break;                             
      case KEYPAD_3_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '3';
                                  else{
                                      // Handle PageDown press here
                                      handle_page_down(t);
                                  }     
                                  break;                                                               

      case KEYPAD_0_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '0';
                                  else{
                                      // Handle Ins press here
                                      handle_insert_press(t);
                                  }     
                                  break;                                                               

      case KEYPAD_DOT_PRESSED    :  if((t->special_key_status >> NUM_LOCK_STATUS_BIT) & 0x01)
                                    return '.';
                                  else{
                                      // Handle  Del press here
                                      handle_delete(t);
                                  } 
                                  break;

      case F1_PRESSED             :  if((t->special_key_status >> ALT_STATUS_BIT ) & 0x01){
                                          handle_terminal_switch(0); // switch to terminal 0
                                     }
                                     break;


      case F2_PRESSED             :  if((t->special_key_status >> ALT_STATUS_BIT ) & 0x01){
                                          handle_terminal_switch(1); // switch to terminal 1
                                     }
                                     break;


      case F3_PRESSED             :  if((t->special_key_status >> ALT_STATUS_BIT ) & 0x01){
                                          handle_terminal_switch(2); // switch to terminal 2
                                     }
                                     break;

      default                   : return 0;                                                                   
                                  break;
    } 


    return 0;
  }
  else{

    // Character is printable
    char pressed = base_characters[scancode];

    // If L is pressed - check for clear screen
    if(pressed == 'l'){
      if((t->special_key_status >> CTRL_STATUS_BIT) & 0x01){
          clear_screen(t);
          return 0;
      }
    }

    // If the character is Alphabet - Check for Upper or Lower case
    if(pressed >= 'a' &&  pressed <= 'z'){

        

        if(((t->special_key_status >> CAPS_LOCK_STATUS_BIT) & 0x01) && !((t->special_key_status >> LEFT_SHIFT_STATUS_BIT) & 0x01) && !((t->special_key_status >> RIGHT_SHIFT_STATUS_BIT) & 0x01)){
          return pressed - CASE_CHANGE;    // Caps
        }
        if(!((t->special_key_status >> CAPS_LOCK_STATUS_BIT) & 0x01) && (((t->special_key_status >> LEFT_SHIFT_STATUS_BIT) & 0x01) || ((t->special_key_status >> RIGHT_SHIFT_STATUS_BIT) & 0x01))){
          return pressed - CASE_CHANGE;    // Caps
        }

        // Else non Caps
        return pressed;

    }
    else{
      // Non alphabet - Check for base or alternate characters
      if(((t->special_key_status >> LEFT_SHIFT_STATUS_BIT) & 0x01) || ((t->special_key_status >> RIGHT_SHIFT_STATUS_BIT) & 0x01))
        return alternate_characters[scancode];
      else
        return pressed;

    }



  }


}

/* Handles received Keyboard Interrupts - gets decoded character - if prinatble, prints it to current cursor location
 * Also Handles maintaining History, vertical scrolling, maintaining cursor position as well as backspaces and Enter.
 */
void keyboard_irq_handler()
{
  unsigned char scancode;

  scancode = inb(0x60);
  int is_window_changed = 0;
  int i = 0;
  
 
  if(n_active_terminals == 0) // No active terminals yet.
    return; 

  //if(curr_active_terminal_number == -1 || active_terminals[curr_active_terminal_number] == NULL)       //Something wrong. No terminals open.
  //  return;


  cli();
  //terminal_t * t = active_terminals[curr_active_terminal_number]; // Get pointer to current active terminal and write to it.
  terminal_t * t ;//current->curr_terminal; // Get pointer to current active terminal and write to it.
  if (active_terminal_task!=NULL)
      t=active_terminal_task->curr_terminal;
  else
      t=current->curr_terminal;
  
  disable_paging();
  char pressed = get_decoded_character(t,scancode);
 
  if(pressed > 0){

        // Check if the user had currently scrolled up. If so, reset display start window
    if(t->n_ups > 0){
      t->disp_start_row = mod(t->disp_start_row + t->n_ups,TOTAL_ROWS);
      t->n_ups = 0;
      show_screen(t);
    }

    switch(pressed){

        case '\b' :   // Backspace 

                      if(t->n_characters_entered == 0)
                          return; // do nothing

                      t->line_buffer[t->n_characters_entered - 1] = '\0';
                      t->n_characters_entered--;
                      // Put space at current cursor location                       
                      t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][t->cursor_x] = ' ';
                      *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = ' ';
                      *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
                      
                      if(t->cursor_x == 0){                        

                        if(t->n_lines_buffered > 0)
                          t->n_lines_buffered --;
                        
                        if(t->cursor_y == 0){                          
                          if(t->n_lines_buffered > 0){                      
                            // Scroll up through history
                            t->disp_end_row = mod((TOTAL_ROWS + t->disp_end_row - NUM_ROWS), TOTAL_ROWS);
                            t->disp_start_row = mod((TOTAL_ROWS + t->disp_start_row - NUM_ROWS), TOTAL_ROWS);
                            is_window_changed = 1;                     
                            t->cursor_y = mod((NUM_ROWS + t->cursor_y - 1), NUM_ROWS);
                            t->cursor_x = mod((NUM_COLS + t->cursor_x - 1), NUM_COLS);                                 
                          }
                          else{

                            // Put cursor in the new cursor position
                            t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][t->cursor_x] = '_';
                            *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = '_';
                            *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
                            return;
                          }
                        }
                        else{                          
                          t->cursor_x = mod((NUM_COLS + t->cursor_x - 1), NUM_COLS);
                          t->cursor_y = mod((NUM_ROWS + t->cursor_y - 1), NUM_ROWS);                                                  
                        }
                        
                      }
                      else
                        t->cursor_x = mod((NUM_COLS + t->cursor_x - 1),NUM_COLS);

                      t->Terminal_Display_Buffer[mod(t->disp_start_row + t->cursor_y, TOTAL_ROWS)][t->cursor_x] = '_';
                      if(is_window_changed){
                        show_screen(t);
                        
                      }
                      else{
                        *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1)) = '_';
                        *(uint8_t *)(video_mem_ptr + ((t->cursor_y*NUM_COLS + t->cursor_x) << 1) + 1) = ATTRIB;
                      }
                      break;

        case '\n' :   // Handle Enter separately
                      t->is_data_available_for_read = 1;
                      t->line_buffer[t->n_characters_entered] = '\n';
                      t->n_characters_entered ++;
                      t->enter_pressed = 1;

                      write_new_line(t);
                      break;

        default   :   // Print any other character at current cursor location, then change cursor    
                      if(t->enter_pressed){
                          t->n_characters_entered = 0;
                          for(i = 0; i < MAX_HOLD_BUFFER_SIZE; i++)
                              t->line_buffer[i] = '\0';
                          t->enter_pressed = 0;
                      }

                      if(t->n_characters_entered < MAX_HOLD_BUFFER_SIZE - 1){
                          t->is_data_available_for_read = 0;
                          t->line_buffer[t->n_characters_entered] = pressed;
                          t->n_characters_entered ++;


                      }
                      else{
                          // Don't allow new characters to be entered
                          return;
                      }

                      write_character(t,pressed);
                      break;


    }
    
  }
  
}

