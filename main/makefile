bst_dir = ../bst/
holdall_dir = ../holdall/
jdis_dir = ../jdis/

CC = gcc
CFLAGS = -std=c2x \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
  -O2 -g \
  -I$(bst_dir) -I$(holdall_dir) -I$(jdis_dir)
vpath %.c $(bst_dir)
vpath %.c $(holdall_dir)
vpath %.c $(jdis_dir)
vpath %.h $(bst_dir)
vpath %.h $(holdall_dir)
vpath %.h $(jdis_dir)
objects = main.o bst.o holdall.o jdis.o
executable = jdis
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

main.o: main.c bst.h jdis.o
bst.o: bst.c bst.h
holdall.o : holdall.c holdall.h
jdis.o : jdis.c jdis.h


include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)
