#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define NSL_DEV_NAME "nsl"
#define NSL_MAJOR 261
#define NSL_GET_INDEX _IO(NSL_MAJOR, 0)
#define NSL_GET_TABLE _IOWR(NSL_MAJOR, 1, void *)

int main(void)
{
	int fd, ret;

	fd = open("/dev/"NSL_DEV_NAME, 0);
	if(fd < 0) {
		fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
		return -1;
	}

	ret = ioctl(fd, NSL_GET_INDEX);
	printf("NSL_GET_INDEX: %d\n", ret);
	return 0;
}
