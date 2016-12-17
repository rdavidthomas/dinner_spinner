#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/select.h>


#include "dtgc_console.h"

#if USE_LOGGER
	#include "dtgc_log.h"
	#define PERR(x,...) dtgc_PERR(dtgc_ERROR, x, ##__VA_ARGS)
	#define PLOG(x,...) dtgc_PERR(dtgc_INFO, x, ##__VA_ARGS)
#else
	#define PERR(x,...) printf( "[ERROR] " x, ##__VA_ARGS__)
	#define PLOG(x,...) printf( "[LOG]   " x, ##__VA_ARGS__)
#endif

static dtgc_console_cmds_t commands[DTGC_MAX_CMDS];
static int command_count;
static dtgc_console_settings_t settings = { \
    .auto_print_menu = DTGC_CONSOLE_AUTO_MENU_REPRINT, \
    .timeout_uS = DTGC_CONSOLE_TIMEOUT_uS, \
    .screen_refresh_flag = 0, \
    .header_func = NULL \
    };
// ={
// { "menu", "Display the menu", display_menu }, 
// { "ts", "Test serial functions", test_serial },
// { "?", "Display the menu", display_menu },
// { "task", "Display the tasks withing the Task Manager.", RFMC_display_tasks },
// { "quit", "Exit this program", RFMC_application_exit }
// };

#define CONSOLE_DEBUG 1
#define PDBG(x, ...) if(CONSOLE_DEBUG) printf("[DEBUG] " x, ##__VA_ARGS__);

int register_command( char const* ascii_cmd, char const* description, int (*cmd_func)(void*, dtgc_console_parsed_t))
{
	if(cmd_func == NULL) return -1;
	commands[command_count].command = ascii_cmd;
	commands[command_count].description = description;
        commands[command_count].type = dtgc_console_param;
	commands[command_count].cmd_func = cmd_func;	
	command_count++;
	return 0;	
}

int register_command_noparam( char const* ascii_cmd, char const* description, int (*cmd_func)(void*))
{
    if(cmd_func == NULL) return -1;
    commands[command_count].command = ascii_cmd;
    commands[command_count].description = description;
    commands[command_count].type = dtgc_console_noparam;
    commands[command_count].cmd_func_np = cmd_func;	
    command_count++;
    return 0;	
}


int register_command_voidall( char const* ascii_cmd, char const* description, void (*cmd_func)())
{
    if(cmd_func == NULL) return -1;
    commands[command_count].command = ascii_cmd;
    commands[command_count].description = description;
    commands[command_count].type = dtgc_console_voidall;
    commands[command_count].cmd_func_vd = cmd_func;	
    command_count++;
    return 0;	
}

static char menu[DTGC_MENU_MAX_SIZE];

static unsigned char menu_built = 0;

static int build_menu( )
{
    int number_of_elements = command_count;//sizeof(commands) / sizeof(dtgc_console_cmds_t);
    int length_of_cmd = 0;
    int length_of_desc = 0;
    int description_start_pos = 0;
    int max_cmd_length = 0;
    int line_length = 0;
    int menu_size = 0;
    int i = 0;
    char buf[128];
    memset((menu + menu_size), '\n', 1);
    menu_size += 1;
    memset((menu + menu_size), '*', DTGC_MENU_CHARACTER_WIDTH);
    menu_size += DTGC_MENU_CHARACTER_WIDTH;
    menu[menu_size] = '\n';
    menu_size++;
    memset((menu + menu_size), ' ', (DTGC_MENU_CHARACTER_WIDTH/2 - 8));
    menu_size += (DTGC_MENU_CHARACTER_WIDTH/2 - 8);
    sprintf((menu + menu_size), "     Main Menu\n");
    menu_size += 15;
    memset((menu + menu_size), '*', DTGC_MENU_CHARACTER_WIDTH);
    menu_size += DTGC_MENU_CHARACTER_WIDTH;
    menu[menu_size] = '\n';
    menu_size++; 
    menu[menu_size] = '\n';
    menu_size++; 
           
    // search for the longest command.
    for( ; i < number_of_elements; i++)
    {
        length_of_cmd = strlen(commands[i].command);
        if( length_of_cmd >  max_cmd_length) max_cmd_length = length_of_cmd;
    }
    
    description_start_pos = max_cmd_length + 5;     
    
   // printf( "%d menu elements\r\n", number_of_elements);
    for(i = 0 ; i < number_of_elements; i++)
    {
        if(commands[i].description[0] == '\0') continue;
        length_of_cmd = strlen(commands[i].command);
        length_of_desc = strlen(commands[i].description);
        line_length = length_of_desc + description_start_pos;
       // printf("%d - command length %d, descrption length %d, line length %d\r\n", i, length_of_cmd, length_of_desc, line_length  );
        if( line_length > 127 ) line_length = 127;
        memcpy(buf, commands[i].command, length_of_cmd);
        memset((buf+length_of_cmd), '.', (description_start_pos - length_of_cmd));
        memcpy( (buf+description_start_pos), commands[i].description, (line_length - description_start_pos) );
        buf[line_length]  = '\0'; //ensure ends in null char.
        if( (menu_size + line_length + 2) >= DTGC_MENU_MAX_SIZE)
        {
            PERR("Console Menu Exceeds Max size, menu will be truncated");
            break;
        }
        //printf("%s\r\n",buf);
        sprintf((menu + menu_size), "%s\r\n", buf);
        menu_size += (line_length + 2);
    }
      
    menu_built = 1;
    return 0;
}

static void prompt()
{
    printf(":>");
}

int display_menu( void* ptr )
{
    if( menu_built == 0 ) build_menu();
    if(settings.header_func != NULL) settings.header_func();
    printf("%s\r\n", menu);
    //prompt();
}


static dtgc_console_parsed_t parse( char* buffer)
{
    dtgc_console_parsed_t p;
    char* sptr;
    p.argc = 0;
    p.argv[p.argc] = strtok_r(buffer, " \r\n", &sptr);   
    if( p.argv[p.argc] == NULL ) return p;
    for(p.argc = 1; p.argc < DTGC_MAX_PARAM; p.argc++)
    {
        p.argv[p.argc] = strtok_r(NULL, " \r\n", &sptr);
        if(p.argv[p.argc] == NULL) break;        
    }
    return p;
}


int process_menu_commands( void *ptr, char *buffer)
{
    int32_t number_of_cmds = command_count;//sizeof(commands) / sizeof(dtgc_console_cmds_t);
    int32_t i = 0;
    dtgc_console_parsed_t p = parse( buffer );
    if(p.argc == 0) return 0;
    if(p.argv[0] == NULL) return 0;  // blank or no command(User just hit enter)
    
    for(i =0 ; i < number_of_cmds; i++)
    {
        if(strncmp(p.argv[0], commands[i].command, strlen(p.argv[0])) ==  0)
        {
            if(settings.screen_refresh_flag != NULL) *settings.screen_refresh_flag = 1;
            if( p.argc == 1) PLOG( "Console Command: %s ", p.argv[0]);
            else if( p.argc == 2) PLOG( "Console Command: %s %s ", p.argv[0], p.argv[1]);
            else if( p.argc == 3) PLOG( "Console Command: %s %s %s", p.argv[0], p.argv[1], p.argv[2]);
            else if( p.argc == 4) PLOG( "Console Command: %s %s %s %s", p.argv[0], p.argv[1], p.argv[2], p.argv[3]);
            else if( p.argc == 5) PLOG( "Console Command: %s %s %s %s %s", p.argv[0], p.argv[1], p.argv[2], p.argv[3], p.argv[4]);
            else if( p.argc == 6) PLOG( "Console Command: %s %s %s %s %s %s", p.argv[0], p.argv[1], p.argv[2], p.argv[3], p.argv[4], p.argv[5]);
            if( commands[i].type == dtgc_console_param) return commands[i].cmd_func(ptr, p);
            else if( commands[i].type == dtgc_console_noparam) return commands[i].cmd_func_np(ptr);
            else if( commands[i].type == dtgc_console_voidall) {commands[i].cmd_func_vd(); return 0;}
            else {PERR("unknown cmd entry type, can't process\r\n"); return -1;}
        }
    }
    printf("Unknown Command\r\n");
    return -1;
}


int console_tasks(void *ptr)
{
    static unsigned char display_prompt = 1;
    int retval = 0;	 
    fd_set fds;
    struct timeval tv;
    char buf[256];
    memset( buf, 0, 256 );
    tv.tv_sec = settings.timeout_uS / 100000;
    tv.tv_usec = settings.timeout_uS % 100000;
    FD_ZERO(&fds); 
    FD_SET(STDIN_FILENO, &fds); 

    if( display_prompt)
    {
        if(settings.auto_print_menu == 1) 
        {
            display_menu(ptr);
            if(settings.screen_refresh_flag != NULL) *settings.screen_refresh_flag = 1;
        }
        printf("\r\n:>");
        fflush(stdout);
        display_prompt = 0;
    }

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)  return 0;
    if(select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) < 0) return -1;
    if (FD_ISSET(STDIN_FILENO, &fds))
    {
        retval = read( STDIN_FILENO, buf, 256);
        if(retval <= 0) return 0;
       
        if(process_menu_commands(ptr, buf) < 0)
        {
            PERR(" Command reported error\r\n");
        }
 
        display_prompt = 1;
        memset( buf, 0, 256);
    }
    return 0;
}

int dtgc_console_set_settings(dtgc_console_settings_t set)
{
    if (memcpy( &settings, &set, sizeof(dtgc_console_settings_t)) != NULL)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int dtgc_console_set_timeout( uint32_t timeout)
{
    if(timeout == 0)  return -1;
    settings.timeout_uS = timeout;
    return 0;
}

int dtgc_console_set_auto_menu_print( uint8_t print)
{
    settings.auto_print_menu = (0x01 & print);
    return 0;
}

int dtgc_console_print_settings(void *ptr)
{
    printf("***  Console Settings  ***\r\n");
    printf("Menu Auto-Reprint: %0d\r\n", settings.auto_print_menu);
    printf("Timeout uS: %0d\r\n", settings.timeout_uS);
    return 0;
}

/**
 *  Internal function to convert from ACSII to Decimal.
 * Assumes bound checking has already been performed so only valid chars
 * will be provided.
 * @param c - valid hex character input
 * @return hex decimal value represented by character.
 */
static uint8_t _hex_char_to_dec64(char c)
{
    static const char lut[] = 
    {
        ['0'] = 0,1,2,3,4,5,6,7,8,9,
        ['A'] = 10,11,12,13,14,15,
        ['a'] = 10,11,12,13,14,15
    };
    return lut[c];
}

static int _is_valid_hex_chars( char* const i_string )
{
    uint64_t len = strlen(i_string);
    char* rd = i_string;
    for(; rd < i_string+len; rd++)
    {
        if(!(  (( *rd >= '0') && (*rd <= '9' ))
            || (( *rd >= 'A') && (*rd <= 'F' ))
            || (( *rd >= 'a') && (*rd <= 'f' )) 
           ))
        {
            return 0;
        }
    }
    return 1;
}

static int _contains_char(char* const i_string, char c, size_t len)
{
    // start at left since will be used most to find decimals "." or "0x"
    char* rd = (i_string + len) - 1;
    for(; rd >= i_string; rd--)
    {
        if( *rd == c ) return 1;
    }
    return 0;
}

static int _find_char(char** i_string, char c, size_t len)
{
    // start at left since will be used most to find decimals "." or "0x"
    char* rd = (*i_string + len) - 1;
    for(; rd >= *i_string; rd--)
    {
        if( *rd == c )
        {
            *i_string = rd;
            return 1;
        }
    }
    return 0;
}

int dtgc_console_hex_to_dec64(char* const i_string, uint64_t *r_dec64)
{
    uint64_t slen = strlen(i_string);
    char* rdptr = i_string;
    int base_count = 1;
    
    // find the start of hex, 
    if( !_find_char(&rdptr, 'x', slen) )
    {
        if( !_find_char(&rdptr, 'X', slen) ) return -1;
    }
    // move the char after the 'x' or 'X'
    rdptr++;
    
    // ensure valid char only.
    if(!_is_valid_hex_chars(rdptr)) return -1;
    
    
    
    for(; rdptr < i_string + slen; rdptr++)
    {
        *r_dec64 = *r_dec64 + (base_count*_hex_char_to_dec64(*rdptr));
        base_count *= 16;
    }
    return 0;
}

int dtgc_console_hex_to_dec32(char* const i_string, uint32_t *r_dec32)
{
    uint64_t val = 0;
    if( dtgc_console_hex_to_dec64(i_string, &val) < 0) return-1;
    // ensure input is less then 32 bits.
    if( val > 0xFFFFFFFF ) return -1;
    
    *r_dec32 = (uint32_t)val;
    return 0;
}


int dtgc_console_str_to_integer64(char* const i_string, uint64_t *r_num64)
{
    size_t len = strlen(i_string);
    if( (_contains_char(i_string, 'x', len)) 
            || (_contains_char(i_string, 'X', len)) )
    {
        return dtgc_console_hex_to_dec64(i_string, r_num64);
    }
    else if( (_contains_char(i_string, '-', len))
            || (_contains_char(i_string, '+', len))  )
    {
        *r_num64 = strtoll(i_string, NULL, 10);
    }
    else if (_contains_char(i_string, '.', len))
    {
        return -1;
    }
    else
    {
        *r_num64 = strtoull(i_string, NULL, 10);
    }
}

int dtgc_console_str_to_integer32(char* const i_string, uint32_t *r_numc32)
{
    uint64_t val = 0;
    if( dtgc_console_str_to_integer64(i_string, &val) < 0) return-1;
    // ensure input is less then 32 bits.
    if( val > 0xFFFFFFFF ) return -1;
    
    *r_numc32 = (uint32_t)val;
    return 0;
}

