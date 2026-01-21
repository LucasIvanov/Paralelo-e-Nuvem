FLAGS=-O2 -Wall
RM=rm -f
CC=gcc

EXEC=nbody

all: $(EXEC)

$(EXEC):
	$(CC) $(FLAGS) $(EXEC).c -o $(EXEC) -lm

clean:
	$(RM) $(EXEC).o $(EXEC)
