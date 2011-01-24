#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/net_stack_logger.h>
#include <sys/mman.h>
#include <sys/user.h>

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
		int ret, fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}
		ret = ioctl(fd, NSL_ENABLE);
		close(fd);
		if (ret) {
			fprintf(stderr, "ioctl failed %d\n", ret);
			return -1;
		}
		return 0;
	}else if (!strcmp(argv[1], "stop")) {
		int ret, fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}
		ret = ioctl(fd, NSL_DISABLE);
		close(fd);
		if (ret) {
			fprintf(stderr, "ioctl failed %d\n", ret);
			return -1;
		}
		return 0;
	}else if (!strcmp(argv[1], "get")) {
		int fd, i;
		struct nsl_entry *nsl_table;

		fd = open("nsl0", 0);
		if(fd < 0) {
			fprintf(stderr, "can't open nsl0\n");
			return -1;
		}

		nsl_table = (struct nsl_entry *)mmap(
			0, NSL_TABLE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
		if (nsl_table == MAP_FAILED) {
			fprintf(stderr, "mmap failed\n");
			close(fd);
			return -1;
		}
		
		printf("cpu,seq,func,eth_protocol,ip_protocol,ip_saddr,ip_daddr,"
			   "tp_sport,tp_dport,time,skb_id,sock_id,pkglen,cnt,len,rec_qlen,bklg_qlen\n");
		for (i = 0; i < NSL_MAX_CPU; i++) {
			int cur, idx, j;
			
			cur = ioctl(fd, NSL_GET_INDEX, i);
			printf("cur:%d\n", cur);
			for (j = 0; j <= cur; j++) {
				char addr[INET_ADDRSTRLEN];
				
				idx = i * NSL_LOG_SIZE + j;
				printf("%d,%d,%d,%d,%d,%s,%s,"
				       "%d,%d,%llu,%llu,%llu,%llu,%u,%lu,%u,%u\n",
				       i, j, nsl_table[idx].func,
				       nsl_table[idx].eth_protocol,
				       nsl_table[idx].ip_protocol,
				       inet_ntop(AF_INET, 
						 &nsl_table[idx].ip_saddr,
						 addr,
						 INET_ADDRSTRLEN),
				       inet_ntop(AF_INET,
						 &nsl_table[idx].ip_daddr,
						 addr,
						 INET_ADDRSTRLEN),
				       ntohs(nsl_table[idx].tp_sport),
				       ntohs(nsl_table[idx].tp_dport),
				       nsl_table[idx].time,
				       nsl_table[idx].skb_id,
				       nsl_table[idx].sock_id,
				       nsl_table[idx].pktlen,
				       nsl_table[idx].cnt,
				       nsl_table[idx].len,
				       nsl_table[idx].receive_qlen,
				       nsl_table[idx].backlog_qlen);
			}
		}
		munmap(nsl_table, NSL_TABLE_SIZE);
		close(fd);

		return 0;
	}else{
		fprintf(stderr, "incorrect argument\n");
		return -1;
	}
}
