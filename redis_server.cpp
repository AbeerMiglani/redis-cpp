#include <cassert>
#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static void do_something(int connfd);

int main (void)
{
    // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM) protocol
    int fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET: IPv4, SOCK_STREAM: TCP, 0: default protocol. SOCK_DGRAM: UDP, AF_INET6: IPv6 - Terminologies
    if (fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl; // cerr is used to print error messages to the standard error stream
        return 1;
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // SO_REUSEADDR: Allows the socket to bind to an address that is already in use

    sockaddr_in addr{}; // sockaddr_in: IPv4 address structure, IPv6: sockaddr_in6
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int rv = bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));

    if (rv != 0)
    {
        perror("bind");
    }

    rv = listen(fd, SOMAXCONN);

    if (rv != 0)
    {
        perror("listen");
    }

    while (true)
    {
        sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int conn_fd = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &addrlen);
        if (conn_fd == -1)
        {
            perror("accept");
            continue;
        }

        while (true)
        {
            int32_t err = one_request(conn_fd);
            if (err != 0)
            {
                break;
            }
        }

        close(conn_fd);

        do_something(conn_fd);
        close(conn_fd);
        return 0;
    }
}

static void do_something(int connfd)
{
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n == -1)
    {
        perror("read");
        return;
    }
    std::cout << "Client says: " << rbuf << std::endl;

    char wbuf[64] = "World";
    write(connfd, wbuf, strlen(wbuf));
}

static int32_t read_full (int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
        {
            return -1;
        }
        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }

    return 0;
}

static int32_t write_all (int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
        {
            return -1;
        }
        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }

    return 0;
}

const size_t k_max_msg = 4096;

static int32_t one_request(int connfd)
{
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err)
    {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    err = read_full(connfd, rbuf + 4, len);
    if (err)
    {
        msg("read error");
        return err;
    }

    std::cout << "Client says: " << std::string(rbuf + 4, len) << std::endl;

    const char reply[] = "World";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);

    return write_all(connfd, wbuf, 4 + len);
}
