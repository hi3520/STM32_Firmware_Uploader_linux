CC = gcc
CFLAGS  = -lpthread
TARGET = bin
# define the C source files
SRC = main.c crc_16.c
INCLUDES = -I/.
BINNAME = FOTA
RM = rm

all:
	$(CC) -o $(BINNAME) $(SRC) $(INCLUDES) $(CFLAGS)

clean: 
	$(RM) $(BINNAME)

run:
	@echo "$(BINNAME) /dev/ttyUSB* filename.bin"