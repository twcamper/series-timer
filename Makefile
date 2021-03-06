# tested with GNU make 3.81
SHELL   = /usr/bin/env sh
CC      = clang
CFLAGS  = -g  -Wall -Wextra -pedantic -std=c99
LD      = clang

#### targets and prerequisites ####
#### CHAPTER 08 HAS ITS OWN MAKEFILE ####
TEMP        = $(shell find . -name '*.c' |  tr '\n' ' ')
OBJECTS     = $(TEMP:.c=.o)
EXECUTABLES = $(TEMP:.c=))

#### Build all executable targets, using a 'Static Pattern Rule' (GNU make manual, 4.11) ####
bin/series-timer : % : src/series-timer.o
	$(LD) $< -o $@

#### compiled object files ####
$(OBJECTS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY : .gitignore clean clean-obj clean-bin
.gitignore:
	@~/scripts/add-executables-to-gitignore.sh

clean: clean-obj clean-bin

XARGS_RM = xargs rm -fv

clean-obj:
	@find . -name '*.o' | $(XARGS_RM)

clean-bin:
	@find . -perm +111 -type f | grep -vE "\.(git|sh|rb)" | $(XARGS_RM)
