#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ufs.h"

ufs UFS;

int check_inum(int inum, bitmap_t i_btmp){
    
        if ((i_btmp.bits[0] & (1 << inum)) == 0) {
        return 0; //inum doesn't exist
        }
    
    return 1; //inum exists
}


int UFS_Init(char *image_file){
    strncpy(UFS.image_filepath, image_file, sizeof(UFS.image_filepath));
    UFS.fd = -1;
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
    //invaid pinum, name does not exist
    if(pinum > 32 | (check_inum(pinum, UFS.inode_btmp) == 0) ){printf("Error: invalid pinum"); return -1;}
    for(int i=0; i< DIRECT_PTRS; i++){
        if(UFS.inodes.inode[pinum].direct[i] != -1){
            int data_idx = UFS.inodes.inode[pinum].direct[i] - 4;
            for(int j=0; j<128;j++){
                if(UFS.dir_blocks.dir_block[data_idx].entries[j].name == name){
                    return UFS.dir_blocks.dir_block[data_idx].entries[j].inum;
                }
                else {continue;}
            }
        }
        else{continue;}
    }
    printf("Error: name does not exist");
    return -1;    
}

int UFS_Write(int inum, char *buffer, int offset, int nbytes){//y access only direct[0]? input of func. doesn't give any info abt which block so by default - assume from start
    if(nbytes>4096){printf("Error: invalid nbytes(>4096)\n"); return -1;}
    if(offset>4096){printf("Error: invalid offset(>4096)\n"); return -1;}
    if(inum>32 || check_inum(inum, UFS.inode_btmp)){printf("Error: invalid inum(>32)"); return -1;}//check_inum must return 1 if inum exists
    if(UFS.inodes.inode[inum].type == 0){printf("Error: invalid inum(Not a file)"); return -1;}
    if(offset+nbytes>4096){printf("Error: data overflowing current block"); return -1;}
    // int n = write();//need to open to file first using inum and then use lseek
    if (UFS.fd == -1){
        UFS.fd = open(UFS.image_filepath, O_RDWR);
        if (UFS.fd == -1){
            perror("open");
            exit(1);
        }
    }
    //use inum to locate file and set cursor to file start
    int file_data_addr = UFS.inodes.inode[inum].direct[0];
    //cursor set to file start
    if (lseek(UFS.fd, offset + 4096*file_data_addr, SEEK_SET) == -1) {//instead, now it does both steps at once (hopefully - need to test)
        printf("Error: lseek failure");
        close(UFS.fd);
        return -1; // Handle lseek failure
    }
    ssize_t bytes_written = write(UFS.fd, buffer, nbytes);
    if (bytes_written == -1) {
        printf("Error: Write failure");
        close(UFS.fd);
        return -1; // Handle write failure
    }

    // Check if the entire buffer was written (considering partial writes)
    if (bytes_written != nbytes) {
        printf("Error: Partial write");
        close(UFS.fd);
        return -1; // Handle partial write or other write errors
    }

    close(UFS.fd);
    //if it exceeds data limit, can u write into another block? no right? SO no need to update inode bitmap. But need to update size of the file.
    if(offset+nbytes>UFS.inodes.inode[inum].size){UFS.inodes.inode[inum].size = offset + nbytes;}//update file size
    UFS.fd = open(UFS.image_filepath, O_RDWR);//write everything to disk file
    int n = write(UFS.fd,&UFS,sizeof(UFS));
    close(UFS.fd);
    return 0;
}

int UFS_Read(int inum, char *buffer, int offset, int nbytes){//Same code for file n dir right? ig..
    if(nbytes>4096){printf("Error: invalid nbytes(>4096)\n"); return -1;}
    if(offset>4096){printf("Error: invalid offset(>4096)\n"); return -1;}
    if(inum>32){printf("Error: invalid inum(>32)"); return -1;}
    //case 1: offset + nbytes < 4096 so read within one block
    //case 2: offset + nbytes >4096 && offset < 4096 then should we read half from one block and half from other block
    //case 3: offset = n*4096 then should we read nbytes from the nth block? so direct[n]?
    //case 4: I'm overthinking. | If this was the case, invalid offset would have no meaning. Offset>4096 could be assumed as nth block and all values would be valid.
    
    UFS.fd = open(UFS.image_filepath, O_RDWR);
    
    int file_data_addr = UFS.inodes.inode[inum].direct[0];
    lseek(UFS.fd, offset + 4096*file_data_addr, SEEK_SET);
    ssize_t bytes_read = read(UFS.fd, &buffer, nbytes);
    close(UFS.fd);
    return 0;
    
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
    //check if name already exists:
    int addr = UFS.inodes.inode[pinum].direct[0];
    for (int i=0;i<128;i++){
        if(UFS.dir_blocks.dir_block[addr-4].entries[i].name == name){printf("Name already exists");return 0;}
        else {continue;}
        }
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
            UFS.inodes.inode[avail_inode].size = 2 * sizeof(dir_ent_t);
            UFS.inodes.inode[avail_inode].direct[0] = avail_data + 4;//data addr + free block addr
            for (int i = 1; i < DIRECT_PTRS; i++){//set data ptrs to -1
	        UFS.inodes.inode[avail_inode].direct[i] = -1;}
            strcpy(UFS.dir_blocks.dir_block[avail_data].entries[0].name, ".");//. entry
            UFS.dir_blocks.dir_block[avail_data].entries[0].inum = avail_inode;
            strcpy(UFS.dir_blocks.dir_block[avail_data].entries[1].name, "..");// .. entry
            UFS.dir_blocks.dir_block[avail_data].entries[1].inum = pinum;
            for (int i = 2; i < 128; i++){//set other entries to -1
	        UFS.dir_blocks.dir_block[avail_data].entries[i].inum = -1;}
            UFS.inodes.inode[pinum].size += sizeof(dir_ent_t);//increase size of parent dir. d_dir got 2 entries but pdir got only 1 extra entry
            UFS.fd = open(UFS.image_filepath, O_RDWR);
            int n = write(UFS.fd, &UFS, sizeof(UFS));
            close(UFS.fd);
            return 0;
        case 1://file
            UFS.inodes.inode[avail_inode].type = 1;
            UFS.inodes.inode[avail_inode].size = 0;
            UFS.inodes.inode[avail_inode].direct[0] = avail_data + 4;//data addr + free block addr
            for (int i = 1; i < DIRECT_PTRS; i++){//set data ptrs to -1
	        UFS.inodes.inode[avail_inode].direct[i] = -1;}
            UFS.fd = open(UFS.image_filepath, O_RDWR);
            int n = write(UFS.fd, &UFS, sizeof(UFS));
            close(UFS.fd);
            return 0;
    }

    
}

int UFS_Unlink(int pinum, char *name){
    //failure modes: pinum does not exist, dir not empty
    //needs to unlink dir/file from parent. 
    if(check_inum(pinum, UFS.inode_btmp) == 0){printf("Error: pinum does not exist"); return -1;}
    for(int i=0; i<DIRECT_PTRS;i++){
        if(UFS.inodes.inode[pinum].direct[i] == -1){continue;}
        int pdir_idx = UFS.inodes.inode[pinum].direct[i] - 4;
        for(int j=0; j<128; j++){
            if(UFS.dir_blocks.dir_block[pdir_idx].entries[j].inum != -1){//some entries might be empty that have been deleted. So checks all from 1st entry
                
                if(UFS.dir_blocks.dir_block[pdir_idx].entries[j].name == name){
                    int d_inum = UFS.dir_blocks.dir_block[pdir_idx].entries[j].inum;//d_inum = daughter inum
                    //irrespective of file/dir, de-allocate inum, data blocks, pdir entry are common. But I have to check if dir is empty so can't keep that code here
                    switch(UFS.inodes.inode[d_inum].type){
                        
                        case 0://dir
                        //is dir empty?
                        if(UFS.inodes.inode[d_inum].size == 2*sizeof(dir_ent_t)){//Yes, unlink. How? deallocate inum, data block of this dir, set this entry to -1, reduce size of pdir
                            UFS.dir_blocks.dir_block[pdir_idx].entries[j].inum = -1;//set entry to -1
                            UFS.inode_btmp.bits[0] = 0x0 << (32-d_inum);//de-allocate inum
                            for(int i=0 ; i<DIRECT_PTRS;i++){//de-allocate all data
                                if(UFS.inodes.inode[d_inum].direct[i] != -1){
                                    UFS.inodes.inode[d_inum].direct[i] = -1;
                                    UFS.data_btmp.bits[0] = 0x0 << (32-i);
                                }
                                else {continue;}
                            }
                            UFS.inodes.inode[pinum].size -= sizeof(dir_ent_t);//reduce size of pdir
                            return 0;
                        }
                        else{printf("Error: directory is not empty"); return -1;}
                        
                        case 1://file
                        //de-allocate inum, data blocks, calc file size and reduce accordingly
                        UFS.dir_blocks.dir_block[pdir_idx].entries[j].inum = -1;//set entry to -1
                        UFS.inode_btmp.bits[0] = 0x0 << (32-d_inum);//de-allocate inum
                        for(int i=0 ; i<DIRECT_PTRS;i++){//de-allocate all data
                            if(UFS.inodes.inode[d_inum].direct[i] != -1){
                                UFS.inodes.inode[d_inum].direct[i] = -1;
                                UFS.data_btmp.bits[0] = 0x0 << (32-i);
                            }
                            else {continue;}
                        }
                        UFS.inodes.inode[pinum].size -= UFS.inodes.inode[d_inum].size;//not cleaning data, but allowing it to be overwritten.
                        return 0;
                    }
                }
                else {continue;}
            }
            else{continue;}
        }
    }
}

int UFS_Shutdown(){
    UFS.fd = open(UFS.image_filepath, O_RDWR);
    int n = write(UFS.fd, &UFS, sizeof(UFS));
    close(UFS.fd);
    exit(0);
}

int UFS_Stat(int inum, UFS_Stat_t *m){
    m->size = UFS.inodes.inode[inum].size;
    m->type = UFS.inodes.inode[inum].type;
    return m;
}//fill-----------------------------------------------------------------------------------------------

// int main(int argc, char *argv[]){
//     printf("super_t: %ld, bitpam_t: %ld, Inodes: %ld, inode_t: %ld\n", sizeof(super_t), sizeof(bitmap_t), sizeof(Inodes), sizeof(inode_t));
//     if (argc != 2) {
// 	    fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
// 	    exit(1);
//     }
// 	UFS_Init(argv[1]);
// 	printf("UFS_Init done\n");
//     return 0;
// }
