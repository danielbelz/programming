/*
 * Find local ip used as source ip in ip packets.
 * Uses getsockname and UDP connection
 * gcc get_ip_addr.c -o get_ip
 */

#include <stdio.h>      //printf
#include <string.h>     //memset
#include <errno.h>      //errno
#include <sys/socket.h> //socket
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>  //getsockname
#include <unistd.h>     //close

int main(int argc, char* argv[])
{
	const char* server = "12.234.42.5";
	int dns_port = 5222;

	struct sockaddr_in serv;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	// Socket could not be created
	if (sock < 0) {
		perror("Socket error");
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*)&name, &namelen);

	char buffer[100];
	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);

	if (p != NULL) {
		printf("Local ip is : %s \n", buffer);
	} else {
		// Some error
		printf("Error number : %d . Error message : %s \n", errno, strerror(errno));
	}

	close(sock);

	return 0;
}