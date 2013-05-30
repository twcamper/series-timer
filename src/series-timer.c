#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

void indent(int tabs);
void wait_showing_progress(int col, int tabs, char *minutes_s);
char *time_s(char *format);
void exit_error(char *msg);
int should_show_tenth_minute(int minutes);
void kill_all_players();
void play_random_song(void);
void say(char *what);

#define TIME_FORMAT_24_HR "%H:%M"
#define TIME_FORMAT_AM_PM "%l:%M %p"
#define NEWLINE printf("\n")
/* #define  MINUTE  2 */
#define MINUTE  60
#define SAYER  "/usr/bin/say"
#define KILLER "./bin/kill.sh"
#define PLAYER "~/prf/bin/prf"

/* temporary config hack */
static int expand_home_dir(char *filename, char **buffer)
{
  size_t len;
  char *tilde = strchr(filename, '~');
  if (tilde) {
    char *home = getenv("HOME");
    len = strlen(tilde + 1);
    size_t home_len = strlen(home);
    /* make sure we'll have room */
    if (home_len + strlen(filename) > FILENAME_MAX) {
      errno = ENAMETOOLONG;
      fprintf(stderr, "%s:%d expand_home_dir() %s: '%s' + '%s'\n", __FILE__, __LINE__, strerror(errno), home, tilde + 1);
      return -1;
    }
    *buffer = malloc(home_len + len + 1);
    strncpy(*buffer, home, home_len + 1);
    strncat(*buffer, tilde + 1, len + 1);
  } else {
    len = strlen(filename);
    *buffer = malloc(len + 1);
    strncpy(*buffer, filename, len + 1);
  }
  return 0;
}
void exit_error(char *msg)
{
  perror(msg);
  exit(1);
}

void play_random_song(void)
{
  pid_t pid = fork();
  errno = 0;
  if (pid == -1)
    exit_error("Can't fork process to start player");
  if (pid == 0)  {
    char *player;
    expand_home_dir(PLAYER, &player);
    errno = 0;
    if (execl(player, player, NULL) == -1)  {
      exit_error("Can't start player");
    }
    free(player);
  }
}

void say(char *what)
{
  int pid_status;
  pid_t pid = fork();

  if (pid == -1)
    exit_error("Can't fork process to 'say' something.");
  if (pid == 0)  {
    if (execl(SAYER, SAYER, what, NULL) == -1)  {
      exit_error("Can't start sayer");
    }
  }
  if (waitpid(pid, &pid_status, 0) == -1) {
    exit_error("Error waiting for child process 'say()'");
  }
}

void kill_all_players()
{
  pid_t pid = fork();

  errno = 0;
  if (pid == -1)
    exit_error("Can't fork process to kill all players");

  if (pid == 0) {
    errno = 0;
    if (execlp("bash", "bash", KILLER, "'prf|Audirvana|VLC'", NULL) == -1)
      exit_error("Can't killall players");
  }
}

void indent(int tabs)
{
  int i;
  for (i = 0; i < tabs; i++) {
    printf("\t");
  }
}

int should_show_tenth_minute(int minutes)
{
  if ((minutes > 0) && (minutes % 10 == 0)) {
    return 1;
  }
  return 0;
}

void wait_showing_progress(int col, int tabs, char *minutes_s)
{
  int minutes = atoi(minutes_s);
  indent(tabs);
  printf("%d) %s minutes\n", col, minutes_s);
  indent(tabs);
  while ( minutes > 0)
  {
    sleep(MINUTE);
    minutes--;
    if (should_show_tenth_minute(minutes)) {
      printf("%s", time_s(TIME_FORMAT_24_HR));
    } else {
      printf(".");
    }

    fflush(stdout);
  }
  NEWLINE;
  NEWLINE;
}

char *time_s(char *format)
{
  time_t t;
  struct tm *tmp;
  static char formatted_time[9];

  t = time(NULL);
  tmp = localtime(&t);
  if (tmp == NULL)
    exit_error("Can't get localtime");

  if (strftime(formatted_time, sizeof(formatted_time), format, tmp) == (size_t)-1) {
    exit_error("strftime returned -1");
  }

  return formatted_time;

}

int main(int argc, char *argv[])
{
  int i, tabs;
  for (i = 1, tabs = 0; i < argc; i++, tabs++) {
    wait_showing_progress(i, tabs, argv[i]);

    kill_all_players();

    say(time_s(TIME_FORMAT_AM_PM));

    play_random_song();
  }

  NEWLINE;
  indent(tabs);
  puts("Last");

  sleep(2);
  /* 'say' is blocked by Audirvana hogging a mutual audio channel,*/
  /* which is how we keep the song alive until it's done.*/
  say("Finished");

  say(time_s(TIME_FORMAT_AM_PM));

  return 0;
}
