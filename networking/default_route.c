/**
 * Programatically retrieves the default route in Linux
 * gcc default_route.c -o default_route
 */

#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 8192

struct route_info
{
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};

static int readNlSock(int sockFd, char* bufPtr, int seqNum, int pId)
{
	struct nlmsghdr* nlHdr;
	int readLen = 0, msgLen = 0;
	do {
		/* Receive response from the kernel */
		if ((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
			perror("SOCK READ: ");
			return -1;
		}
		nlHdr = (struct nlmsghdr*)bufPtr;
		/* Check if the header is valid */
		if ((0 == NLMSG_OK(nlHdr, readLen)) || (NLMSG_ERROR == nlHdr->nlmsg_type)) {
			perror("Error in received packet");
			return -1;
		}
		/* Check if it is the last message */
		if (NLMSG_DONE == nlHdr->nlmsg_type) {
			break;
		}
		/* Else move the pointer to buffer appropriately */
		bufPtr += readLen;
		msgLen += readLen;
		/* Check if its a multi part message; return if it is not. */
		if (0 == (nlHdr->nlmsg_flags & NLM_F_MULTI)) {
			break;
		}
	} while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));
	return msgLen;
}

static char* ntoa(int addr)
{
	static char buffer[18];
	sprintf(buffer, "%d.%d.%d.%d", (addr & 0xFF0000FF), (addr & 0x00FF0000) >> 8, (addr & 0x0000FF00) >> 16, (addr & 0x000000FF) >> 24);
	return buffer;
}

/* For parsing the route info returned */
static int parseRoutes(struct nlmsghdr* nlHdr, struct route_info* rtInfo)
{
	struct rtmsg* rtMsg;
	struct rtattr* rtAttr;
	int rtLen;
	int ret;
	char* tempBuf = NULL;

	tempBuf = (char*)malloc(100);
	rtMsg = (struct rtmsg*)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing table	then return. */
	if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) {
		return -1;
    }

	/* get the rtattr field */
	rtAttr = (struct rtattr*)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);
	for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
		switch (rtAttr->rta_type) {
		case RTA_OIF:
			if_indextoname(*(int*)RTA_DATA(rtAttr), rtInfo->ifName);
			break;
		case RTA_GATEWAY:
			rtInfo->gateWay = *(u_int*)RTA_DATA(rtAttr);
			break;
		case RTA_PREFSRC:
			rtInfo->srcAddr = *(u_int*)RTA_DATA(rtAttr);
			break;
		case RTA_DST:
			rtInfo->dstAddr = *(u_int*)RTA_DATA(rtAttr);
			break;
		}
	}
	if (0 == rtInfo->dstAddr) {
		printf("default is %s, %s\n", rtInfo->ifName, ntoa(rtInfo->srcAddr));
		ret = 1;
	} else {
		ret = 0;
	}
	free(tempBuf);
	return ret;
}

int main()
{
	struct nlmsghdr* nlMsg;
	struct route_info* rtInfo;
	char msgBuf[BUFSIZE];

	int sock, len, msgSeq = 0;

	/* Create Socket */
	if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
		perror("Socket Creation: ");
    }

	/* Initialize the buffer */
	memset(msgBuf, 0, BUFSIZE);

	/* point the header and the msg structure pointers into the buffer */
	nlMsg = (struct nlmsghdr*)msgBuf;

	/* Fill in the nlmsg header*/
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
	nlMsg->nlmsg_type = RTM_GETROUTE;                      // Get the routes from kernel routing table .

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
	nlMsg->nlmsg_seq = msgSeq++;                     // Sequence of the message packet.
	nlMsg->nlmsg_pid = getpid();                     // PID of process sending the request.

	/* Send the request */
	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
		printf("Write To Socket Failed...\n");
		return -1;
	}

	/* Read the response */
	if ((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
		printf("Read From Socket Failed...\n");
		return -1;
	}
	/* Parse and print the response */
	rtInfo = (struct route_info*)malloc(sizeof(struct route_info));

	printf("Destination\tGateway\tInterface\tSource\n");
	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
		memset(rtInfo, 0, sizeof(struct route_info));
		if (parseRoutes(nlMsg, rtInfo) == 1) {
			break;
        }
	}

	free(rtInfo);
	close(sock);
	return 0;
}