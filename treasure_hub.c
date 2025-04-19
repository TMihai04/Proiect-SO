/*
  Main application file (2) for the Treasure hunt Project. More information in the README.md file

  -- PHASE 2 --
  holds the logic for managing processes, and calls functionalities from the other main file

  Project done by Mihai Toderasc - 2025
 */

#include <stdio.h>
#include <string.h>

#define DEFAULT_BIN_NAME "treasure_manager"
#define DEFAULT_BIN_PATH "."

// codes for menu commands
enum cmdcode_t {
  UNKWN = -1,
  START = 0,
  LIST_H,
  LIST_T,
  VIEW,
  STOP,
  EXIT,
  HELP,
};

// structure with (command literal, command code) pair
struct command_t {
  char* cmd;
  enum cmdcode_t code;
};

// function that prints the usage
void print_usage(char* cmd) {
  printf("Usage: %s\n", cmd);
  printf("Create separate processes and call on them to manage hunts and auxiliary data.\n");
  printf("Requires the \'treasure_manager.c\' script to be compiled under the \'treasure_manager\' name. Both binaries must be located in the same directory\n");
  // NOTE: maybe add a way to specify the name and path of the binary.
  //       Check if the binary file exists in the specified (or default) path before
  //       launching a background process calling the script
  printf("\n");
  printf("Options:\n");
  printf("\t--op                   Lorem ipsum...\n");
  printf("\t--help           print this help and exit\n");
  printf("\n");
  printf("Exit status:\n");
  printf(" 0  if OK,\n");
}

// function that prints the menu header when starting the program
void print_menu() {
  printf("menu text\n");
}

// function that starts a background monitor process
void start_monitor() {
  printf("monitor start\n");
}

// function that signals the monitor to list treasures
void list_treasures() {
  printf("treasure list\n");
}

// function that signals the monitor to list hunts
void list_hunts() {
  printf("hunt list\n");
}

// function that signals the monitor to view a certain treasure
void view_treasure() {
  printf("viewed booty\n");
}

// function that signals the monitor to stop
void stop_monitor() {
  printf("monitor stopped\n");
}

// function that exits the program if the monitor is not running, else throws an error
void exit_program() {
  printf("program exited\n");
}

// function that prints command usage
void print_usage_cmd() {
  printf("cmd usage text\n");
}

int main(int argc, char** argv) {
  // check if any options were given
  // NOTE: for the time being we will only accept the --help option
  if(argc > 1) {
    char curr_option[strlen(argv[1])];

    if(argv[1][0] == '-' && argv[1][1] == '-') {
      strcpy(curr_option, argv[1] + 2);
    }
    else {
      printf("Unknown option. Use --help for details\n");
      return 0;
    }

    // NOTE: change code below if more options are added in the future
    if(!strcmp(curr_option, "help")) {
      print_usage(argv[0]);
    }
    else {
      printf("Unknown option. Use --help for details\n");
    }

    return 0;
  }

  // code for when no options were given
  const static struct command_t COMMANDS[] = {
    {"start_monitor", START},
    {"list_hunts", LIST_H},
    {"list_treasures", LIST_T},
    {"view_treasure", VIEW},
    {"stop_monitor", STOP},
    {"exit", EXIT},
    {"help", HELP}
  };
  const static int COMMAND_CNT = sizeof(COMMANDS) / sizeof(struct command_t);

  enum cmdcode_t curr_command_code = UNKWN;
  int program_run = 1;

  print_menu();
  
  while(program_run) {
    curr_command_code = UNKWN;
    printf(">> ");
    // TO DO: read command input with fgets(),
    //        retain the first token and discard the rest (' ' as separator),
    //        cut the token if its too long,
    //        discard the rest of the input (with fgetc() or similar),
    char cmd[16];
    scanf("%16s", cmd);
    
    for(int i = 0; i < COMMAND_CNT; i++) {
      if(!strcmp(cmd, COMMANDS[i].cmd)) {
	curr_command_code = COMMANDS[i].code;
	break;
      }
    }

    // if no command in the list was given skip iteration
    if(curr_command_code == UNKWN) {
      printf("Unknown command. Use \'help\' for command list\n");
      continue;
    }

    // check for command code and respond accordingly
    switch(curr_command_code) {
    case START:
      start_monitor();
      break;
    case LIST_T:
      list_treasures();
      break;
    case LIST_H:
      list_hunts();
      break;
    case VIEW:
      view_treasure();
      break;
    case STOP:
      stop_monitor();
      break;
    case EXIT:
      exit_program();
      program_run = 0;
      break;
    case HELP:
      print_usage_cmd();
      break;
    default:
      printf("Somehow you managed to land outside of the designated command space. I am now dissapointed in my ability to write switch statements. Oh well...\n");
      break;
    }
   }
  
  return 0;
}
