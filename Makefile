CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = src/proctop
SRC = src/main.c src/ui.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
