#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

static void
run(char *cmd, char **base, int basec, char *argline)
{
  char *argv[MAXARG];
  int i;
  if (basec + 1 >= MAXARG) {
    fprintf(2, "xargs: too many args\n");
    exit(1);
  }
  for (i = 0; i < basec; i++)
    argv[i] = base[i];
  argv[basec] = argline;
  argv[basec + 1] = 0;

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    exec(cmd, argv);
    fprintf(2, "xargs: exec %s failed\n", cmd);
    exit(1);
  } else {
    wait(0);
  }
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: xargs command [args...]\n");
    exit(1);
  }

  char buf[512];
  int n = 0;
  char *base = argv[1];
  char **base_args = &argv[1];
  int basec = argc - 1;

  char ch;
  while (read(0, &ch, 1) == 1) {
    if (ch == '\n') {
      if (n > 0) {
        buf[n] = 0;
        run(base, base_args, basec, buf);
        n = 0;
      }
    } else {
      if (n < sizeof(buf) - 1)
        buf[n++] = ch;
    }
  }
  if (n > 0) {
    buf[n] = 0;
    run(base, base_args, basec, buf);
  }
  exit(0);
}