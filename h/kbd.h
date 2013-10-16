#ifndef __KBD_H__
#define __KBD_H__

#define i8042_control_port 0x64
#define i8042_datasrc_port 0x60
#define i8082_output_buffer_status_bit 	0
#define i8082_input_buffer_status_bit	1
#define i8082_system_flag_bit 			2
#define i8082_command_data_avail_bit	3
#define i8082_keyboard_active_bit 		4
#define i8082_error_bit					5
#define i8082_timeout_error_bit			6
#define i8082_parity_error_bit			7



void keyboard_init(void);
int kbd_data_ready(void);
int get_kbd_data(void);
int process_key(void);

#endif
