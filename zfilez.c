#include <stdio.h>
#include <string.h>
#include "oufs_lib.h"
#include "vdisk.h"

int main(int argc, char ** argv)
{
 

    /*
       Shell command:

       ./zfilez [<name>]
    */

    /* Behavior: */

    // TODO: No name is specified; list the current working directory
    
    // A directory is specified: give the name of a file

    // A file is specified: give the name of a file

    // It is an error if name does not exist in the file system

 
    /* Output Specifics: */

    // Names are printed on per line (like ls -1)

    // Printed names are shown and not the rest of the path. If the 
    // the directory

    char disk_name[MAX_PATH_LENGTH];
    char cwd[MAX_PATH_LENGTH];

    INODE_REFERENCE inode_ref;
    INODE inode;


    oufs_get_environment(cwd, disk_name);
    vdisk_disk_open(disk_name);

    if(argc == 1) // if './filez' is typed, then list cwd
    {
        oufs_list(cwd, cwd);
    }
    else if(argc > 1) // if a file is selected, list it
    {
	// check if the argv is a file or directory
        oufs_list(cwd, argv[1]);
    }
    else
    {
        fprintf(stderr, "invalid number of arguements\n");
    }
    vdisk_disk_close();
   
}

