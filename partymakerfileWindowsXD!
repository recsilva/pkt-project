SHELL := powershell.exe
.SHELLFLAGS := -Command

LLVM := $(shell llvm-config --ldflags --libs --cxxflags)
CHILDREN := (Get-ChildItem nodes\*.cpp)
all: run

run: RUNME.exe
	.\RUNME.exe

RUNME.exe: for_clanker.ll
	clang++ -o RUNME.exe .\for_clanker.ll

for_clanker.ll: src.nl nlc.exe
	type src.nl | .\nlc.exe

nlc.exe: main.cpp parser.tab.cpp
	g++ parser.tab.cpp  main.cpp $(CHILDREN) $(LLVM) -o nlc.exe

parser.tab.cpp: parser.ypp scanner.c
	win_bison -d parser.ypp

scanner.c: scanner.lex
	win_flex -o scanner.c scanner.lex


clean:
	rm scanner.c parser.tab.cpp parser.tab.hpp nlc.exe RUNME.exe
