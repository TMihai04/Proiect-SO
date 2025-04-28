/*
  Main application file (2) for the Treasure hunt Project. More information in the README.md file

  -- PHASE 2 --
  holds the logic for managing processes, and calls functionalities from the other main file

  Project done by Mihai Toderasc - 2025
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define DEFAULT_BIN_NAME "treasure_manager"
#define DEFAULT_BIN_PATH "."

#define MAX_CMD_LEN 32

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

// global variables
pid_t monitor_pid = -1; // holds the pid for the child process

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
  /*
  printf(" ____ ____ ____   __   ___ __  __ ____ ____    _   _ __  __ ____ \n");
  printf("(_  _(  _ ( ___) /__\\ / __(  )(  (  _ ( ___)  ( )_( (  )(  (  _ \\\n");
  printf("  )(  )   /)__) /(__)\\\\__ \\)(__)( )   /)__)    ) _ ( )(__)( ) _ <\n");
  printf(" (__)(_)\\_(____(__)(__(___(______(_)\\_(____)  (_) (_(______(____/\n");
  */

  printf("(O)----- Welcome to... -----------------------------------------------------------(O)\n");
  printf(" |  .---. .----. .----.  .--.   .----..-. .-..----. .----.   .-. .-..-. .-..----.  | \n");
  printf(" | {_   _}| {}  }| {_   / {} \\ { {__  | { } || {}  }| {_     | {_} || { } || {}  } | \n");
  printf(" |   | |  | .-. \\| {__ /  /\\  \\.-._} }| {_} || .-. \\| {__    | { } || {_} || {}  } | \n");
  printf(" |   `-'  `-' `-'`----'`-'  `-'`----' `-----'`-' `-'`----'   `-' `-'`-----'`----'  | \n");
  printf("(O)------------------------------------------------------- by Mihai Toderasc -----(O)\n");
  printf("\n");
  printf("\e[35m[HUB]\e[0m Insert commands into the prompt bellow and become the hunt manager of your dreams! Type \'help\' for a list of commands and additional details.\n");
  printf("\n");
}

// function that starts a background monitor process
pid_t get_child_process() {
  pid_t ret = fork();

  if(ret == -1) {
    perror("Failed to procreate");
    exit(1);
  }

  return ret;
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
  printf("\e[32m[HLP]\e[0m This is a help prompt for the Treasure Hub menu\n");
  printf("\e[33m[HLP]\e[0m Manage and view hunts and their respective treasures with this interactive menu! Feel the thrill of a true hunter in the wild and explore managerial opportunities of your own making!\n");
  printf("(Requires the binary \'treasure_manager\' file)\n");
  printf("\n");
  printf("\e[33m[HLP]\e[0m Available commands:\n");
  printf("\n");
  printf("[CMD]\tstart_monitor\n");
  printf("[CMD]\tstop_monitor\n");
  printf("[CMD]\tlist_hunts\n");
  printf("[CMD]\tlist_treasures\n");
  printf("[CMD]\tview_treasure\n");
  printf("[CMD]\texit\n");
  printf("[CMD]\thelp\n");
}

// bellow are defined signal handlers

// handler that does nothing
void hdo_nothing(int signum) {
  /* Empty on purpose */
}

// handler for child init
void hchld_open(int signum) {
  printf("\e[32m[LOG]\e[0m Success. Child pid %d\n", monitor_pid);
}

// handler for child error kill
void hchld_err(int signum) {
  printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly\n");
  monitor_pid = -1;
}

// handler for proper child kill
void hchld_kill(int signum) {
  printf("\e[32m[LOG]\e[0m Success. Child killed\n");
  monitor_pid = -1;
}

// handler in the child for peaceful termination
void hchld_fin(int signum) {
  // NOTE: Project requirement
  usleep(3000000); // wait 3 seconds
  
  // resolve stuff in the child before kill
  printf("hchld_fin\n");
  exit(0);
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

  enum cmdcode_t curr_command_code = UNKWN; // command given in menu
  int is_menu_running = 1; // decides if the menu loop should run
  //pid_t parent_pid = getpid(); // holds the pid for the parent process
  struct sigaction* sa = NULL;

  print_menu();
  
  while(is_menu_running) {
    // initialise some things
    curr_command_code = UNKWN;

    // 31 - red, 32 - green, 34 - blue
    // `\e[xxm` -> xx = number above
    printf("\e[32mtreaure_hub\e[0m>> ");

    // take command input from user
    char cmd[MAX_CMD_LEN + 1];
    if(!fgets(cmd, MAX_CMD_LEN + 1, stdin)) {
      perror("Error reading input");
      exit(-1);
    }

    // get rid of the rest of the buffer
    if(cmd[strlen(cmd) - 1] != '\n') {
      while(fgetc(stdin) != '\n') {}
    }
    else {
      cmd[strlen(cmd) - 1] = '\0';
    }

    // take only the first token of the command, separated by ' '
    strtok(cmd, " ");
    
    //printf("%s\n", cmd); // -- debug

    // check if command is in command list
    for(int i = 0; i < COMMAND_CNT; i++) {
      if(!strcmp(cmd, COMMANDS[i].cmd)) {
	curr_command_code = COMMANDS[i].code;
	break;
      }
    }

    // if no command in the list was given skip iteration
    if(curr_command_code == UNKWN) {
      printf("\e[32m[HLP]\e[0m Unknown command. Use \'help\' for command list\n");
      continue;
    }

    // check for command code and respond accordingly
    switch(curr_command_code) {
    case START:
      // BUG: when printing something from the start_monitor() func, it prints twice
      //      from the parent process (only when buffer is not flushed - ends in '\n')
      
      // check if a monitor is already open
      if(monitor_pid > 0) {
	printf("\e[35m[HUB]\e[0m A monitor is already active (id %d)\n", monitor_pid);
	break;
      }

      // if not, open a new one
      printf("\e[34m[LOG]\e[0m Starting new monitor...\n");
      
      monitor_pid = get_child_process();

      // TO DO: make the child process send a signal to the parent when it opened
      //        successfully, then resume and let the user know

      if(monitor_pid > 0) {
	// define sigaction
	sa = calloc(1, sizeof(struct sigaction));
	if(sa == NULL) {
	  perror("Error alocating sigaction");
	  exit(-1);
	}
	// define handler for child encountering error and terminating (ignores stop)
	sa->sa_handler = hchld_err;
	sa->sa_flags = SA_NOCLDSTOP;
	
	sigaction(SIGCHLD, sa, NULL);

	free(sa);
	// define sigaction
	sa = calloc(1, sizeof(struct sigaction));
	if(sa == NULL) {
	  perror("Error alocating sigaction");
	  exit(-1);
	}
	// define handler for child opening
	sa->sa_handler = hchld_open;
	
	sigaction(SIGUSR2, sa, NULL);

	free(sa);
	// wait for child to open
	pause();
      }

      // in child, stop the menu loop
      if(monitor_pid == 0) {
	is_menu_running = 0;
      }
      
      //if(monitor_pid)
      //printf("ID of parent %d, ID of child %d\n", getpid(), monitor_pid); // -- debug
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
      // check if any monitor is open
      if(monitor_pid < 0) {
	printf("\e[35m[HUB]\e[0m No monitor is open at this time. Maybe open one?\n");
	break;
      }

      printf("\e[34m[LOG]\e[0m Ending child...\n");

      // define sigaction
      sa = calloc(1, sizeof(struct sigaction));
      if(sa == NULL) {
	perror("Error alocating sigaction");
	exit(-1);
      }

      // define handler for child closing
      sa->sa_handler = hchld_kill;

      sigaction(SIGCHLD, sa, NULL);
      
      free(sa);

      // stop child
      kill(monitor_pid, SIGTERM);
      //pause();

      while(monitor_pid > 0) {
	if(fgetc(stdin) == '\n') {
	  printf("\e[35m[HUB]\e[0m Input is not validated while monitor is closing\n");
	}
      }
      
      break;
    case EXIT:
      // check if there is any child process running
      if(monitor_pid > 0) {
	printf("\e[35m[HUB]\e[0m Cannot exit while child process is running. Deal with that first\n");
	break;
      }
      
      // exit parent process
      exit_program();
      is_menu_running = 0;
      break;
    case HELP:
      print_usage_cmd();
      break;
    default:
      printf("Somehow you managed to land outside of the designated command space. I am now dissapointed in my ability to write switch statements. Oh well...\n");
      break;
    }
  }

  // code for the child (monitor) process
  if(monitor_pid == 0) {
    // notify parent that child opened successfully
    if(kill(getppid(), SIGUSR2) < 0) {
      perror("Failed to notify the parent process");
      exit(-2);
    }

    // declare handler for termination
    struct sigaction sa_chld = { 0 };
    
    sa_chld.sa_handler = hchld_fin;
    sigaction(SIGTERM, &sa_chld, NULL);
    
    int is_child_running = 1;

    while(is_child_running) {
      usleep(3000000); // wait 3 sec.
      //is_child_running = 0; // -- debug
    }
  }
  
  return 0;
}
