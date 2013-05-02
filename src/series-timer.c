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
void error(char *msg);
int should_show_tenth_minute(int minutes);
void kill_all_players();
void play(char *song_file);
void say(char *what);
char* random_song(void);

#define TIME_FORMAT_24_HR "%H:%M"
#define TIME_FORMAT_AM_PM "%l:%M %p"
#define NEWLINE printf("\n")
/* #define  MINUTE  2 */
#define MINUTE  60
#define PLAYER "/Applications/Audirvana.app/Contents/MacOS/Audirvana"
#define SAYER  "/usr/bin/say"
#define KILLER "./bin/kill.sh"
void error(char *msg)
{
  fprintf(stderr, "%s: %s %d\n", msg, strerror(errno), errno);
  exit(1);
}

char* random_song(void)
{
  FILE *pipe;
  char *cmd = "ruby ./bin/get_random_tune.rb";
  static char song_path[FILENAME_MAX];
  if ((pipe = popen(cmd, "r")) == NULL)
    error("create pipe");

  if (fgets(song_path, FILENAME_MAX + 1, pipe) == NULL)
    error("read from pipe");

  if (pclose(pipe) == EOF)
    error("close pipe");

  song_path[strlen(song_path) - 1] = '\0';
  return song_path;
}

void play(char *song_file)
{
  pid_t pid = fork();
  if (pid == -1)
    error("Can't fork process to start player");
  if (pid == 0)  {
    char *vars[] = {"open", "-a", "Audirvana", song_file, NULL};
    if (execvp("open", vars) == -1)  {
      error("Can't start player");
    }
  }
}

void say(char *what)
{
  int pid_status;
  pid_t pid = fork();

  if (pid == -1)
    error("Can't fork process to 'say' something.");
  if (pid == 0)  {
    if (execl(SAYER, SAYER, what, NULL) == -1)  {
      error("Can't start sayer");
    }
  }
  if (waitpid(pid, &pid_status, 0) == -1) {
    error("Error waiting for child process 'say()'");
  }
}

void kill_all_players()
{
  pid_t pid = fork();

  if (pid == -1)
    error("Can't fork process to kill all players");

  if (pid == 0) {
    if (execlp("bash", "bash", KILLER, "Audirvana", NULL) == -1)
      error("Can't killall players");
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
    error("Can't get localtime");

  if (strftime(formatted_time, sizeof(formatted_time), format, tmp) == (size_t)-1) {
    error("strftime returned -1");
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

    play(random_song());
  }

  NEWLINE;
  indent(tabs);
  puts("Last");

  sleep(2);
  /* 'say' is blocked by Audirvana hogging a mutual audio channel,*/
  /* which is how we keep the song alive until it's done.*/
  say("Finished");

  say(time_s(TIME_FORMAT_AM_PM));

  kill_all_players();

  return 0;
}
