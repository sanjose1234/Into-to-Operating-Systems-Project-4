Project 4

Name: Eric Gonzalez

Email: eric.g.gonzalez-1@ou.edu

Email: 11/29/2018

Description: Project 4 will create a virtual disk that will essentially create miniature
file system. For project 3 namely will not focus on file actions until project 4. Firstly
I had to focus on zformat. zformat will format virtual the virtual disk using function 
from the API headers provided. First I want pass in the cwd and disk_name into the 
get enironment function. We need to do this so that our env is set up to perform necessary
actions. From there I called format disk; while passing disk_name. In format_disk, I need 
initialize various variables. I initialized the data array within the block. 
Then initialize the master block (namely the allocated flag to zero). Then set the 
master block to contain the correct variables for each element, while setting the 
correct byte to be allocated or not. I then write the master block. Looped through the block
read the inode block, loop through the seven blocks, while looping into fifteen blocks.
Then set each data array to be allocated. write the inode block. Then after the loops
read the second inode block. Set the type, n_references, and size. Then clean the dir_block
and write the dir block. This completes the format_disk. Then I worked on find_file function
that will be useful in various function. In find file will tokenize my path. But first i 
need to see if the path is a full path or relative path. Once i figure that out when the 
first char is a / or not i copy and concatenate cwd and path. loop through the tokens and 
read the BLOCK obj. loop through the dir_entries_per_block and see of the a directory 
matches a token. then update inode reference. Read the inode by reference then loop through the data array while it is not unallocated. From there just assign.  
Now in project 4, I need to do various actions that relate to the file system. ztouch will create a file. Essentially the same thing as mkdir but with a file.  
instead. 

Directions: The user will have different options to select from. zformat will format the 
virtual disk. zinspect will print out various portions in the data structure. zfilez 
will list the directories in the filesystem. zmkdirz will create a directory. zrmdirz will
remove a directory. ztouch will create a file. 

Any known bugs or assumptions made: 
- all of project 3 should be working properly. ztouch is completed. Any other project 4 commands have not been completed.

References used:
- http://man7.org/linux/man-pages/man2/open.2.html
- https://stackoverflow.com/questions/18415904/what-does-mode-t-0644-mean- https://www.die.net/search/?q=freopen&sa=Search&ie=ISO-8859-1&cx=partner-pub-5823754184406795%3A54htp1rtx5u&cof=FORID%3A9&siteurl=linux.die.net%2F&ref=www.die.net%2Fsearch%2F%3Fq%3Dfreopen%26sa%3DSearch%26ie%3DISO-8859-1%26cx%3Dpartner-pub-5823754184406795%253A54htp1rtx5u%26cof%3DFORID%253A9&ss=1109j301209j7
- https://linux.die.net/man/3/freopen
- http://pubs.opengroup.org/onlinepubs/009695399/functions/chdir.html
- http://www.cplusplus.com/reference/cstdio/fopen/
- http://www.cplusplus.com/reference/cstdio/fopen/
- https://www.google.com/search?q=how+to+exit+and+save+in+nano&rlz=1C1CHBF_enUS773US773&oq=how+to+exit+and+save+in+nano&aqs=chrome..69i57j0l3.5069j0j7&sourceid=chrome&ie=UTF-8
- http://pubs.opengroup.org/onlinepubs/009695299/functions/access.html
- https://linux.die.net/man/2/access
- https://stackoverflow.com/questions/15177378/recursive-directory-copying-in-c
- https://stackoverflow.com/questions/35185503/how-to-write-to-a-file-using-open-and-printf
- https://stackoverflow.com/questions/15102992/what-is-the-difference-between-stdin-and-stdin-fileno

Alex Kloppenburg, Job Villamil, Cavan Gary, Andrew Wyatt, Lucas Bowker
- explained the basics of the data structure
- walked through how each command should perform
- explained lecture material for OS
- explained the proper makefile that relate to this project 4
- explained the idea of how ztouch works
- ezpalined possible bugs when bit operations

