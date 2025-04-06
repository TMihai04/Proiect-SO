/*
  Main application file for the Treasure hunt Project. More information in the README.md file

  Project done by Mihai Toderasc - 2025
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_PATH_LEN 128 // path for hunt dir + files ! file names included in length !
#define MAX_LOG_LEN 128 // maximum length of log entry

#define LOGFILE_NAME "logged_hunt.log" // name of the log file
#define TRJFILE_NAME "treasures.dat" // name of the treasures file

//#define MAX_TRJID_LEN 32 // maximum length of treasure id
#define MAX_TRJDESC_LEN 64 // maximum length of treasure description
#define MAX_USRID_LEN 32 // maximum length of user id

/*
  maximum values on bits (unsigned):
  8-bit : 255
  16-bit : 65,535
  32-bit : 4,294,967,295
 */

// codes for options
enum opcode_t {
  UNKWN = -1,
  ADD,
  LIST,
  VIEW,
  RM_H,
  RM_T
};

// structure with (option literal, option code) pair
struct option_t {
  char* op;
  enum opcode_t code;
};

// rename types that will be used for clarity
// ---> renames here <--- // ...maybe

// structure with (treasure id, starting byte) pair
// sitauated at the start of the treasures file for optimisation purposes
struct header_t {
  //char trj_id[MAX_TRJID_LEN];
  uint8 trj_id;
  uint16 st_byte;
};

// structure containing the treasure details
// makes for every actual entry in the treasures file
struct trj_data_t {
  //char trj_id[MAX_TRJID_LEN];
  uint8 trj_id;
  char usr_id[MAX_USRID_LEN];
  float lati, longi;
  char desc[MAX_TRJDESC_LEN];
  int val;
};

// function that prints the usage of the program
// TO DO: add the actual --help text
void print_usage(char* cmd) {
  printf("Usage: %s [OPTION]... [ARGS]...\n", cmd);
  printf("Lorem ipusm...\n");
}

// function that adds a treasure to a certain hunt who's id is given as parameter
void add_treasure(char* hunt_id) {
  struct stat sb = { 0 }; // stat buffer
  char dir_path[MAX_PATH_LEN] = "./"; // local path prefix
  char log_path[MAX_PATH_LEN];
  char trj_path[MAX_PATH_LEN];

  int log_fd, trj_fd;
  char wr_log[MAX_LOG_LEN];
  wr_log[0] = '\0';

  strcat(dir_path, hunt_id); // whole path for directory (relative)

  strcpy(log_path, dir_path);
  strcat(log_path, "/");
  strcat(log_path, LOGFILE_NAME); // whole path for log file

  strcpy(trj_path, dir_path);
  strcat(trj_path, "/");
  strcat(trj_path, TRJFILE_NAME); // whole path for treasure file
  
  // if directory doesn't exist, create it and initilaise it with respective files
  if(!(stat(dir_path, &sb) && S_ISDIR(sb.st_mode))) {
    printf("The hunt \'%s\' is not currently happening. Making it happen...\t", hunt_id);
    mkdir(dir_path, 0777); // make a dir with all permisions (?)
    printf("Done.\n");

    printf("Creating hunt logs...\t");
    // create the log file and give r/w permissions
    if((log_fd = open(log_path, O_EXCL | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
      printf("Error creating log file.\n");
      exit(1);
    }
    printf("Done.\n");

    // log the directory creation
    wr_log[0] = '\0';
    strcat(wr_log, "Hunt \'");
    strcat(wr_log, hunt_id);
    strcat(wr_log, "\' created.\n");
    write(log_fd, wr_log, strlen(wr_log) + 1);

    // log the log file creation
    wr_log[0] = '\0';
    strcat(wr_log, "Log file created.\n");
    write(log_fd, wr_log, strlen(wr_log) + 1);

    printf("Creating space for treasures to be stored...\t");
    // create the treasures file and give r/w permissions
    if((trj_fd = open(trj_path, O_EXCL | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
      printf("Error creating treasure file.\n");
      exit(2);
    }
    close(trj_fd);
    printf("Done.\n");

    // log treasure file creation
    wr_log[0] = '\0';
    strcat(wr_log, "Treasures file created.\n");
    write(log_fd, wr_log, strlen(wr_log) + 1);
    
    close(log_fd);
    printf("Logged initial operation.\n");
  }

  
}

int main(int argc, char** argv) {
  // check if number of arguments is right
  if(argc == 1) {
    print_usage(argv[0]);
    exit(0);
  }

  // if arguments exist, we check if it is the right format
  char curr_option[strlen(argv[1])];
  curr_option[0] = '\0';
  
  if(argv[1][0] == '-' && argv[1][1] == '-') {
    strcpy(curr_option, argv[1] + 2);
    //printf("%s\n", curr_option); // --debug
  }
  else {
    printf("Unknown option.\n");
    print_usage(argv[0]);
    exit(0);
  }

  // if the option is in the right format then we check which one it is
  const static struct option_t OPTIONS[] = {
    {"add", ADD},
    {"list", LIST},
    {"view", VIEW},
    {"remove_treasure", RM_T},
    {"remove_hunt", RM_H}
  };
  const static int OPTION_CNT = sizeof(OPTIONS)/sizeof(struct option_t);
  //printf("%d\n", option_cnt); // --debug
  
  enum opcode_t curr_option_code = UNKWN;

  for(int i = ADD; i < OPTION_CNT; i++) {
    if(!strcmp(curr_option, OPTIONS[i].op)) {
      curr_option_code = i;
      break;
    }
  }

  if(curr_option_code == UNKWN) {
    printf("Unknown option.\n");
    print_usage(argv[0]);
    exit(0);
  }

  //printf("OK %d.\n", curr_option_code); // --debug

  // use the obtained option code to do stuff
  switch(curr_option_code) {
  case ADD:
    if(argc != 3) {
      if(argc > 3)
	printf("Did not yet implement adding multiple treasures at a time. Use one argument\n");
      else
	printf("Option takes argument <hunt_id>\n");
      
      print_usage(argv[0]);
      exit(0);
    }
    
    add_treasure(argv[2]); // adds a treasure to given hunt by id
    
    break;
  default:
    printf("Somehow you managed to get past the option check and landed outside the defined range of options. I am truly impressed\n");
    print_usage(argv[0]);
    break;
  }
  
  return 0;
}


