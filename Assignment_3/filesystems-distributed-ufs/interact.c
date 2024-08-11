#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define UFS_BLOCK_SIZE (4096)
int main(int argc, char *argv[]) {
	printf("Usage: %s <image_file>\n", argv[0]);
	char *image_file = argv[1];
	int fd = open(image_file, O_RDONLY);
	off_t new_pos = lseek(fd, 4096, SEEK_SET);
  	if (new_pos == -1) {
    	perror("lseek");
    	close(fd);
    	return 1;
  	}
	typedef struct {
		unsigned int bits[UFS_BLOCK_SIZE / sizeof(unsigned int)];
    	} bitmap_t;
	bitmap_t b;
  	int n = read(fd, &b, sizeof(b));
	printf("%b", b.bits[0]);
  	printf("\n");
	close(fd);
	return 0;


}
