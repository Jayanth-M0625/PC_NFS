#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "ufs.h"


int main(int argc, char *argv[]) {
	if (argc != 2) {
	    fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
	    exit(1);
 	 }
	char *image_file = argv[1];
	int fd = open(image_file, O_RDONLY);
	if (fd <0) {
		perror("open");
		exit(1);
	}

	super_t s;
	ssize_t bytes_read = read(fd, &s, sizeof(super_t));
	if (bytes_read != sizeof(super_t)) {
		perror("read");
		close(fd);
		exit(1);
	}
	printf("===========================================\n");
  	printf("Superblock contents:\n");
  	printf("  num_inodes:     %d\n", s.num_inodes);
  	printf("  num_data:       %d\n", s.num_data);
  	printf("  inode_bitmap_addr: %d\n", s.inode_bitmap_addr);
  	printf("  inode_bitmap_len:  %d\n", s.inode_bitmap_len);
  	printf("  data_bitmap_addr:  %d\n", s.data_bitmap_addr);
  	printf("  data_bitmap_len:   %d\n", s.data_bitmap_len);
  	printf("  inode_region_addr: %d\n", s.inode_region_addr);
  	printf("  inode_region_len:  %d\n", s.inode_region_len);
  	printf("  data_region_addr:  %d\n", s.data_region_addr);
  	printf("  data_region_len:   %d\n", s.data_region_len);

  	close(fd);
  	
	printf("===========================================\n");
	return 0;
}


