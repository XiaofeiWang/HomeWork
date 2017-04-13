#define LOG_NDDEBUG 0

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>


#include <utils/RefBase.h>
#include <utils/Log.h>
#include <gui/BitTube.h>

#define MAXEVENTS 64
#define SOCKET_BUFFER_SIZE_NON_BATCHED 4 * 1024

using namespace android;

ssize_t my_write(int fd, void const* vaddr, size_t size)
{
    ssize_t err, len;
    do {
        len = ::send(fd, vaddr, size, MSG_DONTWAIT | MSG_NOSIGNAL);
        // cannot return less than size, since we're using SOCK_SEQPACKET
        err = len < 0 ? errno : 0;
    } while (err == EINTR);
    return err == 0 ? len : -err;
}

ssize_t my_read(int fd, void* vaddr, size_t size)
{
    ssize_t err, len;
    do {
        len = ::recv(fd, vaddr, size, MSG_DONTWAIT);
        err = len < 0 ? errno : 0;
    } while (err == EINTR);
    if (err == EAGAIN || err == EWOULDBLOCK) {
        // EAGAIN means that we have non-blocking I/O but there was
        // no data to be read. Nothing the client should care about.
        return 0;
    }
    return err == 0 ? len : -err;
}

int main(int argc, char* argv[])
{
    ALOGD("looptester start");
    pid_t pid;
    int beforeError;
    int epollfd;
    int count = 0;
    struct epoll_event *events;
    struct epoll_event event_read;
    struct epoll_event event_write;
    char w_buffer[10] = "wang";
    char r_buffer[10];
    sp<BitTube> channel = new BitTube(SOCKET_BUFFER_SIZE_NON_BATCHED);
    epollfd = epoll_create(256);
    /*event_read.data.fd = channel->getFd();
    event_read.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, channel->getFd(), &event_read);
    */

    /*event_write.data.fd = channel->getSendFd();
    event_write.events = EPOLLOUT | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, channel->getSendFd(), &event_write);
    */

    /* Buffer where events are returned */
    events = (epoll_event *)calloc (MAXEVENTS, sizeof event_write);

    ALOGD("the recFd: %d, the sendfd: %d", channel->getFd(), channel->getSendFd());
    if ((pid = fork()) > 0) { // for parent
        while(1) {
            ALOGD("parent write buffer: %s", w_buffer);
            int n = epoll_wait(epollfd, events, MAXEVENTS, -1);
            ALOGD("epoll_wait return from the parent: %d", n);
            for (int i = 0 ; i< n; i++) {
                if (events[i].events & EPOLLOUT) {
                    ALOGD("the fd is : %d", events[i].data.fd);
                    beforeError = errno;
                    errno = 0;
                    count =my_write(channel->getSendFd(), w_buffer, sizeof(w_buffer));
                    ALOGD("parent write count: %d, %s", count, strerror(errno));
                    errno = beforeError;
                } else {
                    continue;
                }
            }
        }
    } else { // for child
       // while(1) {
            ALOGD("child start to read");
            memset(r_buffer,0, 10);
            //epoll_wait(epollfd, &event_read, 20, -1);
            //count = read(channel->getFd(), r_buffer, sizeof(r_buffer));
            //ALOGD("child read buffer: %s, size: %d", r_buffer, count);
            sleep(1);
        //}
    }

    if (epollfd >= 0) {
        close(epollfd);
    }
    return 0;
}
