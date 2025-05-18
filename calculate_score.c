/*
  Main application file (3) for the Treasure hunt Project. More information in the README.md file

  -- PHASE 3 --
  calculates the score per hunt for each user
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_PATH_LEN 128

#define MAX_USRID_LEN 32
#define MAX_TRJDESC_LEN 64
#define MAX_TRJ_CNT 256

#define LOGFILE_NAME "logged_hunt.log"
#define TRJFILE_NAME "treasures.dat"

// structure containing the treasure details
// makes for every actual entry in the treasures file
struct trj_data_t {
  //char trj_id[MAX_TRJID_LEN];
  uint8_t id;
  char usr_id[MAX_USRID_LEN]; // 32
  float lati, longi;
  char desc[MAX_TRJDESC_LEN]; // 64
  int val;
};

// structure with (usr id, score) pair
struct dentry_t {
  char usr_id[MAX_USRID_LEN];
  int score;
};

// global variables
pid_t calc_pid = -1; // holds the pid for the current reading process
int pipe_fd[2]; // holds the pipe file descriptors when and if one opens
struct dentry_t tracker[MAX_TRJ_CNT]; // array that acts as a dictionary for user scores
int tracker_cnt = 0; // keeps track of the number of entries in the dictionary

// function that starts a background calculator process
pid_t get_child_process() {
  pid_t ret = fork();

  if(ret == -1) {
    perror("Failed to procreate");
    exit(errno);
  }

  return ret;
}

// handler in the parent that signals a child finished the execution thread
void hchld_done(int signum) {
  // wait for child and grab status code
  int wstatus;
  if(waitpid(calc_pid, &wstatus, 0) < 0) {
    perror("Error waiting for child");
    exit(errno);
  }

  // check if child finished with exit() or return in main()
  if(WIFEXITED(wstatus)) {
    // check if child finished with exit code 0 (success)
    int westatus = WEXITSTATUS(wstatus);
    
    if(westatus == 0) {
      //printf("Success. Child finished execution (Code %d)\n", westatus);
    }
    else {
      printf("Error. Child process ended unexpectedly with code %d\n", westatus);
    }
  }
  else {
    printf("Error. Child process ended unexpectedly and badly\n");
  }
  
  calc_pid = -1;
}

int main(int argc, char** argv) {
  // NOTE: this script is not intended to take any arguments, but the argc and argv
  //       variables were left in case argument handling is required

  // open the pipe for communication between processes
  //if(pipe(pipe_fd) < 0) {
  //  perror("Error opening pipe");
  //  exit(errno);
  //}

  // define sigaction
  struct sigaction* sa = calloc(1, sizeof(struct sigaction));
  if(sa == NULL) {
    perror("Error alocating sigaction");
    exit(errno);
  }

  // define handler for child termination
  sa->sa_handler = hchld_done;

  sigaction(SIGCHLD, sa, NULL);

  free(sa);
  
  // open the current directory
  DIR* this_dir = NULL;
  struct dirent* curr_entry = NULL;
  int hunt_cnt = 0;

  if((this_dir = opendir(".")) == NULL) {
    perror("Error accessing current directory.");
    exit(errno);
  }

  while((curr_entry = readdir(this_dir))) {
    // check if the read entry is a directory
    if(curr_entry->d_type != DT_DIR) {
      continue;
    }

    // skip over current (.) and preceding directories (..)
    if(!strcmp(curr_entry->d_name, ".") || !strcmp(curr_entry->d_name, "..")) {
      continue;
    }

    //printf("%s\n", curr_entry->d_name); // -- debug

    // open the entry and look for the two specific hunt files
    // logged_hunt.log <-- LOGFILE_NAME
    // treasures.dat   <-- TRJFILE_NAME
    //printf("Read dir \'%s\'\n", curr_entry->d_name); // -- debug
    DIR* curr_hunt = opendir(curr_entry->d_name);
    
    // if opening the entry fails, inform the user
    if(!curr_hunt) {
      perror(curr_entry->d_name);
      continue;
    }

    // check hunt integrity
    int has_log = 0, has_trj = 0;
    struct dirent* ent = NULL;
      
    while((ent = readdir(curr_hunt))) {
      // skip over current (.) and preceding directories (..)
      if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
	continue;
      }
      	
      if(ent->d_type == DT_REG) {
	if(!strcmp(ent->d_name, LOGFILE_NAME)) {
	  has_log++;
	}
	else if(!strcmp(ent->d_name, TRJFILE_NAME)) {
	  has_trj++;
	}
      }
    }

    //printf("l:%d t:%d\n", has_log, has_trj); // -- debug

    // notify the user about hunt integrity
    if(has_trj || has_log) {
      hunt_cnt++;
      
      printf("Read hunt \'%s\'", curr_entry->d_name);
      fflush(stdout);

      if(!has_log) {
	printf("(Logs missing)\n");
      }
      else if(!has_trj){
	printf("(Treasures missing)\n");
      }
      else {
	printf("\n");
      }

      // start child process for this hunt and calculate scores
      if(has_trj) {
	calc_pid = get_child_process();

	if(calc_pid == 0) {
	  // child process code

	  // open the specific treasure file
	  char trj_path[MAX_PATH_LEN];
	  
	  strcpy(trj_path, curr_entry->d_name);
	  strcat(trj_path, "/");
	  strcat(trj_path, TRJFILE_NAME);

	  int trj_fd = open(trj_path, O_RDONLY);

	  if(trj_fd < 0) {
	    perror("Error opening treasure file");
	    exit(errno);
	  }

	  struct trj_data_t curr_trj = { 0 };

	  // process each entry
	  while(read(trj_fd, &curr_trj, sizeof(struct trj_data_t))) {
	    int found = 0;
	    
	    for(int i = 0; i < tracker_cnt; i++) {
	      if(!strcmp(tracker[i].usr_id, curr_trj.usr_id)) {
		tracker[i].score += curr_trj.val;
		found = 1;
		break;
	      }
	    }

	    if(!found) {
	      strcpy(tracker[tracker_cnt].usr_id, curr_trj.usr_id);
	      tracker[tracker_cnt].score = curr_trj.val;
	      tracker_cnt++;
	    }
	  }

	  // display results
	  for(int i = 0; i < tracker_cnt; i++) {
	    printf("\t:: %s --> %d\n", tracker[i].usr_id, tracker[i].score);
	  }

	  // close treasure file
	  close(trj_fd);

	  // close directories
	  closedir(curr_hunt);
	  closedir(this_dir);

	  exit(0);
	}
	else {
	  // parent waits for the child to finish
	  pause();
	}
      }
    }

    closedir(curr_hunt);
  }

  closedir(this_dir);

  if(!hunt_cnt) {
    printf("It seems that you cannot take something from where it isn't...\n");
  }
  
  return 0;
}
