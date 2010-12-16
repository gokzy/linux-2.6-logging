#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define NSL_DEV_NAME "nsl"
#define NSL_MAJOR 261
#define NSL_GET_INDEX _IO(NSL_MAJOR, 0)
#define NSL_GET_TABLE _IOWR(NSL_MAJOR, 1, void *)

struct net_stack_log{
	uint32_t func;
	uint32_t cpu;
	uint16_t eth_protocol;
	uint8_t  ip_protocol;
	uint32_t ip_saddr;
	uint32_t ip_daddr;
	uint16_t tp_sport;
	uint16_t tp_dport;
	uint64_t time;
};

void usage(void)
{
	printf("usage: nsl [start|get|stop]\n");
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return -1;
	}

	if (!strcmp(argv[1], "start")) {
		fprintf(stderr, "start not implemented yet\n");
		return -1;
	}else if (!strcmp(argv[1], "stop")) {
		fprintf(stderr, "stop not implemented yet\n");
		return -1;
	}else if (!strcmp(argv[1], "get")) {
		int fd, index, i;
		struct net_stack_log *log;

		fd = open("/dev/"NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
			return -1;
		}

		index = ioctl(fd, NSL_GET_INDEX);
		printf("NSL_GET_INDEX: %d\n", index);

		log = (struct net_stack_log *)malloc(
			index * sizeof(struct net_stack_log));
		if(!log) {
			fprintf(stderr, "malloc failed\n");
			return -1;
		}
		*(int *)log = index;
		index = ioctl(fd, NSL_GET_TABLE, (void *)log);
		printf("NSL_GET_TABLE: %d\n", index);
		
		for (i = 0; i < index; i++) {
			printf("[%d] func:%d cpu:%d eth_protocol:%d ip_protocol:%d ip_saddr:%x ip_daddr:%x tp_sport:%d tp_dport:%d time:%x\n",
			       i, log[i].func, log[i].cpu, log[i].eth_protocol, log[i].ip_protocol, log[i].ip_saddr, log[i].ip_daddr, log[i].tp_sport, log[i].tp_dport, log[i].time);
		}

		free(log);
		close(fd);

		return 0;
	}else{
		fprintf(stderr, "incorrect argument\n");
		return -1;
	}
}
