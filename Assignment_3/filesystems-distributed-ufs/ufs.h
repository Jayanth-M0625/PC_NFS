#ifndef __ufs_h__
#define __ufs_h__

#define UFS_DIRECTORY (0)
#define UFS_REGULAR_FILE (1)

#define UFS_BLOCK_SIZE (4096)

#define DIRECT_PTRS (30)

typedef struct {
    int type;   // MFS_DIRECTORY(0) or MFS_REGULAR(1)
    int size;   // bytes
    unsigned int direct[DIRECT_PTRS]; 
} inode_t;

typedef struct {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} dir_ent_t;

// presumed: block 0 is the super block
typedef struct __super {
    int inode_bitmap_addr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    int data_bitmap_addr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    int inode_region_addr; // block address (in blocks)
    int inode_region_len;  // in blocks
    int data_region_addr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int num_inodes;        // just the number of inodes
    int num_data;          // and data blocks...
} super_t;

//Manually defined:

typedef struct {
        //unsigned int bits[UFS_BLOCK_SIZE / sizeof(unsigned int/*4bytes => 1024bytes*/)]; but 1 unsigned int ka space in enof to store status of 32 blocks. So
        //unsigned int bits[1];//but even for this syntax is the same so let's just copy everything directly.
	unsigned int bits[UFS_BLOCK_SIZE / sizeof(unsigned int)];
} bitmap_t;

typedef struct {//An array to access each inode easily. Store blk 3 in this. Called inode table too
	inode_t inode[32];
} Inodes;

typedef struct {
	dir_ent_t entries[128];
    } dir_block_t;

typedef struct {
    dir_block_t dir_block[32];
} Data_Blocks;

typedef struct __ufs {
	super_t s;
	bitmap_t inode_btmp;//read everything but bits[0] is enof for all func.
	bitmap_t data_btmp;//bits[0] to write
	Inodes inodes;//read 4th block(3). inodes.inode[0] for first one
    Data_Blocks dir_blocks;//usage: for both this and inode blocks, to access the first one use dir_blocks.dir_block[0] and dir_blocks.dir_block[31] for the last one
    char image_filepath[4096+1];//store image file path.
    int fd; // File descriptor for image file (initialized to -1)
} ufs;

typedef struct __UFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} UFS_Stat_t;


#endif // __ufs_h__
