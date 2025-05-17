/*
  Main application file (2) for the Treasure hunt Project. More information in the README.md file

  -- PHASE 2 --
  holds the logic for managing processes, and calls functionalities from the other main file

  -- PHASE 3 --
  reroute all the outputs from stdout to the parent process through a pipe, such that printing to the screen is done exclusively from the parent process

  Project done by Mihai Toderasc - 2025
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <stddef.h>
#include <fcntl.h>

#define _SRCFILE_NAME "treasure_manager.c"
#define _BINFILE_NAME "treasure_manager"
#define _COMFILE_NAME "commands.cms"
#define _ALLFILE_PATH "."

#define MAX_CMD_LEN 32
#define MAX_PTH_LEN 64
#define MAX_LIN_LEN 64
#define PIPERD_LEN  64

#define _LIST_H "--listh"
#define _LIST_T "--list"
#define _VIEW   "--view"

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
int pipe_fd[2]; // holds the pipe file descriptors when and if one opens
int read_pipe = 0; // tells the current process wether to read from the pipe

// function that prints the usage
void print_usage(char* cmd) {
  printf("Usage: %s\n", cmd);
  printf("Create separate processes and call on them to manage hunts and auxiliary data.\n");
  printf("Requires the \'treasure_manager.c\' script to be located in the same directory as the calling script.\n");
  // NOTE: maybe add a way to specify the name and path of the binary.
  //       Check if the binary file exists in the specified (or default) path before
  //       launching a background process calling the script
  printf("\n");
  printf("Options:\n");
  printf("\t--op                   Lorem ipsum...\n");
  printf("\t--help           print this help and exit\n");
  printf("\n");
  printf("Exit status:\n");
  printf("\tProcess exits with status code 0 if no errors were encountered during the execution, or with the specific errno value set by the encountered error.\n");
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
    exit(errno);
  }

  return ret;
}

// function that reads from the pipe and displays stdout in the parent process
void pipe_to_stdout() {
  char frompipe[PIPERD_LEN];
  int readsmth = 0;

  while(read_pipe) {
    // clear read buffer so it doesn't contain garbo data
    memset(frompipe, 0, PIPERD_LEN);
    
    // attempt to read from pipe
    int nread = read(pipe_fd[0], frompipe, PIPERD_LEN * sizeof(char));

    // do stuff depending on what status code we got from reading
    switch(nread) {
    case -1:
      // if nothing is in the buffer skip iteration
      if(errno == EAGAIN) {
	if(readsmth) {
	  read_pipe = 0;
	}
	break;
      }
      else {
	perror("Reading from pipe failed");
	exit(errno);
      }
    case 0:
      // if writing end of pipe closed (???)
      // NOTE: the pipe's writing end can close only when all the children die
      //       (i.e. the monitor dies unexpectedly), so i don't know if we need to
      //       do something here, since an error is thrown somewhere else in the code
      //       (i hope anyway)
      printf("Idk how the pipe closed but sure");
      read_pipe = 0;
      break;
    default:
      // if pipe has data, display it to the screen (stdout)
      printf("%s", frompipe);
      fflush(stdout);

      readsmth = 1;
      break;
    }
  }
}

// function that signals the monitor to list treasures
void list_treasures(char* arg1) {
  //printf("treasure list\n"); // -- debug
  // write data about the command to be run to the comms file
  // i use fopen() because it's more convenient when working with char[]
  char comfile_path[MAX_PTH_LEN];
  snprintf(comfile_path, MAX_PTH_LEN, "%s/%s", _ALLFILE_PATH, _COMFILE_NAME);

  // open file and truncate it to 0 size, write data, and close file
  FILE* comm_file = fopen(comfile_path, "w");
  fprintf(comm_file, "%s\n", _LIST_T);
  fprintf(comm_file, "%s\n", arg1);

  fclose(comm_file);

  // notify child process it can read from file
  if(kill(monitor_pid, SIGUSR1) < 0) {
    perror("Failed to notify child to read");
    exit(errno);
  }

  // wait for response from child
  pause();

  // read from the pipe
  read_pipe = 1;
  pipe_to_stdout();
}

// function that signals the monitor to list hunts
void list_hunts() {
  //printf("hunt list\n"); // -- debug
  // write data about the command to be run to the comms file
  // i use fopen() because it's more convenient when working with char[]
  char comfile_path[MAX_PTH_LEN];
  snprintf(comfile_path, MAX_PTH_LEN, "%s/%s", _ALLFILE_PATH, _COMFILE_NAME);

  // open file and truncate it to 0 size, write option, and close file
  FILE* comm_file = fopen(comfile_path, "w");
  fprintf(comm_file, "%s\n", _LIST_H);

  fclose(comm_file);

  // notify child process it can read from file
  if(kill(monitor_pid, SIGUSR1) < 0) {
    perror("Failed to notify child to read");
    exit(errno);
  }

  // wait for response from child
  pause();

  // read from the pipe
  read_pipe = 1;
  pipe_to_stdout();
}

// function that signals the monitor to view a certain treasure
void view_treasure(char* arg1, char* arg2) {
  //printf("viewed booty\n");
  // write data about the command to be run to the comms file
  // i use fopen() because it's more convenient when working with char[]
  char comfile_path[MAX_PTH_LEN];
  snprintf(comfile_path, MAX_PTH_LEN, "%s/%s", _ALLFILE_PATH, _COMFILE_NAME);

  // open file and truncate it to 0 size, write data, and close file
  FILE* comm_file = fopen(comfile_path, "w");
  fprintf(comm_file, "%s\n", _VIEW);
  fprintf(comm_file, "%s\n", arg1);
  fprintf(comm_file, "%s\n", arg2);

  fclose(comm_file);

  // notify child process it can read from file
  if(kill(monitor_pid, SIGUSR1) < 0) {
    perror("Failed to notify child to read");
    exit(errno);
  }

  // wait for response from child
  pause();

  // read from the pipe
  read_pipe = 1;
  pipe_to_stdout();
}

// function that signals the monitor to stop
void stop_monitor() {
  printf("monitor stopped\n");
}

// function that exits the program if the monitor is not running, else throws an error
void exit_program() {
  //printf("program exited\n"); // -- debug
  // close the reading end of the pipe
  close(pipe_fd[0]);
}

// function that prints command usage
void print_usage_cmd() {
  printf("\e[32m[HLP]\e[0m This is a help prompt for the Treasure Hub menu\n");
  printf("\e[33m[HLP]\e[0m Manage and view hunts and their respective treasures with this interactive menu! Feel the thrill of a true hunter in the wild and explore managerial opportunities of your own making!\n");
  printf("(Requires the \'treasure_manager.c\' file from Phase 1)\n");
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
/* deprecated
void hchld_err(int signum) {
  printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly\n");
  monitor_pid = -1;
}
*/

// handler for child kill
void hchld_kill(int signum) {
  // wait for child and grab status code
  int wstatus;
  if(waitpid(monitor_pid, &wstatus, 0) < 0) {
    perror("Error waiting for child");
    exit(errno);
  }

  // check if child finished with exit() or return in main()
  if(WIFEXITED(wstatus)) {
    // check if child finished with exit code 0 (success)
    int westatus = WEXITSTATUS(wstatus);
    
    if(westatus == 0) {
      printf("\e[32m[LOG]\e[0m Success. Child killed (Code %d)\n", westatus);
    }
    else {
      printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly with code %d\n", westatus);
    }
  }
  else {
    printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly and badly\n");
  }
  
  monitor_pid = -1;
}

// handler in the child for peaceful termination
void hchld_fin(int signum) {
  // NOTE: Project requirement
  usleep(3000000); // wait 3 seconds
  
  // resolve stuff in the child before kill
  //printf("hchld_fin\n");
  exit(0);
}

// handler in the child for answering to a prompt from a command
// this is the main logic loop in the child, verification done asynchronously
void hchld_usr1(int signum) {
  // open file and read from it
  char comfile_path[MAX_PTH_LEN];
  snprintf(comfile_path, MAX_PTH_LEN, "%s/%s", _ALLFILE_PATH, _COMFILE_NAME);
  
  FILE* comm_file = fopen(comfile_path, "r");
  char curr_arg[MAX_LIN_LEN + 1];
  char* args[5]; // 0 name, 1 op, 2 hunt, 3 trj id, 4 (or 2, 3) NULL

  char binfile_path[MAX_PTH_LEN];
  snprintf(binfile_path, MAX_PTH_LEN, "%s/%s", _ALLFILE_PATH, _BINFILE_NAME);

  for(int i = 0; i < 5; i++) {
    args[i] = NULL;
  }
  
  // set the first argument (i.e. the program name)
  if((args[0] = malloc(sizeof(char) * MAX_PTH_LEN)) == NULL) {
    perror("Error making space for arguments in child");
    exit(errno);
  }

  strcpy(args[0], binfile_path);
  //printf("%s\n", args[0]); // -- debug
  
  // get rest of arguments
  for(int it = 1; it <= 3; it++) {
    curr_arg[0] = '\0';
    // get argument from file
    if(!fgets(curr_arg, MAX_LIN_LEN, comm_file)) {
      break;
    }
    //printf(":: %s\n", curr_arg); // -- debug
    
    // format argument and flush rest of line
    if(curr_arg[strlen(curr_arg) - 1] == '\n') {
      curr_arg[strlen(curr_arg) - 1] = '\0';
    }
    else {
      while(fgetc(comm_file) != '\n') {}
    }

    // alocate a space for a new argument
    if((args[it] = malloc(sizeof(char) * (MAX_LIN_LEN + 1))) == NULL) {
      perror("Error making space for arguments in child");
      exit(errno);
    }

    strcpy(args[it], curr_arg);
  }

  // -- debug -- vv --
  //for(int i = 0; i < 5; i++) {
  //  printf(": %s\n", args[i]);
  //}
  // -- debug -- ^^ --  

  // start a new child process for the manager
  pid_t manager_pid = get_child_process();

  // in the manager child, execute the manager with the arguments
  if(manager_pid == 0) {
    execv(binfile_path, args);
    perror("Execv failed");
    exit(errno);
  }

  // wait for child to exit
  int wstatus = 0;
  waitpid(manager_pid, &wstatus, 0);

  // check if child finished with exit() or return in main()
  if(WIFEXITED(wstatus)) {
    // check if child finished with exit code 0 (success)
    int westatus = WEXITSTATUS(wstatus);
    
    if(westatus == 0) {
      printf("\e[34m[LOG]\e[0m Success. Child finished (Code %d)\n", westatus);
    }
    else {
      printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly with code %d\n", westatus);
    }
  }
  else {
    printf("\e[31m[ERR]\e[0m Error. Child process ended unexpectedly and badly\n");
  }
  

  // notify parent we are done with this operation
  if(kill(getppid(), SIGUSR1) < 0) {
    perror("Failed to notify parent operation is done");
    exit(errno);
  }
}

// handler for when child finished an operation
void hchld_done(int signum) {
  // read from the pipe and display the result to stdout
  /*
     TO DO: since we do not know how many bytes are written to the pipe,
            either communicate that information at the start of the broadcast,
	    or find a way to read from the pipe until all bytes are read
   */   
  
  printf("\e[32m[LOG]\e[0m Done.\n");

  // tells the parent process to stop reading from the pipe
  //read_pipe = 0;
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

  // check if the 'treasure_manager.c' file exists, and compile it
  // if not, let the user know program cannot run
  struct stat st = { 0 };
  if(stat(_SRCFILE_NAME, &st) < 0) {
    perror("Necessary prerequisite file non existent");
    exit(errno);
  }

  // compile the necessary program so we have the binary for it
  // NOTE: i am taking the liberty to use system() here, since we dive deeper
  //       into child processes later
  int compile_cmd_size = strlen("gcc ") + strlen(_SRCFILE_NAME) + strlen(" -o ") + strlen(_BINFILE_NAME) + 1;
  char compile_cmd[compile_cmd_size];

  snprintf(compile_cmd, compile_cmd_size, "gcc %s -o %s", _SRCFILE_NAME, _BINFILE_NAME);
  
  if(system(compile_cmd) != 0) {
    perror("Error compiling prerequisite file");
    exit(errno);
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
  struct sigaction* sa = NULL; // sigaction container variable
  
  // prints menu flavour text
  print_menu();
  
  while(is_menu_running) {
    // initialise some things
    curr_command_code = UNKWN;

    // 31 - red, 32 - green, 34 - blue
    // `\e[xxm` -> xx = number above
    printf("\e[32mtreaure_hub\e[0m>> ");
    fflush(stdout);

    // take command input from user
    char cmd[MAX_CMD_LEN + 1];
    if(!fgets(cmd, MAX_CMD_LEN + 1, stdin)) {
      perror("Error reading input");
      exit(errno);
    }

    // get rid of the rest of the buffer
    if(cmd[strlen(cmd) - 1] != '\n') {
      while(fgetc(stdin) != '\n') {}
    }
    else {
      cmd[strlen(cmd) - 1] = '\0';
    }

    // take only the first token of the command, separated by white spaces
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

      // open a pipe for commnunication between processes
      if(pipe(pipe_fd) < 0) {
	perror("Error opening pipe");
	exit(errno);
      }

      // if not, open a new one
      printf("\e[34m[LOG]\e[0m Starting new monitor...\n");
      
      monitor_pid = get_child_process();

      // parent process code snippet
      if(monitor_pid > 0) {
	// close the writing end of the pipe
	close(pipe_fd[1]);

	// change the behaviour of the reading end of the pipe to be nonblocking
	fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);
	
	// define sigaction
	sa = calloc(1, sizeof(struct sigaction));
	if(sa == NULL) {
	  perror("Error alocating sigaction");
	  exit(-1);
	}
	// define handler for child encountering error and terminating (ignores stop)
	sa->sa_handler = hchld_kill;
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

	// define sigaction
	sa = calloc(1, sizeof(struct sigaction));
	if(sa == NULL) {
	  perror("Error alocating sigaction");
	  exit(-1);
	}
	// define handler for child finishing operation
	sa->sa_handler = hchld_done;

	sigaction(SIGUSR1, sa, NULL);

	free(sa);
      }

      // in child, stop the menu loop
      if(monitor_pid == 0) {
	is_menu_running = 0;
	break;
      }
      
      //if(monitor_pid)
      //printf("ID of parent %d, ID of child %d\n", getpid(), monitor_pid); // -- debug
      break;
    case LIST_T:
      // check if any monitor is open
      if(monitor_pid < 0) {
	printf("\e[35m[HUB]\e[0m No monitor is open at this time. Maybe open one?\n");
	break;
      }

      // read additional data for the option (huntID)
      char read_line[MAX_LIN_LEN];
      printf("\e[35m[HUB]\e[0m Hunt id: ");
      fflush(stdout);

      fgets(read_line, MAX_LIN_LEN, stdin);

      // format and flush the rest of stdin
      if(read_line[strlen(read_line) - 1] == '\n') {
	read_line[strlen(read_line) - 1] = '\0';
      }
      else {
	while(fgetc(stdin) != '\n') {}
      }

      // take only the first token (separator is white space)
      char* hunt_id = strtok(read_line, " ");
      //printf(": %s:\n", aux); // -- debug
      
      list_treasures(hunt_id);
      break;
    case LIST_H:
      // check if any monitor is open
      if(monitor_pid < 0) {
	printf("\e[35m[HUB]\e[0m No monitor is open at this time. Maybe open one?\n");
	break;
      }

      list_hunts();
      break;
    case VIEW:
      // check if any monitor is open
      if(monitor_pid < 0) {
	printf("\e[35m[HUB]\e[0m No monitor is open at this time. Maybe open one?\n");
	break;
      }

      // read additional data for the option (huntID)
      //char read_line[MAX_LIN_LEN];
      char read_line2[MAX_LIN_LEN];
      printf("\e[35m[HUB]\e[0m Hunt id: ");
      fflush(stdout);

      fgets(read_line2, MAX_LIN_LEN, stdin);

      // format and flush the rest of stdin
      if(read_line2[strlen(read_line2) - 1] == '\n') {
	read_line2[strlen(read_line2) - 1] = '\0';
      }
      else {
	while(fgetc(stdin) != '\n') {}
      }

      // take only the first token (separator is white space)
      char* hunt_id2 = strtok(read_line2, " ");

      char read_line3[MAX_LIN_LEN];
      printf("\e[35m[HUB]\e[0m Treasure id: ");
      fflush(stdout);

      fgets(read_line3, MAX_LIN_LEN, stdin);

      // format and flush the rest of stdin
      if(read_line3[strlen(read_line3) - 1] == '\n') {
	read_line3[strlen(read_line3) - 1] = '\0';
      }
      else {
	while(fgetc(stdin) != '\n') {}
      }

      // take only the first token (separator is white space)
      char* trj_id2 = strtok(read_line3, " ");
      
      view_treasure(hunt_id2, trj_id2);
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
      if(kill(monitor_pid, SIGTERM) < 0) {
	perror("Failed to signal termination to child");
	exit(errno);
      }
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
    // close the reading end of the pipe
    close(pipe_fd[0]);

    // redirect stdout to the writing end of the pipe
    if(dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
      perror("Error redirecting standard output to pipe");
      exit(errno);
    }

    // close writing end of pipe as it is not necessary anymore
    close(pipe_fd[1]);

    // declare handler for termination
    struct sigaction sa_chld_term = { 0 };
    
    sa_chld_term.sa_handler = hchld_fin;
    sigaction(SIGTERM, &sa_chld_term, NULL);

    // declare handler for option execution
    struct sigaction sa_chld_usr1 = { 0 };

    sa_chld_usr1.sa_handler = hchld_usr1;
    sigaction(SIGUSR1, &sa_chld_usr1, NULL);

    // notify parent that child opened successfully
    if(kill(getppid(), SIGUSR2) < 0) {
      perror("Failed to notify the parent process");
      exit(errno);
    }

    /* NOTE : Since in the parent we ignore the SIGCHLD signals sent by the
              stopping or continuing of the child process, we are going to use
	      the SIGUSR1 signal for communication for the commands both to and
	      from the child process.

	      We send SIGUSR1 to the child process to notify it to read data
	      and execute the necessary commands.

	      After the commands were executed, we send SIGUSR1 to the parent
	      in order to let it know that the execution finished, and have
	      it resume its execution.
     */
    
    int is_child_running = 1;

    while(is_child_running) {
      usleep(3000000); // wait 3 sec.
      //is_child_running = 0; // -- debug
    }
  }
  
  return 0;
}
