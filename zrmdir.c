#include <stdio.h>
#include "vdisk.h"
#include <stdio.h>
#include <string.h>
#include "oufs_lib.h"
#include "oufs.h"

int main(int argc, char** argv) 
{
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  oufs_get_environment(cwd, disk_name);

  // Check arguments
  if(argc == 2) 
  {
    // Open the virtual disk
    vdisk_disk_open(disk_name);

    oufs_rmdir(cwd, argv[1]);

  }

}
