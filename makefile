#make noll compiller
LLVM := $(shell llvm-config --ldflags --libs --cxxflags)
PROG = program

DIRS = nodes/*.cpp nodes/base/*.cpp nodes/types/*.cpp nodes/memory/*.cpp nodes/arithmetic/*.cpp

CHILDREN := (Get-ChildItem nodes\*.cpp)

.PHONY: run, build, clear

run: clear $(PROG)
ifeq ($(OS),Windows_NT)
	-@.\$(PROG)
else
	@echo " "
	@echo " -- run --"
	@echo ./$(PROG)
	@./$(PROG)
	@echo exit
endif

clear:
	clear

$(PROG): temp.ll
ifeq ($(OS),Windows_NT)
	clang++ -o $(PROG) .\temp.ll
else
	clang++ $^ -o $@
endif

temp.ll: src.nl nlc nlc
ifeq ($(OS),Windows_NT)
	powershell.exe -Command "type src.nl | .\nlc.exe"
else
	@echo " "
	@echo " -- compile --"
	@echo " "
	./nlc < src.nl
endif

nlc: main.cpp parser.tab.cpp $(DIRS)
ifeq ($(OS),Windows_NT)
	powershell.exe -Command "g++ $^ $(LLVM) -o nlc.exe"
else
	@echo "g++ ... -o $@"
	@g++ $^ $(LLVM) -fexceptions -o $@
endif

parser.tab.cpp: parser.ypp scanner.c
ifeq ($(OS),Windows_NT)
	win_bison -d parser.ypp
else
	bison -d parser.ypp
endif

scanner.c: scanner.lex
ifeq ($(OS),Windows_NT)
	win_flex -o scanner.c scanner.lex
else
	flex -o $@ $^
endif


clean:
	$(RM) scanner.c parser.tab.* nlc temp.ll $(PROG) *.exe