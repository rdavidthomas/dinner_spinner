/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: david2
 *
 * Created on December 17, 2016, 2:40 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dtgc_console.h"
#include "spinner.h"
#include "file.h"

/////////////  Macros /////////////

#define CLR_SCREEN() printf("\033[2J")
#define MOVE_CURSOR(x,y) printf( "\033[%d;%dH", y, x )



//////////// Local structure //////////////

struct appState {
    int exit;
    spin_t spin_handle;
};


////////////// LOCAL FUNCTION PROTOTYPES ///////////////
/**
 * Print the welcome message
 */
static inline void pheader();

/**
 * Prints out a message one char at a time. 
 */
static inline void slow_print(char* text, size_t length, int rate_uS);

static int init_console();

///////////  CONSOLE COMMAND HANDLER PROTOTYPES ////////
static int 
  console_command_place_holder(void* statemem, dtgc_console_parsed_t param);

static int 
  appExit(struct appState* statemem);






////////////////   MAIN  ////////////////
/*
 * 
 */
int main(int argc, char** argv) {
    
    struct appState state = {
        .exit = 0,
        .spin_handle = NULL
    };
    
    pheader();
    
    state.spin_handle = spin_new();
    if( state.spin_handle == NULL) {
        printf("Failed creating spinner\n");
        goto errExit;
    }
    
    //TOD: load menu file
    char* fakemenu[] = {
        "cake",
        "Potato Soup",
        "Chicken and Rice",
        "Chicken and Potato",
        "Hamburger" };
    if( spin_set_selection_list(state.spin_handle, fakemenu, 5) < 0) {
        printf("Failed setting selection spin list\n");
        goto errExit;
    }
    
    init_console();
    
    display_menu(NULL);
    
    while( !state.exit ) {
        console_tasks(&state);
    }
    return (EXIT_SUCCESS);
    
    errExit:
        if(state.spin_handle) spin_destroy(&state.spin_handle);
        return (EXIT_SUCCESS);
}



////////////// LOCAL FUNCTION PROTOTYPES ///////////////


static inline void slow_print(char* text, size_t length, int rate_uS) {
    int txtidx = 0;
    for(txtidx = 0; txtidx < length; txtidx++ ) {
        printf("%c", text[txtidx]);
        usleep(rate_uS);
        fflush(stdout);
    }   
}

static inline void pheader() {
    CLR_SCREEN();
    MOVE_CURSOR(33, 11 );
    fflush(stdout);
    slow_print("Dinner Spinner", 14, 100000);
    usleep(1000000);
    MOVE_CURSOR(1, 13 );
}

static int 
console_command_place_holder(void* statemem, dtgc_console_parsed_t param) {
    printf("\nSomeday I will be implemented and do something... but not yet.\n");
}

static int 
cmd_spin(void* statemem, dtgc_console_parsed_t param) {
    struct appState* s = (struct appState*)statemem;
    char* selection = spin_get_rand_selection(s->spin_handle);
    printf("\n\n Make %s for Dinner\n\n", selection);
    return 0;
}


static int 
cmd_add(void* statemem, dtgc_console_parsed_t param) {
   struct meal_s m;
   if( param.argc != 2) return -1;
   
   strncpy(m.name, param.argv[1],64);
   if( append_meal(&m, "./meals.dat") < 0) {
       printf("Failed to append file\n");
   }
   return 0;
}


static int appExit(struct appState* statemem) {
    statemem->exit = 1;
    return 0;
}


static int init_console() {
    //load menu commands
    int retval = 0;
    retval |= register_command("add", "add <name> - add a meal", cmd_add);
    retval |= register_command("delete", "delete <name> - remove a meal", console_command_place_holder);
    retval |= register_command("load", "load <file name> - load meals from a file", console_command_place_holder);
    retval |= register_command("spin", "spin - select a meal for dinner", cmd_spin);
    retval |= register_command_noparam("help", "help - print this menu", display_menu);
    retval |= register_command_noparam("?", "\0", display_menu);
    retval |= register_command_noparam("exit", "exit - exit this program", (console_cmd_noparam_fptr_t)appExit);
    retval |= register_command_noparam("q", "\0", (console_cmd_noparam_fptr_t)appExit);
    return retval;
}
