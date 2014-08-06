CC=clang
CFLAGS=-O2 -Wall

NAME=pokelike
SOURCES=$(shell find . -iname '*.[ch]')

all: $(NAME)

$(NAME): $(SOURCES)
	$(CC) $(CFLAGS) -o $(NAME) $(filter-out *.h, $(SOURCES)) -lncurses
