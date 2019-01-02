#include "vdisk.h"
#include <stdio.h>
#include <string.h>
#include "oufs_lib.h"
#include "oufs.h"

int main(int argc, char** argv) 
{

    // string that contains the disk name
    char disk_name[MAX_PATH_LENGTH];
     // string used as a  current working directory with the size of the max path length
    char cwd[MAX_PATH_LENGTH];
    
    // use custom API to fetch the key environment
    oufs_get_environment(cwd, disk_name);
    
    // open the name of the virtual disk name that is passed in
    vdisk_disk_open(disk_name);

    

    // create a data block that will be used for the following actions
    //BLOCK block;


    // write zeros to all bytes in the virtual disk
    
    // call oufs format disk to format disk name that is passed in
    oufs_format_disk(disk_name);

    vdisk_disk_close();
}
