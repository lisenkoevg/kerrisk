#include <stdarg.h>
#include <pthread.h>
#include "tlpi_hdr.h"
#include <error_functions.h>

void dbg(const char *format, ...);
void dbg(const char *format, ...) {
  fprintf(stderr, "____");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, " thread: %lu\n", (unsigned long) pthread_self());
}

int main(int argc, char *argv[]) {
  dbg("aaa %d", 1);
}
