PROGRAM  = fix-hostfiles
CC       = clang
CPPFLAGS = -D_POSIX_C_SOURCE=200809L
CFLAGS   = -std=c11 -g -Wall -Wextra -Wpedantic -Werror
LDFLAGS  =
SRC_DIR  = src
OBJ_DIR  = obj
BIN_DIR  = bin
TEST     = test/fix-hostfiles-test.sh
SOURCES  = $(wildcard $(SRC_DIR)/*.c)
OBJECTS  = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
BINARY   = $(BIN_DIR)/$(PROGRAM)
PREFIX  ?= /opt/homebrew
BINDIR  ?= $(PREFIX)/bin
MANDIR  ?= $(PREFIX)/share/man/man1
PANDOC  ?= pandoc
DOXYGEN ?= doxygen
DOC_DIR  = docs
API_HTML = fix-hostfiles-apidoc.html
API_PDF  = fix-hostfiles-apidoc.pdf

.PHONY: all test release docs api-docs install clean

all: $(BINARY)

$(BINARY): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

test: all
	bash -n $(TEST)
	shellcheck $(TEST)
	bash $(TEST)

release: clean
	$(MAKE) CFLAGS='-std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -DNDEBUG' all

docs: readme.pdf api-docs

readme.pdf: README.md
	$(PANDOC) -V geometry:margin=0.7in $< -o $@

api-docs: Doxyfile
	$(DOXYGEN) Doxyfile > makefile.out 2>&1
	$(MAKE) -C $(DOC_DIR)/latex >> makefile.out 2>&1
	ln -sfn $(DOC_DIR)/html/index.html $(API_HTML)
	cp $(DOC_DIR)/latex/refman.pdf $(API_PDF)

install: $(BINARY) fix-hostfiles.1
	install -d $(BINDIR) $(MANDIR)
	install -m 755 $(BINARY) $(BINDIR)/$(PROGRAM)
	install -m 644 fix-hostfiles.1 $(MANDIR)/fix-hostfiles.1

clean:
	$(RM) -r $(BIN_DIR) $(OBJ_DIR)
