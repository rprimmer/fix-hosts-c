# make bin  -> create project directory structure 
# make release -> make final version for release
# make markdown -> convert markdown files

PROGRAM = 	fix-hostfiles
CC  =		clang
CFLAGS =	-g -Wall -Wextra -DDEBUG
LDFLAGS =
SRC = 		src
OBJ = 		obj
SRCS =		$(wildcard $(SRC)/*.c)
OBJS = 		$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BINDIR = 	bin
BIN = 		$(BINDIR)/fix-hostfiles
MANPAGE =	$(PROGRAM).1

all: $(BIN)

$(BINDIR):
	mkdir -p $(SRC) $(OBJ) $(BINDIR)

$(BIN): $(OBJS) $(OBJ)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

release: CFLAGS=-Wall -Wextra -O2 -DNDEBUG
release: clean
release: markdown 
release: $(BIN)

markdown: README.md manpage.md 
	pandoc README.md -o readme.pdf
	pandoc C-Design-spec.md -o c-design-spec.pdf 
#	pandoc manpage.md -s -t man -o $(MANPAGE)

clean:
	$(RM) -rf $(BINDIR)/* $(OBJ)/* *.dSYM readme.pdf $(MANPAGE)
