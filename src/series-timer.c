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
void wait_showing_progress(int col, int tabs, char *minutes_s);
char *time_s(char *format);
void error(char *msg);
int should_show_tenth_minute(int minutes);
void kill_all_players();
void play(char *song_file);
char *random_song();

#define TIME_FORMAT_24_HR "%H:%M"
#define TIME_FORMAT_AM_PM "%l:%M %p"
int MINUTE = 2;
/*int MINUTE = 60;*/
char formatted_time[6];
char *PLAYER = "/Applications/Audirvana.app/Contents/MacOS/Audirvana";
char *SAYER = "./bin/say";
char *KILLER = "./bin/kill.sh";
void error(char *msg)
{
  fprintf(stderr, "%s: %s %d\n", msg, strerror(errno), errno);
  exit(1);
}

char *random_song()
{
  return "../Music/rock/steely_dan/cant_buy_a_thrill/03 - Kings.flac";
}

void play(char *song_file)
{
  pid_t pid = fork();

  if (pid == -1)
    error("Can't fork process to start player");
  if (pid == 0)  {
    if (execlp("open", "open", "-a", "Audirvana", song_file, NULL) == -1)  {
    /*if (execl(PLAYER, PLAYER, "../Music/rock/steely_dan/cant_buy_a_thrill/03 - Kings.flac", NULL) == -1)  {*/
      error("Can't start player");
    }
  }
  /*printf("Started player: %d\n", pid);*/
}

void kill_all_players()
{
  pid_t pid = fork();

  if (pid == -1)
    error("Can't fork process to kill all players");

  if (pid == 0) {
    /*if (execlp("bash", "bash", KILLER, "Audirvana", NULL) == -1)*/
    if (execlp("killall", "killall", "-SIGKILL", "Audirvana", NULL) == -1)
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
  newline();
  newline();
}

char *time_s(char *format)
{
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

int main(int argc, char *argv[])
{
  int i, tabs;
  for (i = 1; i < argc; i++) {
    tabs = i - 1;
    wait_showing_progress(i, tabs, argv[i]);
    kill_all_players();
    play(random_song());
  }
  newline();
  indent(tabs + 1);
  puts("Last");
  sleep(2);
  kill_all_players();

  return 0;
}
