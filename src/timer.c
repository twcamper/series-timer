#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

float MINUTE = 3;
float TIME_EVERY = 10;

char formatted_time[6];

void indent(int tabs);
void newline();
void wait(int minutes);
char *time_s();
void error(char *msg);

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
  puts("\n");
}

void wait(int minutes)
{
  while ( minutes > 0)
  {
    sleep(MINUTE);
    minutes--;
    if ((minutes % (int)TIME_EVERY) == 0)
      printf("%s", time_s());
    else
      printf(".");

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
