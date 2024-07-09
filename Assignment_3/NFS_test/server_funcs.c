#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ufs.h"

ufs UFS;

int UFS_Init(char *image_file){
    
    int fd = open(image_file, O_RDONLY);
	if (fd <0) {
		perror("open");
		exit(1);
	}
    int n = read(fd, &UFS.s, 40);
    off_t new_pos = lseek(fd, 4096, SEEK_SET);
    int m = read(fd, &UFS.inode_btmp, 4096);
    int o = read(fd, &UFS.data_btmp, 4096);
    int p = read(fd, &UFS.inodes, 4096);
    for (int i = 0; i<32; i++){
        int q = read(fd, &UFS.dir_blocks.dir_block[i], 4096);
    }
    
    printf("Data is being read into disk structure UFS...\n");
    printf("inode btmp: %b\n", UFS.inode_btmp.bits[0]);
    printf("data btmp: %b\n", UFS.data_btmp.bits[0]);
    printf("inode 0 contains (0=dir, 1=file): %d\n", UFS.inodes.inode[0].type);
    printf("inode 0 size : %d\n", UFS.inodes.inode[0].size);//64, size of inode(wt is stored inside) = 2*sizeof(dir_ent_t) coz it contains 2 dir entries
    printf("Size of one inode: %ld + %ld + 30*%ld\n", sizeof(UFS.inodes.inode[0].type), sizeof(UFS.inodes.inode[0].size), sizeof(UFS.inodes.inode[0].direct[0]));
    printf("inode 0 DIRECT_PTRS = 0: %d\n", UFS.inodes.inode[0].direct[0]);//4
    printf("inode 0 DIRECT_PTRS = 1: %d\n", UFS.inodes.inode[0].direct[1]);//-1
    printf("dir_block[0], entry 0 name: %s\n", UFS.dir_blocks.dir_block[0].entries[0].name);//.
    printf("dir_block[0], entry 1 name: %s\n", UFS.dir_blocks.dir_block[0].entries[1].name);//..
    printf("dir_block[31]: %b\n", UFS.dir_blocks.dir_block[31]);
    printf("dir_block[31], entry 0 name: %s\n", UFS.dir_blocks.dir_block[31].entries[0].name);//
    printf("there are 30 DIRECT_PTRS in total (-1 for unused)\n");
    printf("Successfully initialised UFS...\n");
    return 0;
}

int UFS_Lookup(int pinum, char *name){
    int numOf_data_blocks = UFS.inodes.inode[pinum].size/4096;
    numOf_data_blocks += 1;
    int data_block_Addrs[numOf_data_blocks];
    for(int i=0; i<numOf_data_blocks;i++){
        data_block_Addrs[i] = UFS.inodes.inode[pinum].direct[i];
    }
    for(int j=0;j<numOf_data_blocks;j++){
        int n = data_block_Addrs[j];
        int numOfEntries;
        for(int i=0;i<128;i++){
            if(UFS.dir_blocks.dir_block[n].entries[i].inum == -1){
                numOfEntries = i;
            }
            else numOfEntries=128;
        }
        
        for(int i=0; i<numOfEntries;i++){
        if(name == UFS.dir_blocks.dir_block[n].entries[i].name){
            return UFS.dir_blocks.dir_block[n].entries[i].inum;
        }
        else {
            return -1;
        }
    }
    }
    
}

int UFS_Write(int inum, char *buffer, int offset, int nbytes){
    if(nbytes>4096){printf("Error: invalid nbytes(>4096)\n"); return -1;}
    if(offset>4096){printf("Error: invalid offset(>4096)\n"); return -1;}
    if(inum>32){printf("Error: invalid inum(>32)"); return -1;}
    if(UFS.inodes.inode[inum].type == 0){printf("Error: invalid inum(Not a file)"); return -1;}
    if(offset+nbytes>4096){printf("Error: data overflowing current block"); return -1;}
    // int n = write();//
    //if it exceeds data limit, can u write into another block? no right? SO no need to update inode bitmap
    return 0;
}

int UFS_Read(int inum, char *buffer, int offset, int nbytes){
    if(nbytes>4096){printf("Error: invalid nbytes(>4096)\n"); return -1;}
    if(offset>4096){printf("Error: invalid offset(>4096)\n"); return -1;}
    if(inum>32){printf("Error: invalid inum(>32)"); return -1;}
    switch(UFS.inodes.inode[inum].type){
        case 0://dir
            for (int i=0;i<30;i++){
                if(UFS.inodes.inode[inum].direct[i] == -1){break;}
                else {
                    int addr_idx = UFS.inodes.inode[inum].direct[i]-4/*addr will be from 0 but idx should be from 4*/;
                    for(int j=0;j<128;j++){
                        if(UFS.dir_blocks.dir_block[addr_idx].entries[j].inum == -1){
                            break;
                        }
                        else {
                            printf("Entry: %d Name: %s inum: %d\n", j, UFS.dir_blocks.dir_block[i].entries[j].name, UFS.dir_blocks.dir_block[i].entries[j].inum);
                        }
                    }
                }
            }
        case 1://file
        for (int i=0;i<30;i++){ 
            if(UFS.inodes.inode[inum].direct[i] == -1){break;}
            else {
                int addr_idx = UFS.inodes.inode[inum].direct[i]-4;
                //128 entries are for dirs. For a file, there are no entries. So just read whole data of this block depending on size. 
                //IMPLEMENT READ FOR size BYTES.
            }
        }
    }
}

int find_mt_iORd(bitmap_t btmp) {//inode_btmp = UFS.inode_btmp
  for (int i = 0; i < 32; i++) {
    if ((btmp.bits[0] & (1 << i)) == 0) {
      return i; // MT num is i
    }
  }
  return -1; // No MT space
}

int UFS_Creat(int pinum, int type, char *name){
    //Failure Modes:
    if(UFS.inode_btmp.bits[0] & (1 << pinum) == 0){printf("Error: pinum doesn't exist");}
    if(sizeof(name)>28){printf("Error: Name is too long");}
    //first find available inum and data blocks.
    int avail_inode = find_mt_iORd(UFS.inode_btmp);
    if(avail_inode == -1){printf("Error: All inodes occupied"); return -1;}
    int avail_data = find_mt_iORd(UFS.data_btmp);
    if(avail_data == -1){printf("Error: All data blocks occupied"); return -1;}
    //Update inodes
    UFS.inode_btmp.bits[0] |= (1 << avail_inode);
    UFS.data_btmp.bits[0] |= (1 << avail_data);
    //Make dir/file
    switch (type){
        case 0://dir
            UFS.inodes.inode[avail_inode].type = 0;
            UFS.inodes.inode[avail_inode].size = sizeof(dir_ent_t);
            UFS.inodes.inode[avail_inode].direct[0] = avail_data + 4;//data addr + free block addr
        case 1://file
            
    }

    
}

int UFS_Unlink(int pinum, char *name){}

int UFS_Shutdown(){}

// int UFS_Stat(int inum, UFS_Stat_t *m){}

int main(int argc, char *argv[]){
    printf("super_t: %ld, bitpam_t: %ld, Inodes: %ld, inode_t: %ld\n", sizeof(super_t), sizeof(bitmap_t), sizeof(Inodes), sizeof(inode_t));
    if (argc != 2) {
	    fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
	    exit(1);
 	 }
	UFS_Init(argv[1]);
	printf("UFS_Init done\n");
    return 0;
}
