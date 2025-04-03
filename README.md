# Proiect-SO
This repository holds the project for the OS laboratory

## Working files description:
     
* <hunt_id> - directory named <hunt_id> containing other files, e.g. treasure files, logs, other. Id's are unique between directories, and attempting to appoint the same id to a new directory returns an error. Each hunt directory can contain multiple treasure files
	     
* <treasure_file>.trj - binary file containing data about one or multiple treasures submited by a single user. File names are unique in the same directory. Each user generates a different treasure file. If one user submits more than one treasure to the same hunt it gets appended to its corresponding file. It contains the following data:

username : string - maximum 63 characters, terminated by a null character (64 bytes). Represents the user that submited the treasures to the hunt
	
table size : uint8 - 8-bit unsigned integer. Holds the number of entries in the file
	
table entries : [] uint32 - <table size> 32-bit unsigned integers. Each <table entry> retains the byte at which each <treasure entry> starts in the file
	
<treasure entries> : [] - each entry contains multiple fields of varying sizes. Contained fields: treasure id, longitude, latitude, description, value

treasure id : uint8 - 8-bit unsigned integer. Holds a unique id for each <treasure entry> in the file. Id's can be ordered or randomly assigned

longitude : float - floating point number representing the longitude for the treasure in the current entry

latitude : float - floating point number representing the latitude for the treasrure in the current entry

description : string - maximum 127 characters, terminated by a null character (128 bytes). Holds a short description of the current treasure in the entry

value : uint16 - 16-bit unsigned integer. Holds the assigned physical value of the treasure in the current entry

**NOTE**: it could be preferable that the string buffers (username, description) be the shown size regardless of how many characters are actually in the string, by filling up the rest of the string with null values
	eg. if the username is 'John Doe' which has 9 bytes (8 character bytes + 1 null byte), then in memory it would look like '4A 6F 68 6E 20 44 6F 65 00 00 ... 00' (null characters up to the 64 byte limit)

* logged_hunt.log - text file that keeps track of every operation executed on a <treasure_file> from this hunt. Each operation adds a new line at the end of the file, detailing the executed operation, and the target file.

## Working files structure:

## Usage:
