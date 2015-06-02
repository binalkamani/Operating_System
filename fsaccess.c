/* Unix File system Modified*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;	//shows error number on wrong command
int inode_cnt;
int f_loop,f_loop1, counter = 0;
char ret1[100];
char ret2[100];

typedef void (*sighandler_t)(int);
typedef unsigned int uint;
static char *my_argv[100], *my_envp[100];

char newfile_path[1000];


#define BLOCK_SIZE 2048

#define FLAG_INODE_ALLOC 0x8000
#define FLAG_INODE_UNALLOC 0x0000
#define FLAG_INODE_PLN_FILE 0x0000
#define FLAG_INODE_DIRECTORY 0x4000
#define FLAG_INODE_UID 0x0800
#define FLAG_INODE_OWNR_ACCESS 0x01C0 //RWX
#define FLAG_INODE_GRP_ACCESS 0x0030 //RW
#define FLAG_INODE_OTHR_ACCESS 0x0004 //R

struct superblock {
//Total size 2048 bytes
uint isize;					//4
uint fsize;					//4
uint nfree;					//4	//which will state how many free blocks are there in superblock
uint free[100];				//400
uint ninode;				//4	//number of free inodes
unsigned short inode_cnt;	//2	//number of inodes which are assigned
// sum of ninode and inode_cnt will be total free inodes assigned by user at time of initfs
uint inode[100];			//400
char flock;					//1
char ilock;					//1
unsigned short fmod;		//2
uint time[2];				//4
unsigned short freeblks;    //2	//which will state total number of free blocks
unsigned short dir_ent;		//2 which will store number of directory entries written
char filesys_path[200];		//200
unsigned short dummy[505];	//1010
}sb;
struct superblock sb1;

struct inode {
//total size : 128 bytes
unsigned short flags;	// 2 byte
unsigned short nlinks;	// 2 byte
uint  uid;				// 4 byte
uint  gid;				// 4 byte
uint size0;				// 4 byte
uint size1;				// 4 byte
uint addr[26];			// 26*4 = 104 byte, max number of blocks = 25600 (26*1024)
char acttime[2];		// 2 byte
char modtime[2];		// 2 byte
};

struct dir_entry{
	unsigned short i_node;	//2 bytes
	char file_name[14];		//14 bytes
	};

struct dir_entry entry1;

struct inode node1[10000];

struct stat info;

struct stat attrib;
//char root_dir[2][16];
unsigned short nfree_default = 100;	//number of blocks in an array
unsigned short free_blk_cnt[100];	//free_block_count array of size 100

//printf("\nThis is inside check_command : %s \n", my_argv[0]);
int i = 0;
unsigned short temp;
int fd,fd1,bytes_written;	// fd = file descriptor, bytes_written = number of bytes written in file
long long int blocks_required;
int arrays_required, arrays_count;
long long int extfile_size, size, size1;

//Functions
void init_fs();
void free_argv();
void cp_in();
void check_input();

void handle_signal(int signo)	//signal handler
{
	printf("\n[Modified_V6 ] ");
	fflush(stdout);
}

/* This function takes the user input string as a parameter and parses it to fill my_argv data structure. */
void fill_argv(char *tmp_argv)
{
	char *foo = tmp_argv;
	int index = 0;
	char ret[100];
	bzero(ret, 100);	//write zero valued bytes
	while(*foo != '\0') {	//loop until enter is not pressed
		if(index == 4)		//number of arguments = 4 for command to be checked, not more than that
			break;

		if(*foo == ' ') {	//if space found take new argument parameter
			if(my_argv[index] == NULL)
				my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			else {
				bzero(my_argv[index], strlen(my_argv[index]));
			}
			strncpy(my_argv[index], ret, strlen(ret));
			strncat(my_argv[index], "\0", 1);

			bzero(ret, 100);
			index++;
		} else {
			strncat(ret, foo, 1);	//else concatenate two strings

		}
		foo++;

	}
	my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret));

	strncpy(my_argv[index], ret, strlen(ret));

	strncat(my_argv[index], "\0", 1);

}

void copy_envp(char **envp)
{
	int index = 0;
	for(;envp[index] != NULL; index++) {
		my_envp[index] = (char *)malloc(sizeof(char) * (strlen(envp[index]) + 1));
		memcpy(my_envp[index], envp[index], strlen(envp[index]));
	}
}



void get_filename(){


		char *foo1 = newfile_path;
		int index1 = 0;

		bzero(ret1, 100);

		while(*foo1 != '\0') {	//if space found take new argument parameter
			index1++;
			if(index1 == 25){

			if(my_argv[index1] == NULL)
				my_argv[index1] = (char *)malloc(sizeof(char) * strlen(ret1) + 1);
			else {
				bzero(my_argv[index1], strlen(my_argv[index1]));
			}
			strncpy(my_argv[index1], ret1, strlen(ret1));
			strncat(my_argv[index1], "\0", 1);
			//printf("\n This is inside fill_argv : %s", my_argv[index]);
			bzero(ret1, 100);

			}
		 else {
			strncat(ret1, foo1, 1);	//else concatenate two strings
		}
		foo1++;

		}


	my_argv[index1] = (char *)malloc(sizeof(char) * strlen(ret1));

	strncpy(my_argv[index1], ret1, strlen(ret1));		//printing some garbage value!

	strncat(my_argv[index1], "\0", 1);
	printf("now file from file path is %s\n", ret1);

}
/* Initilaise File system Command 1 */
void init_fs()
{
	sb.inode_cnt = 1;

	printf("\ncommand 1 detected\n");	//Identified command "initfs"

	if (access(my_argv[1],X_OK) == 0)	//If path is already there then just open fd
	{
		printf("\nPath is valid \n");
		//fopen(my_argv[1],"w");
		fd = open(my_argv[1],O_CREAT, 0777);
		printf("\n fd %d opened \n", fd);
	}
	else  								//Else create the path and file then open it
	{
		printf("\nPath doesnt exist, creating the directory \n");
		fd = open(my_argv[1],O_CREAT, 0777);
		printf("\n fd %d created \n", fd);
		if (access(my_argv[1],0) == 0)
		{
			printf("\nPath %s created \n", my_argv[1]);

		}
	}
	//strcpy(fs_path, my_argv[1]);	//fs_path is containing the path of filesystem
	printf("file path copied = %s" , sb.filesys_path);
	close(fd);
	/* To write the super block */
	fd = open(my_argv[1],O_RDWR|O_CREAT|O_TRUNC, 0666);
	//fd = open(fs_path,O_RDWR|O_CREAT|O_TRUNC, 0666);
	strcpy(sb.filesys_path, my_argv[1]);
	sb.nfree = (uint)atoi((char *)my_argv[2]);	//nfree = number of free blocks given by user
	sb.ninode = atoi((char *)my_argv[3]);		//ninode = number of free inodes given by user
	for(f_loop = 0; f_loop<609; f_loop++)		//To fill super block, remaining bytes are filled with dummy data
	{
		sb.dummy[f_loop] = f_loop;
	}

	printf("nfree = %d ninode = %d \n",sb.nfree,sb.ninode);

	struct inode node[sb.ninode+1];	// inode starts from 1.
	struct inode node1;				//	first inode = node1

	/*populating the inode 1 for root directory*/
	node[sb.inode_cnt].flags = (FLAG_INODE_ALLOC|FLAG_INODE_DIRECTORY|FLAG_INODE_UID|FLAG_INODE_OWNR_ACCESS|
							FLAG_INODE_GRP_ACCESS|FLAG_INODE_OTHR_ACCESS);	//set inode flags
	printf("inode flags = %d \n",node[sb.inode_cnt].flags);
	node[sb.inode_cnt].uid = 0;	//set uid for inode
	node[sb.inode_cnt].gid = 0; 	//set gid for inode
	node[sb.inode_cnt].size0 = 2;	//high word of 32-bit size
	node[sb.inode_cnt].size1 = 2;	//low word of 32-bit size

	stat(my_argv[1], &attrib);
	strcpy(node[sb.inode_cnt].acttime,ctime(&attrib.st_atime));	//time of last access
	strcpy(node[sb.inode_cnt].modtime,ctime(&attrib.st_mtime));	//time of last modification
	printf("acttime = %s \n",node[sb.inode_cnt].acttime);
	printf("modtime = %s \n",node[sb.inode_cnt].modtime);

	node[sb.inode_cnt].addr[0] = 100;	//write 100 in first 2 bytes of block of inode indicating number of free blocks

	for(f_loop = 0; f_loop < 25; f_loop++)	//initialize addrarray of size 26
	{
		node[sb.inode_cnt].addr[f_loop] = 0;
	}

	for(f_loop1 = 0; f_loop1 < 100; f_loop1++)	//initializing free_blk_cnt array of 100 elements
	{
		free_blk_cnt[f_loop1] = 0;
	}
	/*Initilialize free blocks, inode start from 1, first data block for /. & /.. */
	temp = (sb.ninode+sb.nfree+2-(sb.nfree%100))-100;
	//for example ninode = 300, nfree = 1010; then temp = 300+1010+2-10-100 = 1202
	for(f_loop = (sb.ninode+2); f_loop < temp; f_loop = f_loop + 100)	//loop start from 302 to 1201 for above example
	{
		counter = 0;
		for(f_loop1 = f_loop+100; f_loop1 < f_loop+200; f_loop1++)	//loop from 402 to 501
		{
			free_blk_cnt[counter] = f_loop1; //fill array with 100 elements "402 to 501"
			counter = counter + 1;
		}
		lseek(fd, f_loop*BLOCK_SIZE, SEEK_SET);	//set offset to 302*2048

		//write first 2 bytes of this block as 100 which will be number of free blocks available
		bytes_written = write(fd,&nfree_default,sizeof(nfree_default));
		//printf("data blk count = %d \n",bytes_written);
		//copy/write free_blk_cnt array to this block which contains next 100 address of blocks which are free
		bytes_written = write(fd,free_blk_cnt,sizeof(free_blk_cnt));
		//printf("data blk count = %d \n",bytes_written);
	}
	if((sb.nfree%100) > 0) // In our example it is 1010%100 = 10 i.e. > 0
	{
		counter = 0;
		temp = sb.nfree%100;
		f_loop = (sb.ninode+sb.nfree+2-(sb.nfree%100));	//floop = 300+1010+2-10 = 1302
		//printf("****************floop value is %d***************\n", f_loop);
		for(f_loop1 = f_loop; f_loop1 < (f_loop+temp); f_loop1++) // loop from 1302 to 1311
		{
			free_blk_cnt[counter] = f_loop1;	//fill 10 elements of free_blk_cnt
			counter = counter + 1;
		}
		i = counter-1;
		for(f_loop1 = i; f_loop1 < 100; f_loop1++)	// from 8 to onwards
		{
			free_blk_cnt[counter] = 0;
			//fill other elements of free_blk_cnt as 0 as those are not free and shouldnt be copied to superblock in case of allocation
			counter = counter + 1;
		}
		lseek(fd, (f_loop-100)*BLOCK_SIZE, SEEK_SET);	//set offset to 1302-100 = 1202*2048
		bytes_written = write(fd,&temp,sizeof(temp));	//temp = 1010%100 = 10
		//printf("bytes_written = %d \n",bytes_written);	//write first 2 bytes as 10-shows next number of free blocks
		bytes_written = write(fd,free_blk_cnt,sizeof(free_blk_cnt));	//write other 10 blocks data

	}

	temp = (sb.ninode+2);	//temp = 300+2 = 302
	for(f_loop1 = 0; f_loop1 < 100; f_loop1++)	//Loop of 100
	{
		sb.free[f_loop1] = temp;	//free blocks array in super block = temp (302 to 401)
		temp++;
	}

	entry1.i_node = sb.inode_cnt;
	strcpy(entry1.file_name,  ".");
	lseek(fd,(sb.ninode+sb.inode_cnt)*BLOCK_SIZE,SEEK_SET);
	bytes_written = write(fd,&entry1,sizeof(entry1));
	//printf("entry1 bytes written = %d \n",bytes_written);
	sb.dir_ent = 1;

	strcpy(entry1.file_name,  "..");
	lseek(fd,((sb.ninode+sb.inode_cnt)*BLOCK_SIZE)+16,SEEK_SET);
	bytes_written = write(fd,&entry1,sizeof(entry1));
	sb.dir_ent++;

	//printf("\nDirectory entries written\n");
	sb.ninode = sb.ninode - 1; 	// first inode allocated to root
	sb.freeblks = sb.nfree;		// number of free blocks which have been assigned by user
	sb.nfree = 100; 			//set the value to 100 as all the blocks are now written.
	lseek(fd,BLOCK_SIZE*1,SEEK_SET);	//skip first block
	bytes_written = write(fd,&sb,sizeof(sb));
	printf("sb bytes written = %d \n",bytes_written);

	lseek(fd,(BLOCK_SIZE*2),SEEK_SET);	//skip first two blocks
	bytes_written = write(fd,&node[1],sizeof(node[1]));
	printf("inode bytes written = %d \n",bytes_written);
	close(fd);

	fd = open(sb.filesys_path,O_RDWR, 0666);
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	read(fd,&sb1,BLOCK_SIZE);
	printf("fd1 = %d nfree_copied = %d ninode_copied = %d \n",fd, sb1.nfree,sb1.ninode);
	printf("dummy array = %d %d \n",sb1.dummy[0], sb1.dummy[40]);
	printf("free blocks copied to sb.freeblocks = %d \n",sb1.freeblks);
	printf("fs path : %s\n",sb1.filesys_path);

	close(fd);
	exit(1);
}

/* CPIN Command 2 */
void cp_in() //ext file v6 file
{
	int fd1,temp;
	strcpy(newfile_path, my_argv[2]);
	printf("path for file to be opened is %s\n", my_argv[1]);	//external file
	printf("path for file to be created is %s\n", my_argv[2]);	//V6 file

	char *cpargs[] = {"cp", my_argv[1], my_argv[2], (char *) 0 };

	strcpy(sb.filesys_path, "/home/004/v/vm/vmk13030/fs.txt");

	fd = open(sb.filesys_path,O_RDWR, 0666);
	//read data from fs.txt
	//modify accordingly and write back to fs.txt
	//Also write inode structure
	lseek(fd,BLOCK_SIZE,SEEK_SET);	//Go to super block
	temp = read(fd,&sb,BLOCK_SIZE);		//Read this block into fd
	printf("bytes of super block read for cpin = %d", temp);
	printf("nfree = %d ninode = %d \n",sb.nfree,sb.ninode);		//why values 0??

	//printf("file is created with inode number : %d\n", inode_cnt);
	sb.ninode--;
	if(sb.ninode < 1)
	{
		exit(0);
	}
	printf("now value of ninode is : %d\n", sb.ninode);
	sb.inode_cnt++;
	printf("now value of inode_cnt is : %d\n", sb.ninode);
	fd1 = open(my_argv[1], O_RDWR, 0666);

	   size = lseek(fd1, 0, SEEK_END);
	   size1 = lseek(fd1, 0, SEEK_SET);

	/*extfile_size = info.st_size;
	printf("\nExternal file size is %lld \n", (long long) extfile_size);
	printf("\nBlocks allocated is %lld\n", (long long) info.st_blocks);
	*/

	extfile_size = size - size1 ;

	printf("file size(in bytes) is %lld\n", extfile_size);

	if((extfile_size % 2048) == 0){
	blocks_required = (extfile_size)/2048 ;
	//printf("in if \n");
	printf("blocks_required : %lld \n", (long long) blocks_required);
	}
	else{
	blocks_required = (extfile_size/2048) + 1 ;
	//printf("in else \n");
	printf("blocks_required : %lld \n", (long long) blocks_required);
	}
	//}
	int num_blocks = blocks_required;
	printf("nfree is %d \n", sb.nfree);


	if(blocks_required < sb.freeblks){

	if(blocks_required < sb.nfree )
	{
		sb.freeblks = sb.freeblks - blocks_required;
		sb.nfree = sb.nfree - blocks_required ;	//Blocks which will be available in superblock after allocating to this file
		printf("Allocated blocks = %lld\n", blocks_required);
		printf("Remaining free blocks in superblock : %d\n", sb.nfree);

	}
	else
	{
		if(sb.nfree != 0 && blocks_required < sb.freeblks){
			arrays_required = blocks_required / sb.nfree ;
			arrays_count = arrays_required;
			int value = sb.ninode+sb.inode_cnt+100+2;
			while(arrays_required != 0){
						//temp = 300+2+100 = 402
					for(f_loop1 = 0; f_loop1 < 100; f_loop1++)	//Loop of 100
					{
						sb.free[f_loop1] = value;	//free blocks array in super block = temp (402 to 501)
						value++;
						sb.nfree--;
						}
					//for 1000 blocks or 10 arrays
					//temp = 500
					arrays_required--;
					}
					sb.freeblks = sb.freeblks - (arrays_count*100);	//suppose array_cnt = 5 which means 5 arrays allocated

					//so number of f blocks will be reduced by 500 blocks

					printf("Array allocated are : %d \n", arrays_count);
					}
				}


			lseek(fd,(inode_cnt+1)*BLOCK_SIZE,SEEK_SET);	//Go to block to write inode struct

			struct inode node[sb.inode_cnt+1];	// inode starts from 1.

			/*populating the inode 1 for root directory*/
			node[sb.inode_cnt].flags = (FLAG_INODE_ALLOC|FLAG_INODE_DIRECTORY|FLAG_INODE_UID|FLAG_INODE_OWNR_ACCESS|
									FLAG_INODE_GRP_ACCESS|FLAG_INODE_OTHR_ACCESS);	//set inode flags
			printf("inode flags = %d \n",node[sb.inode_cnt].flags);
			node[sb.inode_cnt].uid = 0;	//set uid for inode
			node[sb.inode_cnt].gid = 0; 	//set gid for inode
			node[sb.inode_cnt].size0 = 2;	//high word of 32-bit size
			node[sb.inode_cnt].size1 = 2;	//low word of 32-bit size

			stat(my_argv[1], &attrib);
			strcpy(node[sb.inode_cnt].acttime,ctime(&attrib.st_atime));	//time of last access
			strcpy(node[sb.inode_cnt].modtime,ctime(&attrib.st_mtime));	//time of last modification
			printf("acttime = %s \n",node[sb.inode_cnt].acttime);
			printf("modtime = %s \n",node[sb.inode_cnt].modtime);

			node[sb.inode_cnt].addr[0] = 100;	//write 100 in first 2 bytes of block of inode indicating number of free blocks

			for(f_loop = 0; f_loop < 25; f_loop++)	//initialize addrarray of size 26
			{
				node[sb.inode_cnt].addr[f_loop] = 0;
			}


			bytes_written = write(fd,&node[sb.inode_cnt+1],sizeof(node[sb.inode_cnt+1]));	//rest modifiation
			printf("bytes_written for inode struct = %d \n",bytes_written);

			lseek(fd,BLOCK_SIZE*1,SEEK_SET); 	//super block
			bytes_written = write(fd,&sb,BLOCK_SIZE);
			printf("bytes_written for super block after cpin = %d \n",bytes_written);
		}

		else{
		printf("There are no free blocks! Initialize the system again");
		}

		get_filename();
		printf("ret1 is %s \n", ret1);
		entry1.i_node = sb.inode_cnt;
		strcpy(entry1.file_name,ret1);
		lseek(fd,(((sb.ninode+sb.inode_cnt)*BLOCK_SIZE)+(16*sb.dir_ent)),SEEK_SET);
		bytes_written = write(fd,&entry1,sizeof(entry1));
		printf("entry1 bytes written = %d \n",bytes_written);
		sb.dir_ent++;

		lseek(fd,BLOCK_SIZE*1,SEEK_SET);	//skip first block
		bytes_written = write(fd,&sb,sizeof(sb));
		printf("sb bytes written = %d \n",bytes_written);

		int j;
		char buffer[2048];
		int total_cnt = sb.ninode + sb.inode_cnt+100+1;
		printf("**** %lld*********",blocks_required);
		//Now I have blocks_required and arrays_required data so accordingly will copy blocks into fs.txt
		for(j = 0; j < num_blocks; j++){
		//read from external file
		lseek(fd1, 0, SEEK_CUR);
		read(fd1, buffer, sizeof(buffer));
		printf("*************");
		//write to V6 file
		lseek(fd,(total_cnt)*BLOCK_SIZE,SEEK_SET);	//300+100+1 = 401
		temp = write(fd, buffer, sizeof(buffer));
		printf("temp value : %d\n", temp);
		printf("Block %d written \n", total_cnt);
		sb.nfree--;
		total_cnt--;

	}


	close(fd);
	printf("\nContent Copied\n");
	execv("/bin/cp", cpargs);
}

/* CPOUT Command 3 */
void cp_out(){
	//If the V6 file exits, create external file and copy contents to it.

	  printf("path for V6 file to be opened is %s\n", my_argv[1]);			//V6 file

	  printf("path for external file to be created is %s\n", my_argv[2]);	//External file

		if (access(my_argv[1],W_OK) == 0)	//If path is already there then just open fd
		{
			printf("\nPath exists \n");
			//fopen(my_argv[1],"w");
			fd = open(my_argv[1],O_RDONLY, 0777);
			printf("\n fd %d opened \n", fd);

			fd = open(my_argv[2],O_CREAT, 0666);
			char *cpargs[] = {"cp", my_argv[1], my_argv[2], (char *) 0 };

			printf("\nContent Copied\n");
			execv("/bin/cp", cpargs);

		}
		else  								//Else do nothing
		{
			printf("\nPath doesnt exist, try again! \n");

		}

}

/* mkdir Command 4 */
void mk_dir()
{
	printf("Path for directory to be created : %s", my_argv[1]);

	if (access(my_argv[1],R_OK) == 0)	//If path is already there do nothing
	{
		printf("\nPath exists \n");
		//fopen(my_argv[1],"w");
		fd = open(my_argv[1],O_RDONLY, 0777);
		printf("\n fd %d opened \n", fd);

	}

	else  								//Else create new directory
	{
		printf("\nPath doesnt exist, Creating directory... \n");
		// write inode structure

		strcpy(sb.filesys_path, "/home/004/v/vm/vmk13030/fs.txt");
		fd = open(sb.filesys_path,O_RDWR, 0666);
		lseek(fd,BLOCK_SIZE,SEEK_SET);
		temp = read(fd,&sb,BLOCK_SIZE);


		printf("bytes of super block read for cpin = %d", temp);
		printf("nfree = %d ninode = %d \n",sb.nfree,sb.ninode);
		sb.ninode--;
		sb.inode_cnt++;
		if(sb.ninode > 1)
		{
			lseek(fd,(sb.inode_cnt+1)*BLOCK_SIZE,SEEK_SET);	//Go to block to write inode struct

			struct inode node;	// inode starts from 1.

			/*populating the inode 1 for root directory*/
			node.flags = (FLAG_INODE_ALLOC|FLAG_INODE_DIRECTORY|FLAG_INODE_UID|FLAG_INODE_OWNR_ACCESS|
									FLAG_INODE_GRP_ACCESS|FLAG_INODE_OTHR_ACCESS);	//set inode flags
			printf("inode flags = %d \n",node.flags);
			node.uid = 0;	//set uid for inode
			node.gid = 0; 	//set gid for inode
			node.size0 = 2;	//high word of 32-bit size
			node.size1 = 2;	//low word of 32-bit size

			stat(my_argv[1], &attrib);
			strcpy(node.acttime,ctime(&attrib.st_atime));	//time of last access
			strcpy(node.modtime,ctime(&attrib.st_mtime));	//time of last modification
			printf("acttime = %s \n",node.acttime);
			printf("modtime = %s \n",node.modtime);

			for(f_loop = 0; f_loop < 25; f_loop++)	//initialize addrarray of size 26
			{
				node.addr[f_loop] = 0;
			}

			lseek(fd,(sb.inode_cnt+1)*BLOCK_SIZE,SEEK_SET);

			bytes_written = write(fd,&node,sizeof(node));	//rest modifiation
			printf("bytes_written for inode struct = %d \n",bytes_written);

			lseek(fd,BLOCK_SIZE,SEEK_SET);

			bytes_written = write(fd,&sb,sizeof(sb));	//rest modifiation
			printf("bytes_written for sb struct = %d \n",bytes_written);

			char *mkargs[] = {"mkdir", my_argv[1], (char *) 0 };
			printf("\nDirectory Created with inode number %d\n", sb.inode_cnt);

			strcpy(newfile_path,my_argv[1]);
			get_filename();
			printf("ret1 is %s \n", ret1);

			entry1.i_node = sb.inode_cnt;
			strcpy(entry1.file_name,  ret1);
			lseek(fd,(((sb.ninode+sb.inode_cnt)*BLOCK_SIZE)+(16*sb.dir_ent)),SEEK_SET);
			bytes_written = write(fd,&entry1,sizeof(entry1));
			printf("entry1 bytes written = %d \n",bytes_written);
			sb.dir_ent++;

			lseek(fd,BLOCK_SIZE*1,SEEK_SET);	//skip first block
			bytes_written = write(fd,&sb,sizeof(sb));
			printf("sb bytes written = %d \n",bytes_written);

			close(fd);

			execv("/bin/mkdir", mkargs);
		}
		else
		{
			printf("No free inodes available");
		}
	}
}

void free_argv()
{
	int index;
	for(index=0;my_argv[index]!=NULL;index++) {
		bzero(my_argv[index], strlen(my_argv[index])+1);
		my_argv[index] = NULL;
		free(my_argv[index]);
	}
}

int main(int argc, char *argv[], char *envp[])
{
	char c;
	int i = 0, fd;

	char *tmp = (char *)malloc(sizeof(char) * 200);

	char *cmd = (char *)malloc(sizeof(char) * 200);


/*.....................Signal Handler...........................*/
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);


	copy_envp(envp);
	//get_path_string(my_envp, path_str);
	//insert_path_str_to_search(path_str);

	if(fork() == 0) {
		execve("/usr/bin/clear", argv, my_envp);
		printf("\n Clearing screen \n");
		exit(1);
	} else {
		wait(NULL);
	}

	printf("[Modified_V6 ] ");
	fflush(stdout);
	while(c != EOF) {
		c = getchar();
		switch(c) {
			case '\n': if(tmp[0] == '\0') {
					   printf("[Modified_V6 ] ");
					   i = 0;
				   } else {
					   i = 0;
					   fill_argv(tmp);
					   strncpy(cmd, my_argv[0], strlen(my_argv[0]));
					   strncat(cmd, "\0", 1);
					   if(index(cmd, '/') == NULL) {

						while(my_argv[i] != NULL){
						   printf("my_arg[%d] is : %s \n", i, my_argv[i]);
						   i++;
						   }
						   i = 0;
							if(strcmp(my_argv[0], "initfs") == 0)
								{
								printf("\ntesting \n");
								//cpin rg1
								init_fs();
								printf("initfs complete \n");

								}

							else if(strcmp(my_argv[0], "cpin") == 0){

								printf("\nCommand 2 detected\n"); // cpin externalfile v6file
								cp_in();
								//copy pathofexternalfile pathofv6file
								}
							else if(strcmp(my_argv[0], "cpout") == 0){
								printf("\nCommand 3 detected\n");
								cp_out();
								}

							else if(strcmp(my_argv[0], "makedir") == 0){
								printf("\nCommand 4 detected\n");
								mk_dir();
								}

							else if(strcmp(my_argv[0], "q") == 0){

								printf("\nCommand 5 detected\n");
								/* quit Command 5 */
								exit(1);
								}

					   } else {

						   printf("%s: command not found\n", cmd);
					     }
					   free_argv();

					   printf("[Modified_V6 ] ");
					   bzero(cmd, 100);
				   }
				   bzero(tmp, 100);
				   break;
			default:
				 strncat(tmp, &c, 1);
				 //printf("\n inside default case \n");
				 break;
	}

}

	free(tmp);
	printf("\n");

	return 0;
}



int freeblock_alloc(int freeblk_req)
{
	int i,j,free_blks_needed, unalloc_free_blks;
	int fd,temp, block_diff,bytes_written;
	free_blks_needed = freeblk_req;
	unalloc_free_blks = sb.nfree;
	block_diff = unalloc_free_blks - free_blks_needed;
	fd = open(my_argv[1],O_RDWR|O_CREAT|O_TRUNC, 0666);
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	read(fd,&sb,BLOCK_SIZE);
	j = sb.ninode;
	if((j >=2 && j <= sb.ninode )|| free_blks_needed <= sb.freeblks)
	{
		sb.freeblks = sb.freeblks - free_blks_needed;
		while(free_blks_needed > 0)
		{
			if((sb.nfree - 1) > 0)
			{
				sb.free[sb.nfree] = 0; // remove the free block entry
				sb.nfree--;
				free_blks_needed--;
			}
			else
			{
				lseek(fd,(BLOCK_SIZE*sb.free[sb.nfree-1]),SEEK_SET);
				temp = read(fd, &sb.nfree,2);
				printf("no of bytes read = %d \n",temp);
				lseek(fd,((BLOCK_SIZE*sb.free[sb.nfree-1])+2),SEEK_SET);
				temp = read(fd,free_blk_cnt,200);
				printf("no of bytes read = %d \n",temp);
				for(f_loop = 0; f_loop < sb.nfree; f_loop++)
				{
					sb.free[f_loop] = free_blk_cnt[f_loop];
				}
			}
		}
		sb.ninode = sb.ninode - 1;
		lseek(fd,BLOCK_SIZE*1,SEEK_SET);
		bytes_written = write(fd,&sb,sizeof(sb));
		printf("sb bytes written = %d \n",bytes_written);
		close (fd);
		return 1;
	}
	else
	{
		printf("Required inodes/blocks unavailable, no blocks allocated \n");
		close (fd);
		return 0;
	}
}
