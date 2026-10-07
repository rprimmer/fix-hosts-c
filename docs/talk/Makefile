# Builds the talk: the call graph from the code, then the Typst slides.
# Needs: Typst, clang, Graphviz (dot) and Python 3. Typst fetches the
# touying and fletcher packages on the first build.

SOURCES = $(wildcard ../../src/*.c)

all: fix-hostfiles-talk.pdf

callgraph.dot: callgraph.py $(SOURCES)
	python3 callgraph.py $(SOURCES) > $@

files.dot: callgraph.py $(SOURCES)
	python3 callgraph.py --files $(SOURCES) > $@

functions.csv: callgraph.py $(SOURCES)
	python3 callgraph.py --table $(SOURCES) > $@

%.svg: %.dot
	dot -Tsvg $< -o $@

fix-hostfiles-talk.pdf: fix-hostfiles-talk.typ callgraph.svg files.svg functions.csv
	typst compile fix-hostfiles-talk.typ

watch: callgraph.svg files.svg functions.csv
	typst watch fix-hostfiles-talk.typ

clean:
	rm -f callgraph.dot files.dot functions.csv *.svg fix-hostfiles-talk.pdf

.PHONY: all watch clean
