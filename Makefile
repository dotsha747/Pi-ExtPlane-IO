CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

HI="\\e[32m"
NORMAL="\\e[37m"



# testMatrix
TestMatrixSRC=$(wildcard testMatrix/*.cpp)
TestMatrixOBJ=$(patsubst %.cpp, %.o, $(TestMatrixSRC))
TestMatrixLIB=wiringPi
TestMatrixEXE=bin/testMatrix 


world: all

.cpp.o:
	@mkdir -p bin
	@/bin/echo -e "$(HI)[CPP] $<$(NORMAL):"
	$(CXX) $(CFLAGS) -c -o $@ $<
	
$(TestMatrixEXE): $(TestMatrixOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestMatrixOBJ) $(patsubst %, -l%, $(TestMatrixLIB))
	
all: $(TestMatrixEXE)

install: $(TestMatrixEXE)

clean:
	rm -f $(TestMatrixEXE) $(TestMatrixOBJ)
	
distclean: clean
