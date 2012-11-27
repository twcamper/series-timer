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
void say(char *what);
char* random_song();

#define TIME_FORMAT_24_HR "%H:%M"
#define TIME_FORMAT_AM_PM "%l:%M %p"
int MINUTE = 2;
/*int MINUTE = 60;*/
char formatted_time[9];
char *PLAYER = "/Applications/Audirvana.app/Contents/MacOS/Audirvana";
char *SAYER = "/usr/bin/say";
char *KILLER = "./bin/kill.sh";
void error(char *msg)
{
  fprintf(stderr, "%s: %s %d\n", msg, strerror(errno), errno);
  exit(1);
}

char* random_song()
{
  int fd[2];
  if (pipe(fd) == -1)
    error("Can't create the pipe in function random_song()");

  pid_t pid = fork();
  if (pid == -1)
    error("Can't fork random_song process");

  /*are we the child process?*/
  if (pid == 0)  {
    /*write to stdout*/
    dup2(fd[1], 1);
    /*close the read-end of the pipe, which the child won't use*/
    close(fd[0]);
    if (execlp("ruby", "ruby", "./bin/get_random_tune.rb", NULL) == -1)
      error("Error from script ''");
  }

  /*read from stdin*/
  dup2(fd[0], 0);
  /*close the write-end of the pipe, which the parent doesn't use*/
  close(fd[1]);

  char *song_path_ = malloc(sizeof(char *));
  printf("%p\n", song_path_);
  fscanf(stdin, "%[^\n]",  song_path_);

  return song_path_;
}

void play(char *song_file)
{
  puts(song_file);
  pid_t pid = fork();
  char *vars[] = {"open", "-a", "Audirvana", song_file, NULL};
  if (pid == -1)
    error("Can't fork process to start player");
  if (pid == 0)  {
    if (execvp("open", vars) == -1)  {
      error("Can't start player");
    }
  }
  free(song_file);
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
  for (i = 1, tabs = 0; i < argc; i++, tabs++) {
    wait_showing_progress(i, tabs, argv[i]);

    kill_all_players();

    say(time_s(TIME_FORMAT_AM_PM));

    /*char song_[1024];*/
    play(random_song());
  }
  newline();
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
