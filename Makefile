CFLAGS = -Wall -lcrypto

TARGET = ./tests/$(TEST)

$(TARGET): $(TARGET).c
	gcc -o ./bin/$(TEST) $(TARGET).c generate.c OTA.c $(CFLAGS) 

clean:
	$(RM) ./bin/$(TEST)