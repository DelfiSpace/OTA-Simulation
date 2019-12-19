CFLAGS = -Wall

TARGET = ./tests/$(test)
BIN = $(test)

ifeq ($(OS),Windows_NT)
	BIN = $(test).exe
else
	CFLAGS += -lcrypto
endif


$(TARGET): $(TARGET).c
	gcc -o ./bin/$(BIN) $(TARGET).c $(CFLAGS) 

clean:
	$(RM) ./bin/$(test)