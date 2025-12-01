#make noll compiller
LLVM := $(shell llvm-config --ldflags --libs --cxxflags)
PROG = program

DIRS = nodes/*.cpp nodes/base/*.cpp nodes/types/*.cpp nodes/memory/*.cpp nodes/arithmetic/*.cpp

CHILDREN := (Get-ChildItem nodes\*.cpp)

.PHONY: run, build

run: $(PROG)
ifeq ($(OS),Windows_NT)
	.\$(PROG)
else
	@./$(PROG) || true
endif

$(PROG): temp.ll
ifeq ($(OS),Windows_NT)
	clang++ -o $(PROG) .\temp.ll
else
	clang++ $^ -o $@
endif

temp.ll: src.nl nlc nlc
ifeq ($(OS),Windows_NT)
	type src.nl | .\nlc.exe
else
	./nlc < src.nl
endif

nlc: main.cpp parser.tab.cpp $(DIRS)
ifeq ($(OS),Windows_NT)
	g++ $^ $(CHILDREN) $(LLVM) -o nlc.exe
else
#nlc: main.cpp parser.tab.cpp nodes/*.cpp
	g++ $^ $(LLVM) -o $@
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

# ifeq ($(OS),Windows_NT)
# 	SHELL := powershell.exe
# 	.SHELLFLAGS := -Command

# 	CHILDREN := (Get-ChildItem nodes\*.cpp)

# 	run: $(PROG)
# 		.\$(PROG)

# 	$(PROG): temp.ll
# 		clang++ -o $(PROG) .\temp.ll

# 	temp.ll: src.nl nlc.exe
# 		type src.nl | .\nlc.exe

# 	nlc.exe: main.cpp parser.tab.cpp nodes/*.cpp
# 		g++ parser.tab.cpp  main.cpp $(CHILDREN) $(LLVM) -o nlc.exe

# 	parser.tab.cpp: parser.ypp scanner.c
# 		win_bison -d parser.ypp

# 	scanner.c: scanner.lex
# 		win_flex -o scanner.c scanner.lex

# else
# 	run: $(PROG)
# 		@./$(PROG) || true

# 	build: $(PROG)

# 	$(PROG): temp.ll
# 		clang++ $^ -o $@

# 	temp.ll: nlc src.nl
# 		./nlc < src.nl

# 	nlc: main.cpp parser.tab.cpp nodes/*.cpp
# 		g++ $^ $(LLVM) -o $@

# 	parser.tab.cpp : parser.ypp scanner.c
# 		bison -d parser.ypp

# 	scanner.c : scanner.lex
# 		flex -o $@ $^

# endif

clean:
	$(RM) scanner.c parser.tab.* nlc temp.ll $(PROG) *.exe