#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "oufs_lib.h"

int main(int argc, char **argv)
{

   
    char cwd[MAX_PATH_LENGTH];

    char disk_name[MAX_PATH_LENGTH];
    oufs_get_environment(cwd, disk_name);

    if(argc == 2)
    {
        vdisk_disk_open(disk_name);

	
        oufs_ztouch(cwd, argv[1]);

        vdisk_disk_close();

    }
    else
    {
        fprintf(stderr, "Usage: zmkdir <dirname>\n");
    }

    // open disk
    
   
    // get environ vars



}
