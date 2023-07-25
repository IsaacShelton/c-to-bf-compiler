
CC=gcc
SRCDIR=src
OBJDIR=obj
C_SOURCES=$(wildcard $(SRCDIR)/*.c)
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS=-c -Wall
SMOOCHC=smoochc

release: directories $(SMOOCHC)

$(SMOOCHC): $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -o $(SMOOCHC)

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
	if exist $(SMOOCHC) del $(SMOOCHC) /Q
	del obj\*.* /Q
else
	rm -f 2> /dev/null $(SMOOCHC)
	rm -f 2> /dev/null obj/*.*
endif
