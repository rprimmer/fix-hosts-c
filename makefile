# make bin -> create project directory structre
# make release -> make final version for release

CC  =		clang
CFLAGS =	-g -Wall -Wextra
LDFLAGS =
SRC = 		src
OBJ = 		obj
SRCS =		$(wildcard $(SRC)/*.c)
OBJS = 		$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BINDIR = 	bin
BIN = 		$(BINDIR)/myapp

all: $(BIN)

$(BINDIR):
	mkdir -p $(SRC) $(OBJ) $(BINDIR)

$(BIN): $(OBJS) $(OBJ)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

release: CFLAGS=-Wall -Wextra -O2 -DNDEBUG
release: clean
release: $(BIN)

readme: README.md
	pandoc README.md -o readme.pdf

clean:
	$(RM) -rf $(BINDIR)/* $(OBJ)/* *.dSYM readme.pdf
