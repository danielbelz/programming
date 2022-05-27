/**
 * Programatically sets the interface speed in Linux
 * gcc set_eth_speed.c -o set_eth_speed
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

// autoneg off
static int autoneg_wanted = AUTONEG_DISABLE;
// speed 1000
static int speed_wanted = SPEED_1000;
// duplex full
static int duplex_wanted = DUPLEX_FULL;
// advertising
static int advertising_wanted = ADVERTISED_1000baseT_Full;

static void set_interface_speed(int fd, struct ifreq* ifr)
{
	int err;
	struct ethtool_cmd ecmd;

	ecmd.cmd = ETHTOOL_GSET;
	ifr->ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, ifr);
	if (err < 0) {
		perror("Cannot get current device settings");
	} else {
		/* Change everything the user specified. */
		if (speed_wanted != -1) {
			ethtool_cmd_speed_set(&ecmd, speed_wanted);
        }
		if (duplex_wanted != -1) {
			ecmd.duplex = duplex_wanted;
        }
		if (autoneg_wanted != -1) {
			ecmd.autoneg = autoneg_wanted;
        }
		if (advertising_wanted != -1) {
			if (advertising_wanted == 0) {
				ecmd.advertising = ecmd.supported &
				                   (ADVERTISED_10baseT_Half |
				                    ADVERTISED_10baseT_Full |
				                    ADVERTISED_100baseT_Half |
				                    ADVERTISED_100baseT_Full |
				                    ADVERTISED_1000baseT_Half |
				                    ADVERTISED_1000baseT_Full |
				                    ADVERTISED_2500baseX_Full |
				                    ADVERTISED_10000baseT_Full);
            } else {
				ecmd.advertising = advertising_wanted;
            }
		}

		/* Try to perform the update. */
		ecmd.cmd = ETHTOOL_SSET;
		ifr->ifr_data = (caddr_t)&ecmd;
		err = ioctl(fd, SIOCETHTOOL, ifr);
		if (err < 0) {
			perror("Cannot set new settings");
        }
	}
}

int main(int argc, char** argv)
{
	int sock;
	struct ifreq ifr;
	struct ethtool_cmd edata;
	int rc;
	const char* ifname;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	if (argc != 2) {
		fprintf(stderr, "Usage: set_eth_speed <interface>\n");
		return 1;
	}

	ifname = argv[1];
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_data = &edata;
	edata.cmd = ETHTOOL_GSET;

	rc = ioctl(sock, SIOCETHTOOL, &ifr);
	if (rc < 0) {
		perror("ioctl");
		exit(1);
	}
	if (ethtool_cmd_speed(&edata) != SPEED_1000) {
		printf("Interface Speed is %d, reset it to 1000\n", edata.speed);
		set_interface_speed(sock, &ifr);
	}

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