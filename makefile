CC=gcc 

SRCDIR=src
BINDIR=bin
BUILDDIR=build
TARGET=bin/myshell

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
# OBJECTS := $(SOURCES:.c=.o)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

CFLAGS=-Wall -g -std=gnu99
INC := -I include

RM = rm -f

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

compress:
	tar -cvzf 8467.tar ./*