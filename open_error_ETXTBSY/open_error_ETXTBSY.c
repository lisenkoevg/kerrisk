#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
  int fd;
  fd = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
    errExit("opening file %s", argv[1]);
  close(fd);
}
