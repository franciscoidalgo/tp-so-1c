CC = gcc
CFLAGS = -g -Wall
LIBS = -lcommons -lpthread -lreadline

BIN = bin
OBJ = obj
SRC = src

TARGET = $(BIN)/i-mongo-store
SRCS = $(wildcard $(SRC)/*.c)
HEADERS = $(wildcard $(SRC)/*.h)
OBJS = $(patsubst $(SRC)/%.c, obj/%.o, $(SRCS))


SHARED_SRCDIR = ../shared/src
SHARED_ODIR = ../shared/obj
SHARED_IDIR = ../shared/include

SHARED_HEADERS = $(wildcard $(SHARED_IDIR)/*.h)
SHARED_SRC = $(wildcard $(SHARED_SRCDIR)/*.c)
SHARED_OBJ = $(patsubst $(SHARED_SRCDIR)/%.c,$(SHARED_ODIR)/%.o,$(SHARED_SRC))

.PHONY: default all clean

all: $(TARGET)
release: CFLAGS = -Wall -O2 -DNDEBUG
release: clean
release: $(TARGET)


$(TARGET): $(OBJS) $(SHARED_OBJ)
	$(CC) $(OBJS) $(CFLAGS) -I$(SHARED_IDIR) $(LIBS) -o $@

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS) $(SHARED_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_ODIR)/%.o: $(SHARED_SRCDIR)/%.c $(SHARED_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BIN)/* $(OBJ)/*