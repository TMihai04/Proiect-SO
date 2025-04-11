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
#include <time.h>

#define MAX_PATH_LEN 128 // path for hunt dir + files ! file names included in length !
#define MAX_LOG_LEN 128 // maximum length of log entry

#define LOGFILE_NAME "logged_hunt.log" // name of the log file
#define TRJFILE_NAME "treasures.dat" // name of the treasures file

//#define MAX_TRJID_LEN 32 // maximum length of treasure id
#define MAX_TRJDESC_LEN 64 // maximum length of treasure description
#define MAX_USRID_LEN 32 // maximum length of user id
#define MAX_TRJ_CNT 256 // maximum number of treasure entries

#define MAX_BUFFER_READ 128 // maximum number of bytes that can be read at once at stdin

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
  RM_T,
  RM_H,
};

// structure with (option literal, option code) pair
struct option_t {
  char* op;
  enum opcode_t code;
};

// rename types that will be used for clarity
// ---> renames here <--- // ...maybe

// structure with (treasure id, starting byte) pair
// situated at the start of the treasures file for optimisation purposes
/*
struct header_t {
  //char trj_id[MAX_TRJID_LEN];
  uint8_t entries_cnt;
  uint8_t trj_ids[MAX_TRJ_CNT];
  uint16_t st_bytes[MAX_TRJ_CNT];
};
 */
// i decided i will go for simplicity

// structure containing the treasure details
// makes for every actual entry in the treasures file
struct trj_data_t {
  //char trj_id[MAX_TRJID_LEN];
  uint8_t id;
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
  
  // if directory doesn't exist, create it and initialise it with respective files
  if(stat(dir_path, &sb) && !S_ISDIR(sb.st_mode)) {
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
    sprintf(wr_log, "Hunt \'%s\' was created.\n", hunt_id);
    if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
      printf("Error updateing logs.\n");
      exit(27);
    }

    // log the log file creation
    sprintf(wr_log, "Log file created.\n");
    if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
      printf("Error updating logs.\n");
      exit(28);
    }

    // create the symbolic link to the log file
    char symlink_name[MAX_PATH_LEN];
    sprintf(symlink_name, "logged_hunt-<%s>", hunt_id);
    if(symlink(log_path, symlink_name) == -1) {
      printf("Error creating symbolic link to logs.\n");
      exit(26);
    }

    // log the symlink file creation
    sprintf(wr_log, "Symlink to log file created.\n");
    if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
      printf("Error updating logs.\n");
      exit(29);
    }

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
    write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char));
    
    close(log_fd);
    printf("Logged initial operation.\n");
  }

  // now that we are sure the required directory exists, we can add the entry

  // open treasures file for reading and writing
  if((trj_fd = open(trj_path, O_RDWR)) == -1) {
    printf("Error retrieving treasures file.\n");
    exit(3);
  }

  // read through the file and find the first unsuded id in range
  uint8_t used_ids[MAX_TRJ_CNT] = { 0 };
  struct trj_data_t curr_trj = { 0 };
  
  while(read(trj_fd, &curr_trj, sizeof(struct trj_data_t))) {
    used_ids[curr_trj.id]++;
  }

  uint8_t new_id = 0;
  while(used_ids[new_id]) { new_id++; }
  if(new_id >= MAX_TRJ_CNT) {
    printf("This hunt already holds value beyond imagination. Try assigning this treasure to another hunt or remove one treasure from this hunt and retry the operation.\n");
    exit(0);
  }

  char line[MAX_BUFFER_READ + 2]; // line read from stdin
  struct trj_data_t new_entry = { 0 }; // entry to be added to file
  new_entry.id = new_id;
  
  // prompt the user for information about the treasure entry
  printf("\n -- Now serving treasure %d: --\n", new_id);
  printf("Input the data as asked. In the case that the string inputed is longer than the limit shown, it will be truncated to fit the length.\n\n");
  
  // user id: 
  printf("Who adds this treasure? (max %d characters)\n", MAX_USRID_LEN);
  printf("user-id: ");

  if(fgets(line, MAX_USRID_LEN, stdin) == NULL) {
    printf("Error reading user-id.\n");
    exit(4);
  }

  // flush stdin buffer if input is longer
  if(line[strlen(line) - 1] != '\n') {
    while(fgetc(stdin) != '\n') {}
  }
  
  line[strlen(line) - 1] = '\0';
  //printf("%s\n", line); -- debug
  strcpy(new_entry.usr_id, line);

  //printf("%s\n", new_entry.usr_id); // -- debug

  // latitude
  printf("X marks the spot? (2 floats)\n");

  int is_lat_ok = 0;
  float lati_res = -1;
  
  do {
    printf("latitude: ");
    is_lat_ok = 1;
    
    if(fgets(line, MAX_BUFFER_READ, stdin) == NULL) {
      printf("Error reading float.\n");
      exit(31);
    }

    if(line[strlen(line) - 1] != '\n') {
      while(fgetc(stdin) != '\n') {}
    }
  
    line[strlen(line) - 1] = '\0';

    char* chrs = NULL;
    lati_res = strtof(line, &chrs);

    if(strlen(chrs)) {
      printf("Given input is not a valid float number. Try again.\n");
      is_lat_ok = 0;
    }
    
  } while(!is_lat_ok);

  new_entry.lati = lati_res;

  // longitude
  int is_long_ok = 0;
  float longi_res = -1;
  
  do {
    printf("longitude: ");
    is_long_ok = 1;
    
    if(fgets(line, MAX_BUFFER_READ, stdin) == NULL) {
      printf("Error reading float.\n");
      exit(32);
    }

    if(line[strlen(line) - 1] != '\n') {
      while(fgetc(stdin) != '\n') {}
    }
  
    line[strlen(line) - 1] = '\0';

    char* chrs = NULL;
    longi_res = strtof(line, &chrs);

    if(strlen(chrs)) {
      printf("Given input is not a valid float number. Try again.\n");
      is_long_ok = 0;
    }
    
  } while(!is_long_ok);

  new_entry.longi = longi_res;

  // flush newline from last input
  //fgetc(stdin);

  // description
  printf("I fancy a good story... (max %d characters)\n", MAX_TRJDESC_LEN);
  printf("Description: ");

  if(fgets(line, MAX_TRJDESC_LEN, stdin) == NULL) {
    printf("Error reading description.\n");
    exit(5);
  }

  // flush stdin buffer if input is longer
  if(line[strlen(line) - 1] != '\n') {
    while(fgetc(stdin) != '\n') {}
  }
  
  line[strlen(line) - 1] = '\0';
  strcpy(new_entry.desc, line);

  // value
  printf("Here for the booty! (integer)\n");

  int is_val_ok = 0;
  long val = -1;
  do {
    printf("value: ");
    is_val_ok = 1;

    if(fgets(line, MAX_BUFFER_READ, stdin) == NULL) {
      printf("Error reading value.\n");
      exit(34);
    }

    if(line[strlen(line) - 1] != '\n') {
      while(fgetc(stdin) != '\n') {}
    }
  
    line[strlen(line) - 1] = '\0';

    char* chrs = NULL;
    val = strtol(line, &chrs, 10);

    if(strlen(chrs)) {
      printf("Given input is not a valid value. Try again.\n");
      is_val_ok = 0;
    }
    else if(val < 0 || val > 255) {
      printf("Given input is not in range. Try again.\n");
      is_val_ok = 0;
    }
    
  } while(!is_val_ok);

  new_entry.val = (uint8_t)val;

  // flush newline from last input
  // helps if multiple treasures are added at the same time
  //fgetc(stdin);

  //printf("%d\n%s\n%f\n%f\n%s\n%d\n", new_entry.id, new_entry.usr_id, new_entry.lati, new_entry.longi, new_entry.desc, new_entry.val); // -- debug

  printf("Marking treasure down...\t");
  if(write(trj_fd, &new_entry, sizeof(new_entry)) == -1) {
    printf("Error writing treasure.\n");
    exit(6);
  }
  printf("Done.\n");

  //read(trj_fd, &new_entry, sizeof(new_entry)); // -- debug
  //printf("%d\n%s\n%f\n%f\n%s\n%d\n", new_entry.id, new_entry.usr_id, new_entry.lati, new_entry.longi, new_entry.desc, new_entry.val); // -- debug
  
  close(trj_fd);

  printf("Logging new changes...\t");
  if((log_fd = open(log_path, O_APPEND | O_WRONLY)) == -1) {
    printf("Error retrieving logs.\n");
    exit(7);
  }

  wr_log[0] = '\0';
  sprintf(wr_log, "User \'%s\' added treasure with index \'%d\'\n",
	  new_entry.usr_id,
	  new_entry.id);
  if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
    printf("Error updating logs.\n");
    exit(8);
  }
  printf("Done.\n");
    
  close(log_fd);
}

// function that lists the ids of all treasures found inside of a hunt
void list_treasures(char* hunt_id) {
  struct stat sb = { 0 }; // stat buffer
  char dir_path[MAX_PATH_LEN] = "./"; // local path prefix
  char log_path[MAX_PATH_LEN];
  char trj_path[MAX_PATH_LEN];

  int trj_fd, log_fd;
  char wr_log[MAX_LOG_LEN];
  wr_log[0] = '\0';

  strcat(dir_path, hunt_id); // whole path for directory (relative)

  strcpy(log_path, dir_path);
  strcat(log_path, "/");
  strcat(log_path, LOGFILE_NAME); // whole path for log file

  strcpy(trj_path, dir_path);
  strcat(trj_path, "/");
  strcat(trj_path, TRJFILE_NAME); // whole path for treasure file
  
  // if directory doesn't exist, let the user know
  if(stat(dir_path, &sb) && !S_ISDIR(sb.st_mode)) {
    printf("The hunt \'%s\' has not been started yet. Take the initiative, make your dreams come true!\n", hunt_id);
    return;
  }

  // if directory exists, display data about it
  printf("-- -- -- -- --\n");
  printf("Name: %s\n", hunt_id);
  printf("Total size: %ld\n", sb.st_size);
  printf("Last modification time: %s", ctime(&sb.st_mtime));
  printf("-- -- -- -- --\n");

  // then display the ids of the treasures inside
  if((trj_fd = open(trj_path, O_RDONLY)) == -1) {
    printf("Error retrieving treasures.\n");
    exit(9);
  }
  
  // if the directory exists, print minimal details about the treasures
  struct trj_data_t curr_trj = { 0 };
  int entries_read = 0;

  while(read(trj_fd, &curr_trj, sizeof(struct trj_data_t))) {
    printf("Treasure id: %3d\n", curr_trj.id);
    printf("Treasure owner: %s\n\n", curr_trj.usr_id);
    entries_read++;
  }

  // if no treasures exist, let the user know
  if(entries_read == 0) {
    printf("Oh wow, such empty.\n");
  }

  close(trj_fd);

  printf("Found %d treasures in this hunt.\n", entries_read);
  printf("-- -- -- -- --\n");

  // log the results in the log file
  if((log_fd = open(log_path, O_APPEND | O_WRONLY)) == -1) {
    printf("Error retrieving logs.\n");
    exit(10);
  }

  printf("Updating logs...\t");
  sprintf(wr_log, "Listed data about hunt \'%s\' and %d items\n", hunt_id, entries_read);
  
  if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
    printf("Error updating logs.\n");
    exit(11);
  }
  printf("Done.\n");

  close(log_fd);
}

// function that displays the actual data of a treasure given by id
void view_treasure(char* hunt_id, char* trj_id) {
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
  
  // if directory doesn't exist, notify user
  if(stat(dir_path, &sb) && !S_ISDIR(sb.st_mode)) {
    printf("The hunt you're looking for seems to be missing. Try lost and found?\n");
    return;
  }

  // convert the treasure id from char[] to uint8_t
  char* other_chrs = NULL;
  long trj_idl = strtol(trj_id, &other_chrs, 10);

  // check if treasure id is the right format
  // is integer in range 0 - 255 inclusive, only contains numbers
  // check is required since it is read as a char buffer
  //printf("%s\n", other_chrs); // -- debug
  if(strlen(other_chrs)) {
    printf("Treasure id is not a number. \n");
    return;
  }

  if(trj_idl < 0 || trj_idl > 255) {
    printf("Treasure id is out of range. (0 - 255)\n");
    return;
  }

  uint8_t trj_id8 = (uint8_t)trj_idl;

  //printf("%d\n", trj_id8); // --debug

  // integrity of id checked, now parse the file and look for the requested treasure
  if((trj_fd = open(trj_path, O_RDONLY)) == -1) {
    printf("Error retrieving treasures.\n");
    exit(12);
  }

  struct trj_data_t curr_trj = { 0 };
  int found = 0;
  while(read(trj_fd, &curr_trj, sizeof(struct trj_data_t))) {
    if(curr_trj.id == trj_id8) {
      printf("Treasure id: %d\n", curr_trj.id);
      printf("Treasure owner: %s\n", curr_trj.usr_id);
      printf("Latitude / longitutde: (%f, %f)\n", curr_trj.lati, curr_trj.longi);
      printf("Description: %s\n", curr_trj.desc);
      printf("Value: %d\n", curr_trj.val);
      
      found++;
      break;
    }
  }
  
  close(trj_fd);

  // if treasure is not indexed, let the user know
  if(!found) {
    printf("Looks like someone was one step ahead. No such treasures here.\n");
  }

  printf("\nLogging details...\t");
  if((log_fd = open(log_path, O_APPEND | O_WRONLY)) == -1) {
    printf("Error retrieving logs.\n");
    exit(13);
  }

  sprintf(wr_log, "Tried to view details about treasure \'%d\' in hunt \'%s\'...\t",
	  trj_id8,
	  hunt_id);
  if(found) {
    strcat(wr_log, "Success\n");
  }
  else {
    strcat(wr_log, "Not found\n");
  }

  if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
    printf("Error updating logs.\n");
    exit(14);
  }
  printf("Done.\n");

  close(log_fd);
}

// function that removes the data of a treasure given by id from the specified directory
void remove_treasure(char* hunt_id, char* trj_id) {
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
  
  // if directory doesn't exist, notify user
  if(stat(dir_path, &sb) && !S_ISDIR(sb.st_mode)) {
    printf("The hunter hungers for game, though he will not find it here.\n");
    return;
  }

  // convert the treasure id from char[] to uint8_t
  char* other_chrs = NULL;
  long trj_idl = strtol(trj_id, &other_chrs, 10);

  // check if treasure id is the right format
  // is integer in range 0 - 255 inclusive, only contains numbers
  // check is required since it is read as a char buffer
  //printf("%s\n", other_chrs); // -- debug
  if(strlen(other_chrs)) {
    printf("Treasure id is not a number. \n");
    return;
  }

  if(trj_idl < 0 || trj_idl > 255) {
    printf("Treasure id is out of range. (0 - 255)\n");
    return;
  }

  uint8_t trj_id8 = (uint8_t)trj_idl;

  // integrity of id checked, now parse the file and look for the requested treasure
  if((trj_fd = open(trj_path, O_RDWR)) == -1) {
    printf("Error retrieving treasures.\n");
    exit(15);
  }

  struct trj_data_t curr_trj = { 0 };
  int found = 0, cnt = 0;
  while(read(trj_fd, &curr_trj, sizeof(struct trj_data_t))) {
    cnt++;
    
    if(curr_trj.id == trj_id8) {
      found++;
      continue;
    }

    if(found) {
      if(lseek(trj_fd, -2 * sizeof(struct trj_data_t), SEEK_CUR) == -1) {
	printf("Error moving cursor. Treasure file might be corrupted!\n");
	exit(16);
      }

      if(write(trj_fd, &curr_trj, sizeof(struct trj_data_t)) == -1) {
	printf("Error deleting entry. Treasure file might be corrupted!\n");
	exit(17);
      }

      if(lseek(trj_fd, sizeof(struct trj_data_t), SEEK_CUR) == -1) {
	printf("Error moving cursor. Treasure file might be corrupted!\n");
	exit(18);
      }
    }
  }

  if(found) {
    struct trj_data_t empty = { 0 };
    if(lseek(trj_fd, -sizeof(struct trj_data_t), SEEK_END) == -1) {
	printf("Error moving cursor. Treasure file might be corrupted!\n");
	exit(19);
      }

    if(write(trj_fd, &empty, sizeof(struct trj_data_t)) == -1) {
      printf("Error deleting entry. Treasure file might be corrupted!\n");
      exit(20);
    }

    if((ftruncate(trj_fd, (cnt - 1) * sizeof(struct trj_data_t))) == -1) {
      printf("Error truncating file.\n");
      exit(21);
    }
  }

  close(trj_fd);

  // if treasure is not indexed, let the user know
  if(!found) {
    printf("Sadly, you cannot delete something that doesn't already exist.\n");
  }

  // log the operation
  printf("\nLogging details...\t");
  if((log_fd = open(log_path, O_APPEND | O_WRONLY)) == -1) {
    printf("Error retrieving logs.\n");
    exit(22);
  }

  sprintf(wr_log, "Tried deleting treasure \'%d\' in hunt \'%s\'...\t",
	  trj_id8,
	  hunt_id);
  if(found) {
    strcat(wr_log, "Success\n");
  }
  else {
    strcat(wr_log, "Not found\n");
  }

  if(write(log_fd, wr_log, (strlen(wr_log) + 1) * sizeof(char)) == -1) {
    printf("Error updating logs.\n");
    exit(23);
  }
  printf("Done.\n");

  close(log_fd);
}

// function that removes a directory given by id
void remove_hunt(char* hunt_id) {
  struct stat sb = { 0 }; // stat buffer
  char dir_path[MAX_PATH_LEN] = "./"; // local path prefix
  char log_path[MAX_PATH_LEN];
  char trj_path[MAX_PATH_LEN];
  char sym_path[MAX_PATH_LEN];

  if(hunt_id[strlen(hunt_id) - 1] == '/') {
    hunt_id[strlen(hunt_id) - 1] = '\0';
  }

  strcpy(sym_path, dir_path);
  strcpy(sym_path, "logged_hunt-<");
  strcat(sym_path, hunt_id);
  strcat(sym_path, ">"); // whole path for symlink

  strcat(dir_path, hunt_id); // whole path for directory (relative)

  strcpy(log_path, dir_path);
  strcat(log_path, "/");
  strcat(log_path, LOGFILE_NAME); // whole path for log file

  strcpy(trj_path, dir_path);
  strcat(trj_path, "/");
  strcat(trj_path, TRJFILE_NAME); // whole path for treasure file
  
  // if directory doesn't exist, notify user
  if(stat(dir_path, &sb) && !S_ISDIR(sb.st_mode)) {
    printf("The hunter hungers for game, though he will not find it here.\n");
    return;
  }

  // if directory exists, remove treasure, log, and symlink files
  if(unlink(sym_path) == -1) {
    printf("Error unlinking symbolic link.\n");
    exit(30);
  }
  
  if(unlink(trj_path) == -1) {
    printf("Error unlinking treasure file.\n");
    exit(24);
  }

  if(unlink(log_path) == -1) {
    printf("Error unlinking log file.\n");
    exit(25);
  }

  // then delete the directory itself
  if(rmdir(dir_path) == -1) {
    printf("Error removing directory.\n");
    exit(25);
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
    /*
    if(argc != 3) {
      if(argc > 3)
	printf("Did not yet implement adding multiple treasures at a time. Use one argument\n");
      else
	printf("Option takes argument <hunt_id>\n");
      
      print_usage(argv[0]);
      exit(0);
    }

    add_treasure(argv[2]); // adds a treasure to given hunt by id
    */
    if(argc < 3) {
      printf("Option takes at least one argument <hunt_id>\n");
      print_usage(argv[0]);
      exit(0);
    }

    for(int i = 2; i < argc; i++) {
      add_treasure(argv[i]);
    }
    
    break;
  case LIST:
    if(argc < 3) {
      printf("Option takes at least one argument <hunt_id>\n");
      print_usage(argv[0]);
      exit(0);
    }

    for(int i = 2; i < argc; i++) {
      list_treasures(argv[i]);
    }
    
    break;
  case VIEW:
    if(argc != 4) {
      printf("Option takes one argument pair <hunt_id> <treasure_id>\n");
      print_usage(argv[0]);
      exit(0);
    }

    view_treasure(argv[2], argv[3]);
    
    break;
  case RM_T:
    if(argc != 4) {
      printf("Option takes one argument pair <hunt_id> <treasure_id>\n");
      print_usage(argv[0]);
      exit(0);
    }

    remove_treasure(argv[2], argv[3]);
    
    break;
  case RM_H:
    if(argc < 3) {
      printf("Option takes at least one argument <hunt_id>\n");
      print_usage(argv[0]);
      exit(0);
    }

    for(int i = 2; i < argc; i++) {
      remove_hunt(argv[i]);
    }
    
    break;
  default:
    printf("Somehow you managed to get past the option check and landed outside the defined range of options. I am truly impressed\n");
    print_usage(argv[0]);
    break;
  }
  
  return 0;
}


