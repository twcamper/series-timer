#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

void indent(int tabs);
void newline();
void wait(int minutes);
char *time_s();
void error(char *msg);
int should_show_tenth_minute(int minutes);

int MINUTE = 60;
char formatted_time[6];

void error(char *msg)
{
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
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

  int i;
  for (i = 1; i < argc; i++) {
    newline();
    indent(i);
    wait(atoi(argv[i]));
  }
  newline();
  return 0;
}
