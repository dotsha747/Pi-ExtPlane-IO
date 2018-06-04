CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

HI="\\e[32m"
NORMAL="\\e[37m"



# testMatrix
TestMatrixSRC=$(wildcard testMatrix/*.cpp)
TestMatrixOBJ=$(patsubst %.cpp, %.o, $(TestMatrixSRC))
TestMatrixLIB=wiringPi
TestMatrixEXE=bin/testMatrix 

#testServo
TestServoSRC=$(wildcard testServo/*.cpp)
TestServoOBJ=$(patsubst %.cpp, %.o, $(TestServoSRC))
TestServoLIB=wiringPi
TestServoEXE=bin/testServo 

#testADC
TestADCSRC=$(wildcard testADC/*.cpp)
TestADCOBJ=$(patsubst %.cpp, %.o, $(TestADCSRC))
TestADCLIB=
TestADCEXE=bin/testADC

world: all

.cpp.o:
	@mkdir -p bin
	@/bin/echo -e "$(HI)[CPP] $<$(NORMAL):"
	$(CXX) $(CFLAGS) -c -o $@ $<
	
$(TestMatrixEXE): $(TestMatrixOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestMatrixOBJ) $(patsubst %, -l%, $(TestMatrixLIB))

$(TestServoEXE): $(TestServoOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestServoOBJ) $(patsubst %, -l%, $(TestServoLIB))
	
$(TestADCEXE): $(TestADCOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestADCOBJ) $(patsubst %, -l%, $(TestADCLIB))
	
all: $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE)

install: $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE)

clean:
	rm -f $(TestMatrixEXE) $(TestMatrixOBJ) $(TestServoEXE) $(TestServoOBJ) $(TestADCOBJ) $(TestADCEXE)
	
distclean: clean
