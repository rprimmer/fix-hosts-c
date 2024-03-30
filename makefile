# make bin -> create project directory structre
# make release -> make final version for release
# make markdown -> convert markdown files
# make docs --> create API doc

PROGRAM = fix-hostfiles
CC      = clang
CFLAGS  = -g -Wall -Wextra -DDEBUG
LDFLAGS =
SRC     = src
OBJ     = obj
BINDIR  = bin
DOCDIR  = docs
SRCS    = $(wildcard $(SRC)/*.c)
OBJS    = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BIN     = $(BINDIR)/$(PROGRAM)
MANPAGE = $(PROGRAM).1
PANDOC  = pandoc
DOXYGEN = /Applications/Doxygen.app/Contents/Resources/doxygen

all: $(BINDIR) $(BIN)

$(BINDIR):
	mkdir -p $(SRC) $(OBJ) $(BINDIR) $(DOCDIR)

$(BIN): $(OBJS) $(OBJ) $(BINDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: release markdown docs clean

release: CFLAGS=-Wall -Wextra -O2 -DNDEBUG
release: clean markdown docs $(BIN)
	@echo "Release build complete."

markdown: README.md
	$(PANDOC) README.md -o readme.pdf

docs: 
	$(DOXYGEN) Doxyfile > makefile.out 2>&1
	$(MAKE) -C $(DOCDIR)/latex >> makefile.out 2>&1
	ln -sf docs/html/index.html $(PROGRAM)-apidoc.html
	cp docs/latex/refman.pdf ./$(PROGRAM)-apidoc.pdf

clean:
	$(RM) -rf $(BINDIR)/* $(OBJ)/* *.dSYM readme.pdf $(PROGRAM)-apidoc.*
