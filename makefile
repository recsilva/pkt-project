#make noll compiller
LLVM := $(shell llvm-config --ldflags --libs --cxxflags)
PROG = program

.PHONY: run
run: $(PROG)
	@./$(PROG) || true

.PHONY: build
build: $(PROG)

$(PROG): temp.ll
	clang++ $^ -o $@

temp.ll: nlc src.nl
	./nlc < src.nl

nlc: main.cpp parser.tab.cpp nodes/*.cpp
	g++ $^ $(LLVM) -o $@

parser.tab.cpp : parser.ypp scanner.c
	bison -d parser.ypp

scanner.c : scanner.lex
	flex -o $@ $^

clean:
	$(RM) scanner.c parser.tab.* nlc temp.ll $(PROG)
