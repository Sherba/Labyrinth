TARGET	 = labyrinth
CC		 = gcc
STANDARD = -std=c99
CFLAGS	 = -Wall -Wextra $(STANDARD)
LDFLAGS	 = -lglut -lGLU -lGL -lm
SRC		 = main.c
OBJ		 = $(SRC:.c=.o)

.PHONY: clean zip

$(TARGET): $(OBJ) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o 
	rm -f ~*
	rm -f $(TARGET)

zip:
	zip -r $(TARGET).zip ./