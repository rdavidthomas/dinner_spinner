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

/////////////  Macros /////////////

#define CLR_SCREEN() printf("\033[2J")
#define MOVE_CURSOR(x,y) printf( "\033[%d;%dH", y, x )



//////////// Local structure //////////////

struct appState {
    int exit;
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
        .exit = 0
    };
    
    pheader();
    
    //TOD: load menu file
    
    init_console();
    
    display_menu(NULL);
    
    while( !state.exit ) {
        console_tasks(&state);
    }
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

static int appExit(struct appState* statemem) {
    statemem->exit = 1;
    return 0;
}


static int init_console() {
    //load menu commands
    int retval = 0;
    retval |= register_command("add", "add <name> - add a meal", console_command_place_holder);
    retval |= register_command("delete", "delete <name> - remove a meal", console_command_place_holder);
    retval |= register_command("load", "load <file name> - load meals from a file", console_command_place_holder);
    retval |= register_command("spin", "spin - select a meal for dinner", console_command_place_holder);
    retval |= register_command_noparam("help", "help - print this menu", display_menu);
    retval |= register_command_noparam("?", "\0", display_menu);
    retval |= register_command_noparam("exit", "exit - exit this program", (console_cmd_noparam_fptr_t)appExit);
    retval |= register_command_noparam("q", "\0", (console_cmd_noparam_fptr_t)appExit);
    return retval;
}
