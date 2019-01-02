/**
Make a directory in the OU File System.

CS3113

*/

#include <stdio.h>
#include <string.h>

#include "oufs_lib.h"

int main(int argc, char** argv) 
{
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  oufs_get_environment(cwd, disk_name);

  int flag = 0;
  BLOCK block;
  // Check arguments
  if(argc == 2) 
  {
    // Open the virtual disk
    vdisk_disk_open(disk_name);

    vdisk_read_block(0, &block);
    for(int i = 0; i < 7; i++)
    {
	if(block.master.inode_allocated_flag[i] != 0xff)
	{
	    flag = -1;

	}
    }	

    if(flag == 0)
    {
	fprintf(stderr, "All inodes are full!\n");
	return -1;

    }
    for(int i = 0; i < 16; i++)
    {
	if(block.master.block_allocated_flag[i] != 0xff)
	{
	    flag = -2;
	    

	}
    }

    if(flag == -1)
    {
	fprintf(stderr, "All blocks are full!\n");
    
	return -1;
    }

    // Make the specified directory
    oufs_mkdir(cwd, argv[1]);

    // Clean up
    vdisk_disk_close();
    
  }else{
    // Wrong number of parameters
    fprintf(stderr, "Usage: zmkdir <dirname>\n");
  }

}
