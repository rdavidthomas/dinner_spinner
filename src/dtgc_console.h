/* 
 * File:   dtgc_console.h
 * Author: rdavidthomas
 *
 * Created on October 15, 2015, 9:42 AM
 */

#ifndef DTGC_CONSOLE_H
#define	DTGC_CONSOLE_H

#undef BEGIN_C_DECLS
#undef END_C_DECLS
#ifdef __cplusplus
# define BEGIN_C_DECLS extern "C" {
# define END_C_DECLS }
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
#endif

BEGIN_C_DECLS

#define DTGC_MENU_CHARACTER_WIDTH 60
#define DTGC_MENU_MAX_SIZE 4096
#define DTGC_MAX_CMDS 64
#define DTGC_MAX_PARAM 7
#define DTGC_CONSOLE_TIMEOUT_uS 100 // 1000mS
#define DTGC_CONSOLE_AUTO_MENU_REPRINT 0

typedef struct
{
    int argc;
    char const *argv[DTGC_MAX_PARAM];
} dtgc_console_parsed_t;


typedef enum
{
    dtgc_console_noparam,
    dtgc_console_param,
    dtgc_console_voidall,
} dtgc_console_paramtype_t;

typedef struct 
{
    const char *command;
    const char *description;
    dtgc_console_paramtype_t type;
    union
    {
        int (*cmd_func)(void*, dtgc_console_parsed_t);
        int (*cmd_func_np)(void*);
        void(*cmd_func_vd)();
    };
} dtgc_console_cmds_t;

typedef struct
{
    uint8_t  auto_print_menu:1;
    uint32_t timeout_uS;
    uint8_t *screen_refresh_flag;
    void (*header_func)(void);
} dtgc_console_settings_t;
        
typedef int (*console_cmd_fptr_t)(void*, dtgc_console_parsed_t);
typedef int (*console_cmd_noparam_fptr_t)(void*);
typedef void (*console_cmd_voidall_fptr_t)(void);


/**
 *  Add a command to the system.  
 * @note commands with a description of '\0' will be hidden and not displayed in the menu.
 * @param ascii_cmd = accii command entered to activate function
 * @param description = description of command displayed in menu, '\0' will hide command.
 * @param cmd_func = pointer to function.
 * @return 
 */
int register_command( char const* ascii_cmd, char const* description, int (*cmd_func)(void*, dtgc_console_parsed_t));
int register_command_noparam( char const* ascii_cmd, char const* description, int (*cmd_func)(void*));
int register_command_voidall( char const* ascii_cmd, char const* description, void (*cmd_func)());



int display_menu( void* ptr );

int process_menu_commands( void *ptr, char *buf);

int console_tasks(void *ptr);

int dtgc_console_set_settings(dtgc_console_settings_t);
int dtgc_console_set_timeout( uint32_t timeout);
int dtgc_console_print_settings(void *ptr);
int dtgc_console_str_to_integer64(char* const i_string, uint64_t *r_num64);
int dtgc_console_str_to_integer32(char* const i_string, uint32_t *r_numc32);
int dtgc_console_hex_to_dec64(char* const i_string, uint64_t *r_dec64);
int dtgc_console_hex_to_dec32(char* const i_string, uint32_t *r_dec32);


END_C_DECLS

#endif	/* DTGC_CONSOLE_H */

