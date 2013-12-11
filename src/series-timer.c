#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

void indent(int tabs);
void wait_showing_progress(int col, int tabs, char *minutes_s);
char *time_s(char *format);
void exit_error(char *msg);
int should_print_tenth_minute(int minutes);
void kill_all_players(void);
void kill_player(pid_t);
void *play_random_song(void *);
void say(char *what);

#define TIME_FORMAT_24_HR "%H:%M"
#define TIME_FORMAT_AM_PM "%l:%M %p"
#define NEWLINE printf("\n")
/* #define  MINUTE  2 */
#define MINUTE  60
#define SAYER  "say"
#define PLAYER "prf"

void exit_error(char *msg)
{
  perror(msg);
  exit(1);
}

static pid_t SERIES_TIMER__player_pid = 0;

/* This is a thread function, so that the progress line may progress */
void *play_random_song(void *param)
{
  SERIES_TIMER__player_pid = fork();
  int pid_status;
  errno = 0;
  if (SERIES_TIMER__player_pid == -1)
    exit_error("Can't fork process to start player");
  if (SERIES_TIMER__player_pid == 0)  {
    errno = 0;
    if (execlp(PLAYER, PLAYER, NULL) == -1)  {
      exit_error("Can't start player");
    }
  }
  errno = 0;
  if (waitpid(SERIES_TIMER__player_pid, &pid_status, 0) == -1) {
    fprintf(stderr, "Waiting for %d(%d): %s\n", SERIES_TIMER__player_pid, pid_status, strerror(errno));
  }
  pthread_exit(0);
}

void say(char *what)
{
  int pid_status;
  errno = 0;
  pid_t pid = fork();

  if (pid == -1)
    exit_error("Can't fork process to 'say' something.");
  if (pid == 0)  {
    errno = 0;
    if (execlp(SAYER, SAYER, what, NULL) == -1)  {
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
    char *kill_script = "for p in `pgrep 'Audirvana|VLC'`;do kill -9 $p; done";
    errno = 0;
    if (execlp("bash", "bash", "-c", kill_script, NULL) == -1)
      exit_error("Can't killall players");
  }
}

void kill_player(pid_t pid)
{
  if (pid) {
    errno = 0;
    if (kill(pid, SIGINT) == -1) {
      if (errno != ESRCH) {  /* don't complain and quit if the process isn't found,
                              * since that's the normal case.
                              */
        fprintf(stderr, "pid %d\t", pid);
        exit_error("Can't kill player:");
      }
    }
  }
}

void indent(int tabs)
{
  int i;
  for (i = 0; i < tabs; i++) {
    printf("\t");
  }
}

int should_print_tenth_minute(int minutes)
{
  if (minutes % 10 == 0) return 1;
  return 0;
}

void wait_showing_progress(int col, int tabs, char *minutes_s)
{
  int minutes = atoi(minutes_s);
  int elapsed = 0;
  indent(tabs);
  printf("%d) %s %s\n", col, minutes_s, (minutes == 1 ? "minute" : "minutes"));
  indent(tabs);
  while ( minutes > 0)
  {
    sleep(MINUTE);
    minutes--;
    elapsed++;
    if (should_print_tenth_minute(elapsed)) {
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
  pthread_t player_thread;
  void *thread_result;

  kill_all_players();

  for (i = 1, tabs = 0; i < argc; i++, tabs++) {
    /* this function works because the player runs in a thread */
    wait_showing_progress(i, tabs, argv[i]);

    /* in case we timeout during a long song */
    /* (should probably handle the thread somehow, but it *seems* to work) */
    kill_player(SERIES_TIMER__player_pid);

    /* this will block if Audirvana is playing, thus the kill above */
    say(time_s(TIME_FORMAT_AM_PM));

    /* do NOT join this thread within the loop,
     * we don't want to wait for it to complete
     */
    errno = 0;
    if (pthread_create(&player_thread, NULL, play_random_song, NULL) == -1)
      exit_error("pthread_create");
  }

  NEWLINE;
  indent(tabs);
  puts("Last");

  /* wait for last thread only */
  errno = 0;
  if (pthread_join(player_thread, &thread_result) == -1)
    exit_error("pthread_join");

  sleep(2);
  say("Finished");

  say(time_s(TIME_FORMAT_AM_PM));

  return 0;
}
