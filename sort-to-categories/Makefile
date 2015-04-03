
SRC=sort-to-categories.cpp
TARGET=sort-to-categories

CC=g++
CFLAGS=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

clean:
	rm -f *.o *~ $(TARGET)

