CFLAGS = -Wall -lcrypto

TARGET = ./tests/generate_test

$(TARGET): $(TARGET).c
	gcc -o ./bin/test $(TARGET).c generate.c OTA.c $(CFLAGS) 

clean:
	$(RM) $(TARGET)

