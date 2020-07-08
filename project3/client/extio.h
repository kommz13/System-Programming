
#ifndef EXTIO_H
#define EXTIO_H

ssize_t writeall(int fd, const void *buf, size_t nbyte);

ssize_t readall(int fd, void *buf, size_t nbyte);

#endif /* EXTIO_H */

