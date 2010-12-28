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
		int fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}
		ioctl(fd, NSL_ENABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "stop")) {
		int fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}
		ioctl(fd, NSL_DISABLE);
		close(fd);

		return 0;
	}else if (!strcmp(argv[1], "get")) {
		int fd, index, i, j, ret;
		char addr[INET_ADDRSTRLEN];

		fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}

		ret = ioctl(fd, NSL_GET_TABLE, (void *)nsl_table);
		if (ret) {
			fprintf(stderr, "ioctl failed %d\n", ret);
			return -1;
		}
		printf("cpu,seq,func,eth_protocol,ip_protocol,ip_saddr,ip_daddr,"
			   "tp_sport,tp_dport,tcp_cwr,tcp_ece,tcp_urg,tcp_ack,tcp_psh,"
			   "tcp_rst,tcp_syn,tcp_fin,doff,time,skb\n");
		for (i = 0; i < NSL_MAX_CPU; i++) {
			for (j = 0; j < NSL_LOG_SIZE; j++) {
				if (!nsl_table[i][j].func)
					break;
				printf("%d,%d,%d,%d,%d,%s,%s,"
					   "%d,%d,%d,%d,%d,%d,%d,"
					   "%d,%d,%d,%x,%llu,%llx\n",
					   i, j, nsl_table[i][j].func,
				       nsl_table[i][j].eth_protocol,
					   nsl_table[i][j].ip_protocol,
					   inet_ntop(AF_INET, 
								 &nsl_table[i][j].ip_saddr,
								 addr,
								 INET_ADDRSTRLEN),
					   inet_ntop(AF_INET,
								 &nsl_table[i][j].ip_daddr,
								 addr,
								 INET_ADDRSTRLEN),
					   ntohs(nsl_table[i][j].tp_sport),
					   ntohs(nsl_table[i][j].tp_dport),
					   nsl_table[i][j].tcp_flags.cwr,
					   nsl_table[i][j].tcp_flags.ece,
					   nsl_table[i][j].tcp_flags.urg,
					   nsl_table[i][j].tcp_flags.ack,
					   nsl_table[i][j].tcp_flags.psh,
					   nsl_table[i][j].tcp_flags.rst,
					   nsl_table[i][j].tcp_flags.syn,
					   nsl_table[i][j].tcp_flags.fin,
					   nsl_table[i][j].tcp_flags.doff,
					   nsl_table[i][j].time,
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
