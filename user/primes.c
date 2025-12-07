#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int leftfd) __attribute__((noreturn));

void
sieve(int leftfd)
{
  int p;
  if (read(leftfd, &p, sizeof(p)) != sizeof(p)) {
    // 没有数据，结束
    close(leftfd);
    exit(0);
  }

  printf("prime %d\n", p);

  int pfd[2];
  if (pipe(pfd) < 0) {
    fprintf(2, "primes: pipe failed\n");
    close(leftfd);
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    close(leftfd);
    close(pfd[0]); close(pfd[1]);
    exit(1);
  }

  if (pid == 0) {
    // 子进程：下游筛，读 pfd[0]
    close(pfd[1]);      // 只读
    close(leftfd);      // 关闭上游读端
    sieve(pfd[0]);      // 递归
  } else {
    // 父进程：过滤当前质数 p 后写给下游
    close(pfd[0]);      // 只写
    int x;
    while (read(leftfd, &x, sizeof(x)) == sizeof(x)) {
      if (x % p != 0) {
        if (write(pfd[1], &x, sizeof(x)) != sizeof(x)) {
          fprintf(2, "primes: write failed\n");
          break;
        }
      }
    }
    close(leftfd);
    close(pfd[1]);      // 关闭写端，通知下游结束
    wait(0);            // 等待下游完成
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int pfd[2];
  if (pipe(pfd) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    close(pfd[0]); close(pfd[1]);
    exit(1);
  }

  if (pid == 0) {
    // 子进程：作为首个筛节点
    close(pfd[1]); // 只读
    sieve(pfd[0]);
  } else {
    // 父进程：生成 2..280
    close(pfd[0]); // 只写
    for (int i = 2; i <= 280; i++) {
      if (write(pfd[1], &i, sizeof(i)) != sizeof(i)) {
        fprintf(2, "primes: write failed\n");
        break;
      }
    }
    close(pfd[1]); // 结束写
    wait(0);       // 等待整个管道结束
  }
  exit(0);
}