CC=clang
CFLAGS=-O2 -Wall -g

UNAME_S:=$(shell uname -s)

NAME=pokelike
SOURCES=$(shell find src -iname '*.[ch]')

INCLUDES=
INCLUDES=-Ideps/portaudio/include
INCLUDES+=-Ideps/libsndfile/src

LIBS=
# TODO package ncurses in repository?
LIBS=-lncurses
LIBS+=deps/portaudio/lib/.libs/libportaudio.a
LIBS+=deps/libsndfile/src/.libs/libsndfile.a

ifeq ($(UNAME_S),Darwin)
  LIBS+=-framework CoreFoundation -framework CoreServices -framework CoreAudio -framework AudioToolbox -framework AudioUnit
else ifeq ($(UNAME_S),Linux)
  LIBS+=-lm -lrt -lasound -ljack -pthread
endif

all: $(NAME)

.PHONY: deps
deps: deps/.portaudio deps/.libsndfile

deps/.portaudio:
	cd deps/portaudio && ./configure && make
	touch $@

LIBSNDFILE_CFLAGS=
ifeq ($(UNAME_S),Darwin)
  LIBSNDFILE_CFLAGS+=-I/Applications/Xcode.app/Contents/Developer//Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
endif

deps/.libsndfile:
	cd deps/libsndfile && ./configure CFLAGS="$(LIBSNDFILE_CFLAGS)" && make
	touch $@

$(NAME): deps $(SOURCES)
	$(CC) $(CFLAGS) -o $(NAME) $(filter-out %.h, $(SOURCES)) $(INCLUDES) $(LIBS)

clean:
	rm -rf deps/.* 2> /dev/null
