.PHONY: clean
TARGET = thread_gol

all: $(TARGET)

$(TARGET): $(TARGET).c
	gcc -g -o $(TARGET) $(TARGET).c -pthread

clean:
	$(RM) $(TARGET) $(TARGET).o
