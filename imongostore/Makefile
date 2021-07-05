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

.PHONY: default all clean

all: $(TARGET)
release: CFLAGS = -Wall -O2 -DNDEBUG
release: clean
release: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LIBS) -o $@

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BIN)/* $(OBJ)/*