
CC=gcc
SRCDIR=src
OBJDIR=obj
C_SOURCES=$(wildcard $(SRCDIR)/*.c)
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS=-c -Wall
BRAINLOVEC=brainlovec

release: directories $(BRAINLOVEC)

$(BRAINLOVEC): $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -o $(BRAINLOVEC)

$(C_OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

ifeq ($(OS), Windows_NT)
directories:
	@if not exist obj mkdir obj
else
directories:
	@mkdir -p obj
endif

clean:
ifeq ($(OS), Windows_NT)
	if exist $(BRAINLOVEC) del $(BRAINLOVEC) /Q
	del obj\*.* /Q
else
	rm -f 2> /dev/null $(BRAINLOVEC)
	rm -f 2> /dev/null obj/*.*
endif
