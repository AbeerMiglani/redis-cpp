#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rv == -1)
    {
        std::cerr << "Failed to connect" << std::endl;
        return 1;
    }

    char msg[] = "Hello";
    write(fd, msg, strlen(msg));

    char rbuf[64];
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n == -1)
    {
        std::cerr << "Failed to read" << std::endl;
        return 1;
    }

    std::cout << "Server says: " << rbuf << std::endl;

    close(fd);
    return 0;
}
