# ======================================================
#   Inner Makefile (Makefile/Makefile)
#   Handles build logic for myshell
# ======================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -g -I../include
SRCDIR  := ../src
OBJDIR  := ../obj
BINDIR  := ../bin
TARGET  := $(BINDIR)/myshell

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all clean run dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(OBJDIR) $(BINDIR)

$(TARGET): $(OBJS)
	@echo "[LD] $@"
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c ../include/shell.h
	@echo "[CC] $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	@echo "--------------------------------------"
	@echo " Running Custom Shell (myshell)"
	@echo "--------------------------------------"
	cd .. && ./bin/myshell

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(TARGET)
