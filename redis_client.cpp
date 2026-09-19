#include <iostream>
#include <cassert>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static int32_t query(int fd, const char *text);
static void msg (const std::string &message);
static int32_t read_full (int fd, char *buf, size_t n);
static int32_t write_all (int fd, const char *buf, size_t n);

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
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rv == -1)
    {
        std::cerr << "Failed to connect" << std::endl;
        return 1;
    }

    int32_t err = query(fd, "hello1");
    if (err)
    {
        goto L_DONE;
    }

    err = query(fd, "hello2");
    if (err)
    {
        goto L_DONE;
    }

L_DONE:
    close(fd);
    return 0;
    /* char msg[] = "Hello";
    write(fd, msg, strlen(msg));

    char rbuf[64];
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n == -1)
    {
        std::cerr << "Failed to read" << std::endl;
        return 1;
    }

    std::cout << "Server says: " << rbuf << std::endl; */

    close(fd);
    return 0;
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char *buf, size_t n)
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

static int32_t write_all(int fd, const char *buf, size_t n)
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

static int32_t query(int fd, const char *text)
{
    uint32_t len = (uint32_t) strlen(text);
    if (len > k_max_msg)
    {
        return -1;
    }

    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(fd, wbuf, 4 + len))
    {
        return err;
    }

    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err)
    {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&len, rbuf, 4);
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    err = read_full(fd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    std::cout << "Server says: " << std::string(rbuf + 4, len) << std::endl;
    return 0;
}

static void msg (const std::string &message)
{
    std::cerr << message << std::endl;
}
