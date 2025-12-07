#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p2c[2]; // parent -> child
  int c2p[2]; // child -> parent
  char buf = 'A';

  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // child
    close(p2c[1]); // close write end from parent->child
    close(c2p[0]); // close read end child->parent

    if (read(p2c[0], &buf, 1) != 1) {
      fprintf(2, "pingpong: child read failed\n");
      exit(1);
    }
    printf("%d: received ping\n", getpid());

    if (write(c2p[1], &buf, 1) != 1) {
      fprintf(2, "pingpong: child write failed\n");
      exit(1);
    }
    close(p2c[0]);
    close(c2p[1]);
    exit(0);
  } else {
    // parent
    close(p2c[0]); // close read end from parent->child
    close(c2p[1]); // close write end child->parent

    if (write(p2c[1], &buf, 1) != 1) {
      fprintf(2, "pingpong: parent write failed\n");
      exit(1);
    }

    if (read(c2p[0], &buf, 1) != 1) {
      fprintf(2, "pingpong: parent read failed\n");
      exit(1);
    }
    printf("%d: received pong\n", getpid());

    close(p2c[1]);
    close(c2p[0]);
    wait(0);
    exit(0);
  }
}