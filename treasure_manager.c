/*
  Main application file for the Treasure hunt Project. More information in the README.md file

  Project done by Mihai Toderasc - 2025
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// codes for options
enum opcode_t {
  UNKWN = -1,
  ADD_H = 0,
  ADD_T,
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

// function that prints the usage of the program
// TO DO: add the actual --help text
void print_usage(char* cmd) {
  printf("Usage: %s [OPTION]... [ARGS]...\n", cmd);
  printf("Lorem ipusm...\n");
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
    {"add_hunt", ADD_H},
    {"add", ADD_T},
    {"list", LIST},
    {"view", VIEW},
    {"remove_treasure", RM_T},
    {"remove_hunt", RM_H}
  };
  const static int OPTION_CNT = sizeof(OPTIONS)/sizeof(struct option_t);
  //printf("%d\n", option_cnt); // --debug
  
  enum opcode_t curr_option_code = UNKWN;

  for(int i = ADD_H; i < OPTION_CNT; i++) {
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

  printf("OK %d.\n", curr_option_code); // --debug

  // use the obtained option code to do stuff
  
  return 0;
}


