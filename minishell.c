/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */
char line[NL]; /* command input buffer */
#define MAXJ 64

struct job
{
  int id;
  pid_t pid;
  int active;
  char cmdline[NL];
};

static struct job jobs[MAXJ];
static int next_job_id = 1;

static void build_cmdline(char *dst, size_t sz, char *const argvv[])
{
  size_t pos = 0;
  for (int i = 0; argvv[i]; i++)
  {
    if (i && pos < sz - 1)
      dst[pos++] = ' ';
    for (const char *p = argvv[i]; *p && pos < sz - 1; ++p)
      dst[pos++] = *p;
    if (pos >= sz - 1)
      break;
  }
  dst[pos] = '\0';
}

static void add_job(pid_t pid, const char *cmdline)
{
  for (int i = 0; i < MAXJ; i++)
  {
    if (!jobs[i].active)
    {
      jobs[i].active = 1;
      jobs[i].pid = pid;
      jobs[i].id = next_job_id++;
      if (next_job_id <= 0)
        next_job_id = 1;
      snprintf(jobs[i].cmdline, sizeof(jobs[i].cmdline), "%s", cmdline);
      printf("[%d] %d\n", jobs[i].id, (int)pid);
      fflush(stdout);
      return;
    }
  }
  fprintf(stderr, "Jobs table full; not tracking pid %d\n", (int)pid);
}

static void reap_background(void) {
  int status;
  pid_t p;
  while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
    for (int i = 0; i < MAXJ; i++) {
      if (jobs[i].active && jobs[i].pid == p) {
        jobs[i].active = 0;
        printf("[%d]+ Done                 %s\n", jobs[i].id, jobs[i].cmdline);
        fflush(stdout);
        break;
      }
    }
  }
}

void prompt(void);

/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[])
{
  int frkRtnVal;       /* value returned by fork sys call */
  char *v[NV];         /* array of pointers to command line tokens */
  char *sep = " \t\n"; /* command line token separators    */
  int i;               /* parse index */

  /* prompt for and process one command line at a time  */

  while (1)
  { /* do Forever */
    reap_background();
    prompt();

    fgets(line, NL, stdin);

    fflush(stdin);

    // This if() required for gradescope
    if (feof(stdin))
    { /* non-zero on EOF  */
      exit(0);
    }

    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
    {
      continue; /* to prompt */
    }

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++)
    {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL)
      {
        break;
      }
    }
    /* assert i is number of tokens + 1 */

    /* Detect background '&' */
    int bg = 0;
    if (i > 0 && v[i-1] && strcmp(v[i-1], "&") == 0) {
      bg = 1;
      v[i-1] = NULL;  /* remove '&' from argv */
      i--;
    }

    if (v[0] && strcmp(v[0], "cd") == 0) {
      const char *path = v[1];
      if (!path) {
        path = getenv("HOME");
        if (!path) path = "/";
      }
      if (chdir(path) == -1) {
        perror("chdir");
      }
      continue; /* back to prompt without forking */
    }

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
      case -1: {           /* fork returns error to parent process */
        perror("fork");
        break;
      }
      case 0: {            /* code executed only by child process */ 
        execvp(v[0], v);
        perror("execvp");
        _exit(127);        /* child terminates on exec failure */
      }
      default: {           /* code executed only by parent process */
        if (bg) {
          char cmdline[NL];
          build_cmdline(cmdline, sizeof(cmdline), v);
          add_job(frkRtnVal, cmdline);
        } else {
          if (waitpid(frkRtnVal, NULL, 0) == -1) {
            perror("waitpid");
          }
        }
        break;
      }
    }               /* switch */
  }                 /* while */
}                 /* main */