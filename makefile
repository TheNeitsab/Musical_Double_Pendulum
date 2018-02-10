#gcc -Wall -Wextra -L./lib -I./inc src/MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI.c -o MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI -lptask -lpthread -lcsfml-audio -lm `allegro-config --libs`
#---------------------------------------------------
# Target file to be compiled by default
#---------------------------------------------------
MAIN = main
#---------------------------------------------------
# CC is the compiler to be used
#---------------------------------------------------
CC = gcc
#---------------------------------------------------
# CFLAGS are the options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall -Wextra -L./lib -I./inc -lptask -lpthread -lcsfml-audio -lm
#---------------------------------------------------
# OBJS are the object files to be linked
#---------------------------------------------------
OBJ1 = functions
OBJS = $(MAIN).o $(OBJ1).o
#---------------------------------------------------
# LIBS are the external libraries to be used
#---------------------------------------------------
LIBS = `allegro-config --libs`
#---------------------------------------------------
# Name of the output file
#---------------------------------------------------
NAME = MUSICAL_DOUBLE_PENDULUM_PEIGNE_GABRIELLI
#---------------------------------------------------
# Dependencies
#---------------------------------------------------
$(MAIN): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LIBS) $(CFLAGS)
$(MAIN).o: $(MAIN).c
	$(CC) -c $(MAIN).c
$(OBJ1).o: $(OBJ1).c
	$(CC) -c $(OBJ1).c
#---------------------------------------------------
# Command that can be specified inline: make clean
#---------------------------------------------------
clean:
	rm -rf *o $(NAME)
