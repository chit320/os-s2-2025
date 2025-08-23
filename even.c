#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

static void hup_handler(int sig)
{
    (void)sig;
    const char msg[] = "Ouch!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}
static void int_handler(int sig)
{
    (void)sig;
    const char msg[] = "Yeah!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

static int install_handler(int signo, void (*fn)(int))
{
    struct sigaction sa;
    sa.sa_handler = fn;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    return sigaction(signo, &sa, NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        return 1;
    }
    char *endp = NULL;
    long n = strtol(argv[1], &endp, 10);
    if (!endp || *endp != '\0' || n < 0)
    {
        fprintf(stderr, "Invalid n: %s\n", argv[1]);
        return 1;
    }

    if (install_handler(SIGHUP, hup_handler) == -1)
    {
        perror("sigaction SIGHUP");
        return 1;
    }
    if (install_handler(SIGINT, int_handler) == -1)
    {
        perror("sigaction SIGINT");
        return 1;
    }

    for (long i = 0; i < n; i++)
    {
        printf("%ld\n", 2 * i);
        fflush(stdout);
        unsigned int remain = 5;
        while ((remain = sleep(remain)) != 0)
        {
        }
    }
    return 0;
}
