#include <arpa/inet.h>
#include <bsd/libutil.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const char* DEFAULT_IP = "127.0.0.1";
const char* DEFAULT_PASSWORD = "PowerJudgeV1.1";
const char* DEFAULT_PORT = "12345";


void fatal_error(const char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[], char *envp[])
{
	const char* ip = DEFAULT_IP;
	const char* port = DEFAULT_PORT;
	int cfd;
	const char* password = DEFAULT_PASSWORD;
	char buffer[256] = {0};
	ssize_t numRead;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	/* Allows IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	if (getaddrinfo(ip, port, &hints, &result) != 0) {
		fatal_error("cannot get addr info");
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (cfd == -1) {
			continue;
		}
		/* On fatal_error, try next address */
		if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}
		/* Success */
		/* Connect failed: close this socket and try next address */
		close(cfd);
	}

	if (rp == NULL) {
		fatal_error("Could not connect socket to any address");
	}
	freeaddrinfo(result);

	if (write(cfd, password, strlen(password)) != strlen(password)) {
		fatal_error("Partial/failed write (password)");
	}

	numRead = read(cfd, buffer, 256);
	if (numRead == -1) {
		fatal_error("readLine");
	}
	if (numRead == 0) {
		fatal_error("Unexpected EOF from server");
	}
	printf("%s\n", buffer);

	// strcpy(buffer, "test");
	int sid = 1065;
	int cid = 0;
	int pid = 2549;
	int language = 2;
	int timeLimit = 5000;
	int memoryLimit = 65536;
	sprintf(buffer, "%d %d %d %d %d %d", sid, cid, pid, language, timeLimit, memoryLimit);

	if (write(cfd, buffer, strlen(buffer)) != strlen(buffer)) {
		fatal_error("Partial/failed write (buffer)");
	}

	numRead = read(cfd, buffer, 256);
	if (numRead == -1) {
		fatal_error("readLine");
	}
	if (numRead == 0) {
		fatal_error("Unexpected EOF from server");
	}
	printf("%s\n", buffer);


	return 0;
}
