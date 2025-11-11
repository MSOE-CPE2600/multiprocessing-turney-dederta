# === Compiler and Flags ===
CC = gcc
CFLAGS = -Wall -g -c
LDFLAGS = -ljpeg -lm

# === Executables ===
MANDEL_EXE = mandel
MOVIE_EXE = mandelMovie

# === Source Files ===
MANDEL_SRC = mandel.c jpegrw.c
MOVIE_SRC  = mandelMovie.c

# === Object Files ===
MANDEL_OBJ = $(MANDEL_SRC:.c=.o)
MOVIE_OBJ  = $(MOVIE_SRC:.c=.o)

# === Default Target ===
all: $(MANDEL_EXE) $(MOVIE_EXE)

# === Dependency Tracking ===
-include $(MANDEL_OBJ:.o=.d)
-include $(MOVIE_OBJ:.o=.d)

# === Build mandel ===
$(MANDEL_EXE): $(MANDEL_OBJ)
	$(CC) $(MANDEL_OBJ) $(LDFLAGS) -o $@

# === Build mandelMovie ===
$(MOVIE_EXE): $(MOVIE_OBJ)
	$(CC) $(MOVIE_OBJ) $(LDFLAGS) -o $@

# === Generic Compile Rule ===
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

# === Clean Everything ===
clean:
	rm -rf $(MANDEL_OBJ) $(MOVIE_OBJ) $(MANDEL_EXE) $(MOVIE_EXE) *.d *.jpg
