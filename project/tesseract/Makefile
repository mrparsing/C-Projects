SRC = tesseract.c
BIN = tesseract

CFLAGS = -Wall -Wextra -O2
LIBS = `sdl2-config --cflags --libs`

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LIBS)

clean:
	rm -f $(BIN)