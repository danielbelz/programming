/**
 * Programatically retrieves the interface speed in Linux
 * gcc get_eth_speed.c -o get_eth_speed
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <string.h>
#include <stdlib.h>

// Use ethtool to get the ethernet speed

int main(int argc, char** argv)
{
	int sock;
	struct ifreq ifr;
	struct ethtool_cmd edata;
	int rc;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

    // Interface name from which we are getting the speed
	strncpy(ifr.ifr_name, "eth1_1", sizeof(ifr.ifr_name));
	ifr.ifr_data = &edata;
	edata.cmd = ETHTOOL_GSET;

    // Send the ioctl to retrieve information
	rc = ioctl(sock, SIOCETHTOOL, &ifr);
	if (rc < 0) {
		perror("ioctl");
		exit(1);
	}

    // Parse and display information
	switch (ethtool_cmd_speed(&edata)) {
        case SPEED_10:
            printf("10Mbps\n");
            break;
        case SPEED_100:
            printf("100Mbps\n");
            break;
        case SPEED_1000:
            printf("1Gbps\n");
            break;
        case SPEED_2500:
            printf("2.5Gbps\n");
            break;
        case SPEED_10000:
            printf("10Gbps\n");
            break;
        default:
            printf("Speed returned is %d\n", edata.speed);
	}

	return (0);
}