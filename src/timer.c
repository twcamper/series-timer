#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

void indent(int tabs);
void newline();
void wait(int minutes);
char *time_s();
void error(char *msg);
int should_show_tenth_minute(int minutes);
void kill_player(pid_t pid);
void kill_all_players();
void start_player(pid_t *pid);

int MINUTE = 2;
/*int MINUTE = 60;*/
char formatted_time[6];
char *PLAYER = "./bin/Audirvana";
char *SAYER = "./bin/say";
char *KILLER = "./bin/kill.sh";
void error(char *msg)
{
  fprintf(stderr, "%s: %s %d\n", msg, strerror(errno)), errno;
  exit(1);
}

void start_player(pid_t *player_pid)
{
  if (*player_pid)  {
    kill_player(*player_pid);
  }

  pid_t pid = fork();

  if (pid == -1)
    error("Can't fork process to start player");
  if (pid == 0)  {
    if (execl(PLAYER, PLAYER, NULL) == -1)  {
      error("Can't start player");
    }
  }
  *player_pid = pid;
  printf("Started player: %d\n", *player_pid);
}

void kill_player(pid_t pid)
{
  printf("Killing player: %d\n", pid);
  if (kill(pid, SIGKILL) == -1)
    error("Can't kill player process");
}

void indent(int tabs)
{
  int i;
  for (i = 0; i < tabs; i++) {
    printf("\t");
  }
}

void newline()
{
  puts("");
}

int should_show_tenth_minute(int minutes)
{
  if ((minutes > 0) && (minutes % 10 == 0)) {
    return 1;
  }
  return 0;
}

void wait(int minutes)
{
  while ( minutes > 0)
  {
    sleep(MINUTE);
    minutes--;
    if (should_show_tenth_minute(minutes)) {
      printf(time_s());
    } else {
      printf(".");
    }

    fflush(stdout);
  }
  newline();
  newline();
}

char *time_s()
{
  char *format = "%H:%M";

  time_t t;
  struct tm *tmp;

  t = time(NULL);
  tmp = localtime(&t);
  if (tmp == NULL)
    error("Can't get localtime");

  if (strftime(formatted_time, sizeof(formatted_time), format, tmp) == -1) {
    error("strftime returned -1");
  }

  return formatted_time;

}

int main(int argc, char *argv[]) {

  /*kill_all_players();*/
  int i;
  pid_t *player_pid;
  *player_pid = 0;
  for (i = 1; i < argc; i++) {
    newline();
    indent(i);
    printf("%d) %s minutes", i, argv[i]);
    newline();
    indent(i);
    wait(atoi(argv[i]));
    start_player(player_pid);
  }
  newline();
  indent(argc - 1);
  puts("Last");
  kill_player(*player_pid);
  return 0;
}
