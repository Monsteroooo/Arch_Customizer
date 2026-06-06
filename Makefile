CC = gcc
CFLAGS = -Wall `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4`
TARGET = hypr-tweaker

all:
	$(CC) src/main.c src/parser.c -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
