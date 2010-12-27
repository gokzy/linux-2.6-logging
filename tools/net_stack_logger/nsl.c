#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/net_stack_logger.h>

struct net_stack_log nsl_table[NSL_MAX_CPU][NSL_LOG_SIZE];

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
		int fd = open(NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open "NSL_DEV_NAME"\n");
			return -1;
		}
		ioctl(fd, NSL_ENABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "stop")) {
		int fd = open(NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open "NSL_DEV_NAME"\n");
			return -1;
		}
		ioctl(fd, NSL_DISABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "get")) {
		int fd, index, i, j, ret;
		char addr[INET_ADDRSTRLEN];

		fd = open(NSL_DEV_NAME, 0);
		if(fd < 0) {
			fprintf(stderr, "can't open "NSL_DEV_NAME"\n");
			return -1;
		}

		ret = ioctl(fd, NSL_GET_TABLE, (void *)nsl_table);
		if (ret) {
			fprintf(stderr, "ioctl failed %d\n", ret);
			return -1;
		}
		printf("NSL_GET_TABLE:\n");
		
		for (i = 0; i < NSL_MAX_CPU; i++) {
			for (j = 0; j < NSL_LOG_SIZE; j++) {
				if (!nsl_table[i][j].func)
					break;
				printf("cpu:%d ", i);
				printf("seq:%d ", j);
				printf("func:%d ", nsl_table[i][j].func);
				printf("eth_protocol:%d ",
				       nsl_table[i][j].eth_protocol);
				printf("ip_protocol:%d ",
				       nsl_table[i][j].ip_protocol);
				printf("ip_saddr:%s ",
				       inet_ntop(AF_INET, 
						 &nsl_table[i][j].ip_saddr,
						 addr,
						 INET_ADDRSTRLEN));
				printf("ip_daddr:%s ",
				       inet_ntop(AF_INET,
						 &nsl_table[i][j].ip_daddr,
						 addr,
						 INET_ADDRSTRLEN));
				printf("tp_sport:%d ",
				       ntohs(nsl_table[i][j].tp_sport));
				printf("tp_dport:%d ",
				       ntohs(nsl_table[i][j].tp_dport));
				printf("time:%llu ",
				       nsl_table[i][j].time);
				printf("skb:%llx\n",
				       nsl_table[i][j].skb);
			}
		}
		close(fd);

		return 0;
	}else{
		fprintf(stderr, "incorrect argument\n");
		return -1;
	}
}
