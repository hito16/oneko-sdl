OUTPUT := oneko-sdl

SOURCES := \
	oneko_sdl.c \
	xbm_helper.c

INCLUDE := ..

OBJECTS := $(SOURCES:.c=.o)


SDL_CFLAGS := $(shell pkg-config sdl2 --cflags)
SDL_LIBS   := $(shell pkg-config sdl2 --libs)

CPPFLAGS := $(SDL_CFLAGS) -DUSE_SDL

CFLAGS += -I..

LIBS := $(SDL_LIBS) -lm

.PHONY: all clean


all: $(OUTPUT)


clean:
	$(RM) $(OUTPUT)
	$(RM) *.o


$(OUTPUT): $(OBJECTS)
	$(CC) -o $@  $^ $(CFLAGS) $(LDFLAGS) $(LIBS)
