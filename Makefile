CFLAGS = -Wall

ifeq ($(OS),Windows_NT)

else
	CFLAGS += -lcrypto
endif

TARGET = ./tests/$(test)

$(TARGET): $(TARGET).c
	gcc -o ./bin/$(test) $(TARGET).c generate.c OTA.c $(CFLAGS) 

clean:
	$(RM) ./bin/$(test)