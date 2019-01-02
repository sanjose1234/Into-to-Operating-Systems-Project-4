#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "oufs_lib.h"

#define debug 0
/**
 * Read the ZPWD and ZDISK environment variables & copy their values into cwd and 
 * disk_name.
 * If these environment variables are not set, then reasonable defaults are given.
 *
 * @param cwd String buffer in which to place the OUFS current working directory.
 * @param disk_name String buffer containing the file name of the virtual disk.
 */
void oufs_get_environment(char *cwd, char *disk_name)
{
  // Current working directory for the OUFS
  char *str = getenv("ZPWD");
  if(str == NULL) {
    // Provide default
    strcpy(cwd, "/");
  }else{
    // Exists
    strncpy(cwd, str, MAX_PATH_LENGTH-1);
  }

  // Virtual disk location
  str = getenv("ZDISK");
  if(str == NULL) {
    // Default
    strcpy(disk_name, "vdisk1");
  }else{
    // Exists: copy
    strncpy(disk_name, str, MAX_PATH_LENGTH-1);
  }

}

/**
 * Configure a directory entry so that it has no name and no inode
 *
 * @param entry The directory entry to be cleaned
 */
void oufs_clean_directory_entry(DIRECTORY_ENTRY *entry) 
{
  entry->name[0] = 0;  // No name
  entry->inode_reference = UNALLOCATED_INODE;
}

/**
 * Initialize a directory block as an empty directory
 *
 * @param self Inode reference index for this directory
 * @param self Inode reference index for the parent directory
 * @param block The block containing the directory contents
 *
 */
void oufs_clean_directory_block(INODE_REFERENCE self, INODE_REFERENCE parent, BLOCK *block)
{
  // Debugging output
  if(debug)
    fprintf(stderr, "New clean directory: self=%d, parent=%d\n", self, parent);

  // Create an empty directory entry
  DIRECTORY_ENTRY entry;
  oufs_clean_directory_entry(&entry);

  // Copy empty directory entries across the entire directory list
  for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
    block->directory.entry[i] = entry;
  }

  // Now we will set up the two fixed directory entries

  // Self
  strncpy(entry.name, ".", 2);
  entry.inode_reference = self;
  block->directory.entry[0] = entry;

  // Parent (same as self
  strncpy(entry.name, "..", 3);
  entry.inode_reference = parent;
  block->directory.entry[1] = entry;
  
}

/**
 * Allocate a new data block
 *
 * If one is found, then the corresponding bit in the block allocation table is set
 *
 * @return The index of the allocated data block.  If no blocks are available,
 * then UNALLOCATED_BLOCK is returned
 *
 */
BLOCK_REFERENCE oufs_allocate_new_block()
{
  BLOCK block;
  // Read the master block
  vdisk_read_block(MASTER_BLOCK_REFERENCE, &block);

  // Scan for an available block
  int block_byte;
  int flag;

  // Loop over each byte in the allocation table.
  for(block_byte = 0, flag = 1; flag && block_byte < N_BLOCKS_IN_DISK / 8; ++block_byte) {
    if(block.master.block_allocated_flag[block_byte] != 0xff) {
      // Found a byte that has an opening: stop scanning
      flag = 0;
      break;
    };
  };
  // Did we find a candidate byte in the table?
  if(flag == 1) {
    // No
    if(debug)
      fprintf(stderr, "No blocks\n");
    return(UNALLOCATED_BLOCK);
  }

  // Found an available data block 

  // Set the block allocated bit
  // Find the FIRST bit in the byte that is 0 (we scan in bit order: 0 ... 7)
  int block_bit = oufs_find_open_bit(block.master.block_allocated_flag[block_byte]);

  // Now set the bit in the allocation table
  block.master.block_allocated_flag[block_byte] |= (1 << block_bit);

  // Write out the updated master block
  vdisk_write_block(MASTER_BLOCK_REFERENCE, &block);

  if(debug)
    fprintf(stderr, "Allocating block=%d (%d)\n", block_byte, block_bit);

  // Compute the block index
  BLOCK_REFERENCE block_reference = (block_byte << 3) + block_bit;

  if(debug)
    fprintf(stderr, "Allocating block=%d\n", block_reference);
  
  // Done
  return(block_reference);
}


/**
 *  Given an inode reference, read the inode from the virtual disk.
 *
 *  @param i Inode reference (index into the inode list)
 *  @param inode Pointer to an inode memory structure.  This structure will be
 *                filled in before return)
 *  @return 0 = successfully loaded the inode
 *         -1 = an error has occurred
 *
 */
int oufs_read_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug)
    fprintf(stderr, "Fetching inode %d\n", i);

  // Find the address of the inode block and the inode within the block
  BLOCK_REFERENCE block = i / INODES_PER_BLOCK + 1;
  int element = (i % INODES_PER_BLOCK);

  BLOCK b;
  if(vdisk_read_block(block, &b) == 0) {
    // Successfully loaded the block: copy just this inode
    *inode = b.inodes.inode[element];
    return(0);
  }
  // Error case
  return(-1);
}

/**
  * oufs find open function
  *
  *    
  * Cited by: Lucas Bowker
  *
  */
int oufs_find_open_bit(unsigned char value)
{
    
    // block_bit will be used as a counter for the amount of times
    // right shifting is needed to make value to zero
    
    int block_bit = 0;

   
    for(unsigned char i = (value ^ 0xff); i > 0; i /= 2)
    {
        if(i%2 == 1)
        {
            return block_bit;
        }
        ++block_bit;

    }
    return block_bit;
    // while the value is not zero
  
}

/**
  * This function will creates a file
  * @param cwd - pass in current working directory
  * @param path - pass in path 
  */
int oufs_ztouch(char *cwd, char* path)
{


    // inode references for the parent child
    INODE_REFERENCE parentRef;
    INODE_REFERENCE childRef;

    // block object
    BLOCK block;
    //INODE_REFERENCE inodeRef;
    INODE parent;
    // holds the local namee
    char local_name[MAX_PATH_LENGTH];
    // retrun flag
    int ret;

    int flag = 0;
    // Attempt to find the specified directory
    if((ret = oufs_find_file(cwd, path, &parentRef, &childRef, local_name)) < -1)
    {
	if(debug)
	    fprintf(stderr, "oufs_ztouch(): ret = %d\n", ret);
	return(-1);
    }
    if(childRef != UNALLOCATED_BLOCK)
    {
	return -1;
    }

    // clean memory then read inode by reference 
    memset(&block, 0, 256);
    oufs_read_inode_by_reference(parentRef, &parent);
    // then read block of the the mster parent block
    vdisk_read_block(parent.data[0], &block);
    
    // loop directory  then string local_name to entry name 
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
	if(block.directory.entry[i].inode_reference == UNALLOCATED_INODE)
	{

	    //block.directory.entry[i].inode_reference = inodeRef;
	    strcpy(block.directory.entry[i].name, local_name);
	 
	    // flag is good
	    flag = i;
	    

	    break;
	}	
    }
    
    // if directory block is full, then error
    if(flag == 0)
    {
	fprintf(stderr, "Directory block is full!\n");
	return -1;
    }
    // allocate new inode, then assing inode refference 
    INODE_REFERENCE inodeRef = oufs_allocate_new_inode();
    block.directory.entry[flag].inode_reference = inodeRef;

    // write the parent master block 
    vdisk_write_block(parent.data[0], &block);

    // set variables that actually create a file, write it
    INODE in;
    memset(&in, 0, 36);
    in.type = 'F';
    in.n_references = 1;
    for(int i = 0; i < BLOCKS_PER_INODE; ++i)
    {
	in.data[i] = UNALLOCATED_BLOCK;
    }
    in.size = 0;

    // locad the parent reference
    oufs_read_inode_by_reference(parentRef, &parent);
    // increment the parent size, write the parent and child reference
    parent.size++;
    oufs_write_inode_by_reference(parentRef, &parent);
    oufs_write_inode_by_reference(inodeRef, &in);
    return 1;

}

/**
 *  Make a new directory
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_mkdir(char *cwd, char *path)
{
    // variables that will hold parent and child inode references 

    INODE_REFERENCE parentRef;
    INODE_REFERENCE childRef;

    // block object, and inode object 
    BLOCK block;
    //INODE_REFERENCE inodeRef;
    INODE parent;
    char local_name[MAX_PATH_LENGTH];
    // return flag
    int ret;

    int flag = 0;
    // Attempt to find the specified directory, throw error when necessary, break
    if((ret = oufs_find_file(cwd, path, &parentRef, &childRef, local_name)) < -1)
    {
	if(debug)
	    fprintf(stderr, "oufs_mkdir(): ret = %d\n", ret);
	return(-1);
    }
    if(childRef != UNALLOCATED_BLOCK)
    {
	return -1;
    }

    // clean memory, read inode parent reference, vdisk read parent data
    memset(&block, 0, 256);
    oufs_read_inode_by_reference(parentRef, &parent);
    vdisk_read_block(parent.data[0], &block);
    
    // loop direcotry entries, if the inode reference is unallocated,
    // then copy local name to entry name 
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
	if(block.directory.entry[i].inode_reference == UNALLOCATED_INODE)
	{

	    //block.directory.entry[i].inode_reference = inodeRef;
	    strcpy(block.directory.entry[i].name, local_name);
	 
	    flag = i;
	    

	    break;
	}	
    }
    
    // if directory block is full
    if(flag == 0)
    {
	fprintf(stderr, "Directory block is full!\n");
	return -1;
    }
    // allocate new inode entry flag inode set to inode reference 
    INODE_REFERENCE inodeRef = oufs_allocate_new_inode();
    block.directory.entry[flag].inode_reference = inodeRef;

    // vdisk write parent block
    vdisk_write_block(parent.data[0], &block);

    // set variables to actually make a directory
    INODE in;
    memset(&in, 0, 36);
    in.type = 'D';
    in.n_references = 1;
    in.data[0] = oufs_allocate_new_block();
    for(int i = 1; i < BLOCKS_PER_INODE; ++i)
    {
	in.data[i] = UNALLOCATED_BLOCK;
    }
    in.size = 2;
    oufs_write_inode_by_reference(inodeRef, &in);
    // clean memory, set dblocks to correct values, then write the block
    memset(&block, 0, 256);
    strcpy(block.directory.entry[0].name, ".");
    strcpy(block.directory.entry[1].name, "..");
    block.directory.entry[0].inode_reference = inodeRef;
    block.directory.entry[1].inode_reference = parentRef;

   
    for(int i = 2; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
	block.directory.entry[i].inode_reference = UNALLOCATED_INODE;
    }
    vdisk_write_block(in.data[0], &block);
    // read the inode, then update changes
    oufs_read_inode_by_reference(parentRef, &parent);
    parent.size++;
    oufs_write_inode_by_reference(parentRef, &parent);
    return 1;

}

/**
 *  oufs_format_disk 
 *
 *  @param virtual_disk_name will pass in the name of the virtual disk
 *
 *  @return 0 = successfully loaded the inode
 *
 */
int oufs_format_disk(char *virtual_disk_name)
{


    //Set all of the disk to zero initially.
    BLOCK b;
    memset(&b, 0, 256);
    for(int i = 0; i < N_BLOCKS_IN_DISK; ++i)
    {
	vdisk_write_block(i, &b.data);
    }


    //Set master block appropriately.

    b.master.inode_allocated_flag[0] = 0x01;

    for(int i = 1; i < N_INODES >> 3; ++i)
    {
	b.master.inode_allocated_flag[i] = 0x00;
    }
    b.master.block_allocated_flag[0] = 0xff;
    b.master.block_allocated_flag[1] = 0x03;
    for(int i = 2; i < N_BLOCKS_IN_DISK >> 3; ++i)
    {
	b.master.block_allocated_flag[i] = 0x00;
    }
    vdisk_write_block(0, &b.master);

    //Set inode[0] appropriately.
    memset(&b, 0, 256);
    b.inodes.inode[0].type = IT_DIRECTORY;
    b.inodes.inode[0].n_references = 1;
    b.inodes.inode[0].data[0] = 9;
    for(int i = 1; i < BLOCKS_PER_INODE; ++i)
    {
	b.inodes.inode[0].data[i] = UNALLOCATED_BLOCK;
    }
    b.inodes.inode[0].size = 2;
    //Catch the rest of this block's inodes and set appropriately.
    for(int i = 1; i < INODES_PER_BLOCK; ++i)
    {
	b.inodes.inode[i].type = IT_NONE;
	b.inodes.inode[i].n_references = 1;
	for(int j = 0; j < BLOCKS_PER_INODE; ++j)
	{
	    b.inodes.inode[i].data[j] = UNALLOCATED_BLOCK;
	}
	b.inodes.inode[i].size = 0;
    }
    vdisk_write_block(1, &b.inodes);
    
    //Set the rest of the inodes appropriately.
    memset(&b, 0, 256);
    for(int i = 0; i < INODES_PER_BLOCK; ++i)
    {
	b.inodes.inode[i].type = IT_NONE;
	b.inodes.inode[i].n_references = 1;
	for(int j = 0; j < BLOCKS_PER_INODE; ++j)
	{
	    b.inodes.inode[i].data[j] = UNALLOCATED_BLOCK;
	}
	b.inodes.inode[i].size = 0;
    }
    for(int i = 2; i < N_INODE_BLOCKS + 1; ++i)
    {
	vdisk_write_block(i, &b.inodes);
    }
    //Setting up root directory in block 9.
    memset(&b, 0, 256);
    strncpy(b.directory.entry[0].name, ".", FILE_NAME_SIZE);
    strncpy(b.directory.entry[1].name, "..", FILE_NAME_SIZE);
    for(int i = 2; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
	b.directory.entry[i].inode_reference = UNALLOCATED_INODE;
    }
    // write the root directory block
    vdisk_write_block(ROOT_DIRECTORY_BLOCK, &b.directory);

    return 0;
}


/**
 *  Given a current working directory and either an absolute or relative path, find both the inode of the
 * file or directory and the inode of the parent directory.  If one or both are not found, then they are
 * set to UNALLOCATED_INODE.
 *
 *  This implementation handles a variety of strange cases, such as consecutive /'s and /'s at the end of
 * of the path (we have to maintain some extra state to make this work properly).
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file/directory to be found
 * @param parent Inode reference for the parent directory
 * @param child  Inode reference for the file or directory specified by path
 * @param local_name String name of the file or directory without any path information (i.e., name relative
 *        to the parent)
 * @return 0 if no errors
 *         -1 if child not found
 *         -x if an error
 *
 */
int oufs_find_file(char *cwd, char * path, INODE_REFERENCE *parent, INODE_REFERENCE *child, char *local_name)
{
  // grand parent 
  INODE_REFERENCE grandparent;
  char full_path[MAX_PATH_LENGTH];

  // Construct an absolute path the file/directory in question
  if(path[0] == '/')
  {
    strncpy(full_path, path, MAX_PATH_LENGTH-1);
  }
  else // the path is not containing a root
  {
    if(strlen(cwd) > 1)
    {
      strncpy(full_path, cwd, MAX_PATH_LENGTH-1);
      strncat(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-1-strnlen(full_path, MAX_PATH_LENGTH));
    }else{
      strncpy(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-2);
    }
  }

  if(debug) {
    fprintf(stderr, "Full path: %s\n", full_path);
  };

  // Start scanning from the root directory
  // Root directory inode
  grandparent = *parent = *child = 0;
  if(debug)
    fprintf(stderr, "Start search: %d\n", *parent);

  // Parse the full path
  char *directory_name;
  directory_name = strtok(full_path, "/");
  while(directory_name != NULL) {
    if(strlen(directory_name) >= FILE_NAME_SIZE-1) 
      // Truncate the name
      directory_name[FILE_NAME_SIZE - 1] = 0;
    if(debug){
      fprintf(stderr, "Directory: %s\n", directory_name);
    }
    if(strlen(directory_name) != 0) {
      // We have a non-empty name
      // Remember this name
      if(local_name != NULL) {
	// Copy local name of file 
	strncpy(local_name, directory_name, MAX_PATH_LENGTH-1);
	// Make sure we have a termination
	local_name[MAX_PATH_LENGTH-1] = 0;
      }

      // Real next element
      INODE inode;
      // Fetch the inode that corresponds to the child
      if(oufs_read_inode_by_reference(*child, &inode) != 0) {
	return(-3);
      }

      // Check the type of the inode
      if(inode.type != 'D') {
	// Parent is not a directory
	*parent = *child = UNALLOCATED_INODE;
	return(-2);  // Not a valid directory
      }
      // Get the new inode that corresponds to the name by searching the current directory
      INODE_REFERENCE new_inode = oufs_find_directory_element(&inode, directory_name);
      grandparent = *parent;
      *parent = *child;
      *child = new_inode;
      if(new_inode == UNALLOCATED_INODE) {
	// name not found
	//  Is there another (nontrivial) step in the path?
	//  Loop until end or we have found a nontrivial name
	do {
	  directory_name = strtok(NULL, "/");
	  if(directory_name != NULL && strlen(directory_name) >= FILE_NAME_SIZE-1) 
	    // Truncate the name
	    directory_name[FILE_NAME_SIZE - 1] = 0;
	}while(directory_name != NULL && (strcmp(directory_name, "") == 0));
	
	if(directory_name != NULL) {
	  // There are more sub-items - so the parent does not exist
	  *parent = UNALLOCATED_INODE;
	};
	// Directory/file does not exist
	return(-1);
      };
    }
    // Go on to the next directory
    directory_name = strtok(NULL, "/");
    if(directory_name != NULL && strlen(directory_name) >= FILE_NAME_SIZE-1) 
      // Truncate the name
      directory_name[FILE_NAME_SIZE - 1] = 0;
  };

  // Item found.
  if(*child == UNALLOCATED_INODE) {
    // We went too far - roll back one step ***
    *child = *parent;
    *parent = grandparent;
  }
  if(debug) {
    fprintf(stderr, "Found: %d, %d\n", *parent, *child);
  }
  // Success!
  return(0);

}

/**
  * will allocate new direcotry
  * @param parent - pass in INODEreference for the parent
  *
  */
INODE_REFERENCE oufs_allocate_new_directory(INODE_REFERENCE parent)
{

    BLOCK block;
    INODE inode;
    // read inode parent
    oufs_read_inode_by_reference(parent, &inode);
    vdisk_read_block(inode.data[0], &block);

    // loop directory entreis per block and if unnallocated, allocate them
    for (int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++)
    {
        if(block.directory.entry[i].inode_reference == UNALLOCATED_INODE)
        {
            block.directory.entry[i].inode_reference = oufs_allocate_new_inode();
            
        }

    }

    return -1;
}

/**
  * allocate a new inode
  *
  *
  */
INODE_REFERENCE oufs_allocate_new_inode()
{
  BLOCK block;
  // Read the master block
  vdisk_read_block(MASTER_BLOCK_REFERENCE, &block);

  // Scan for an available block
  int block_byte;
  int flag;

  // Loop over each byte in the allocation table.
  for(block_byte = 0, flag = 1; flag && block_byte < N_INODES / 8; ++block_byte) {
    if(block.master.inode_allocated_flag[block_byte] != 0xff) {
      // Found a byte that has an opening: stop scanning
      flag = 0;
      break;
    };
  };
  // Did we find a candidate byte in the table?
  if(flag == 1) {
    // No
    if(debug)
      fprintf(stderr, "No blocks\n");
    return(UNALLOCATED_INODE);
  }

   // Set the block allocated bit
  // Find the FIRST bit in the byte that is 0 (we scan in bit order: 0 ... 7)
  int block_bit = oufs_find_open_bit(block.master.inode_allocated_flag[block_byte]);

  // Now set the bit in the allocation table
  block.master.inode_allocated_flag[block_byte] |= (1 << block_bit);

  // Write out the updated master block
  vdisk_write_block(MASTER_BLOCK_REFERENCE, &block);

  if(debug)
    fprintf(stderr, "Allocating block=%d (%d)\n", block_byte, block_bit);

  // Compute the block index
  INODE_REFERENCE block_reference = (block_byte << 3) + block_bit;

  if(debug)
    fprintf(stderr, "Allocating block=%d\n", block_reference);
  
  // Done
  return(block_reference);


}

/**
  * find a directory element 
  * @param inode 
  * @param direcotyr_name
  */
INODE_REFERENCE oufs_find_directory_element(INODE *inode, char *directory_name)
{
    
    BLOCK block;
    memset(&block, 0, 256);
    vdisk_read_block((*inode).data[0], &block);
    // if the entry names and the directory names equals zero, then return them
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++)
    {
        if(strcmp(block.directory.entry[i].name, directory_name) == 0)
        {
            return block.directory.entry[i].inode_reference;

        }
    }
    return -1;
    
    
}

/**
  * write the inode by reference  
  *
  */
int oufs_write_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{

    if(debug)
        fprintf(stderr, "Fetching inode %d\n", i);
  
    // Find the address of the inode block and the inode within the block
    BLOCK_REFERENCE block = i / INODES_PER_BLOCK + 1;
    int element = (i % INODES_PER_BLOCK);

    BLOCK b;
    if(vdisk_read_block(block, &b) == 0)
    {

        // Successfully loaded the block: copy just this inode
        b.inodes.inode[element] = *inode;
        vdisk_write_block(block, &b);
        return(0);
    }
    // Error case
  
    return(-1);
    
}

OUFILE* oufs_fopen(char *cwd, char *path, char *mode)
{
    //INODE_REFERENCE



    return NULL;

}

// TODO: cite in README 
//  https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
int cstring_cmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
	/* strcmp functions works exactly as expected from
	comparison function */ 
}

/*
 * list the files
 *
 *
 */
int oufs_list(char *cwd, char *path)
{
   
    INODE_REFERENCE parent;
    INODE_REFERENCE child;
    char local_name[MAX_PATH_LENGTH];
    // find the file
    oufs_find_file(cwd, path, &parent, &child, local_name);



    // read the inode and block
    INODE ichild;
    oufs_read_inode_by_reference(child, &ichild);
    BLOCK block;

    vdisk_read_block(ichild.data[0], &block);

    int size = 0;
    char *newArray[DIRECTORY_ENTRIES_PER_BLOCK];
    // if inode reference is unallocated, then copy arrays
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++)
    {
        if(block.directory.entry[i].inode_reference != UNALLOCATED_INODE)
        {

            // copy into array; keep track
            newArray[size] = block.directory.entry[i].name;

            size++;
        }

    }
    
    //qsort(newArray, size of array...)

    qsort(newArray, size, sizeof(char*), cstring_cmp);

    // counter to parse through the directory entries per block
    int ctr = 0;
    // while the counter is not the size 
    while(ctr != size)
    {
	// as we parse through the directory entries per block
        for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++)
	{

	    // if the newArray is the same as the entry name, then print
	    if(strcmp(block.directory.entry[i].name, newArray[ctr]) == 0)
	    {
		// load the inode reference 
		oufs_read_inode_by_reference(block.directory.entry[i].inode_reference, &ichild);
		// if the inode the type is file
		if(ichild.type == 'F')
		{
		    // print without the '/'
		    printf("%s\n", newArray[ctr]);

		}
		else // else ichild is a directory, so append a '/', then print
		{
		    strcat(newArray[ctr], "/");
		    printf("%s\n", newArray[ctr]);

		}

	    }


	}

        ++ctr;

    }

    return 0;
}

/**
  * This function will remove a specified directory.
  *
  * @param cwd - will pass in current working directory string
  * @param path - will pass in the path specified
  *
  */
int oufs_rmdir(char *cwd, char *path)
{
    // INODE references for both the parent and child
    INODE_REFERENCE parentRef;
    INODE_REFERENCE childRef;

    // BLOCK block;
    // INODE objects for the parent and child
    INODE parent;
    INODE child;
    // string that holds the local name
    char local_name[MAX_PATH_LENGTH];
    // ret is used to hold the return value of when find_file is called
    int ret;
    //int inode_child_size;
  //  int flag = 0;


    // if find_file throws an error, then stderr
    if((ret = oufs_find_file(cwd, path, &parentRef, &childRef, local_name)) < -1)
    {
	if(debug)
	    fprintf(stderr, "oufs_rmdir(): ret = %d\n", ret);
//	return -1;
    }
  
    // if the child reference is unallocated, stderr and exit function
    if(childRef == UNALLOCATED_BLOCK)
    {
	fprintf(stderr, "Child is unallocated\n");
	return -1;

    }

    // read the inode by reference for the parent and child references
    oufs_read_inode_by_reference(parentRef, &parent);
    oufs_read_inode_by_reference(childRef, &child);

    //inode_child_size = child.size;
    //if(child.size == 2)
	//inode_child_size++;

    //printf("inode_child_size is %d\n", inode_child_size);
    //printf("child.size is %d\n", child.size);
    //printf("childRef is %d\n", childRef);
   
    //printf("inode parent is %d\n", parent);

    // if the child size is not 2, exit the program 
    if(child.size > 2)
    {
	fprintf(stderr, "Cannot remove a parent directory!\n");
	return -1;

    }
    else // child.size is <= 2
    {
	// block object used to access various data in the data structure
	BLOCK block;

	// clean memory block
	memset(&block, 0, 256);
	// write the child.data master block
	vdisk_write_block(child.data[0], &block);

	// clean the inode memory block
	memset(&block, 0, sizeof(INODE));
    
	// write the inode by reference for the child reference
	oufs_write_inode_by_reference(childRef, &child);
    
	// decrement size b/c we've removed the parent directory
	parent.size--;
	// write the inode parent reference
	oufs_write_inode_by_reference(parentRef, &parent);
   
	// read the parent data master block
	vdisk_read_block(parent.data[0], &block);
    
	// loop through the directory blocks
	for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++)
	{
	    if(strcmp(local_name, block.directory.entry[i].name) == 0)
	    {
		// set the directory entry names as an empty string
		strcpy(block.directory.entry[i].name, "");
		// assign parentRef to the inode reference before setting it to unallocated
		parentRef = block.directory.entry[i].inode_reference;
		// then set it as unallocated, b/c we've removed the directory
		block.directory.entry[i].inode_reference = UNALLOCATED_INODE;
	  
		// re-write the parent block, then break
		vdisk_write_block(parent.data[0], &block);
		break;
	    }

	}
    
	//printf("%d\n", childRef);
    
	// clean thee memory
	memset(&block, 0, 256);
	// read the master block
	vdisk_read_block(0, &block);
    
	// FIXME: bug after rmdir, master block is not updating correctly
	//printf("childRef before bit operation is %d\n", childRef);
    
	/*
	   with inode_allocated_flag, I divide by 8 to find the correct byte in the table,
	   then subtract it from the childRef % 8 so that I can locate the bit 
	 *
	 */
	block.master.inode_allocated_flag[childRef / 8] &= ~(1 << (childRef % 8));//(childRef % 8) << 1;
   

	//printf("childRef after bit operation is %d\n", childRef);

	BLOCK deallocated_block;

	// read the 
	vdisk_read_block(parent.data[0], &deallocated_block);
    
	block.master.block_allocated_flag[child.data[0] / 8] &= ~(1 << (child.data[0] % 8));//(parent.data[0] % 8) << 1;
	//printf("parent.data[0] is %d\n", 


	// printf("1 << (parent.data[0] % 8) is %d\n", (1 << (parent.data[0] % 8)));
   
	vdisk_write_block(0, &block);
	// TODO: do the same thing but with parentRef
    }
    return 0;
}

