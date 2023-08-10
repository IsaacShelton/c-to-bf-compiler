
CC=gcc
SRCDIR=src
OBJDIR=obj
C_SOURCES=$(wildcard $(SRCDIR)/*.c)
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS=-c -Wall
PECKC=peckc

release: directories $(PECKC)

$(PECKC): $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -o $(PECKC)

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
	if exist $(PECKC) del $(PECKC) /Q
	del obj\*.* /Q
else
	rm -f 2> /dev/null $(PECKC)
	rm -f 2> /dev/null obj/*.*
endif
