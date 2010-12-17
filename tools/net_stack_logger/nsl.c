#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define NSL_DEV_NAME "nsl"
#define NSL_MAJOR 261

#define NSL_GET_INDEX _IO(NSL_MAJOR, 0)
#define NSL_RESET_INDEX _IO(NSL_MAJOR, 1)
#define NSL_GET_TABLE _IOW(NSL_MAJOR, 2, void *)
#define NSL_ENABLE _IO(NSL_MAJOR, 3)
#define NSL_DISABLE _IO(NSL_MAJOR, 4)

#define NSL_LOG_SIZE 1048576

struct net_stack_log {
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

struct net_stack_log nsl_table[NSL_LOG_SIZE];

void usage(void)
{
	printf("usage: nsl [start|stop|reset|get]\n");
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return -1;
	}

	if (!strcmp(argv[1], "start")) {
		int fd = open("/dev/"NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
			return -1;
		}
		ioctl(fd, NSL_ENABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "stop")) {
		int fd = open("/dev/"NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
			return -1;
		}
		ioctl(fd, NSL_DISABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "reset")) {
		int fd = open("/dev/"NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
			return -1;
		}
		ioctl(fd, NSL_RESET_INDEX);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "get")) {
		int fd, index, i, ret;
		char addr[INET_ADDRSTRLEN];

		fd = open("/dev/"NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open /dev/"NSL_DEV_NAME"\n");
			return -1;
		}

		index = ioctl(fd, NSL_GET_INDEX);
		printf("NSL_GET_INDEX: %d\n", index);

		ret = ioctl(fd, NSL_GET_TABLE, (void *)nsl_table);
		if (ret) {
			fprintf(stderr, "ioctl failed %d\n", ret);
			return -1;
		}
		printf("NSL_GET_TABLE:\n");
		
		for (i = 0; i < index; i++) {			
			printf("[%d] func:%d cpu:%d eth_protocol:%d ",
			       i, nsl_table[i].func, nsl_table[i].cpu, 
			       nsl_table[i].eth_protocol);
			printf("ip_protocol:%d ", nsl_table[i].ip_protocol);
			printf("ip_saddr:%s ",
			       inet_ntop(AF_INET, &nsl_table[i].ip_saddr,
					 addr, INET_ADDRSTRLEN));
			printf("ip_daddr:%s ",
			       inet_ntop(AF_INET, &nsl_table[i].ip_daddr,
					 addr, INET_ADDRSTRLEN));
			printf("tp_sport:%d tp_dport:%d time:%llu\n",
			       ntohs(nsl_table[i].tp_sport),
			       ntohs(nsl_table[i].tp_dport),
			       nsl_table[i].time);
		}
		close(fd);

		return 0;
	}else{
		fprintf(stderr, "incorrect argument\n");
		return -1;
	}
}
