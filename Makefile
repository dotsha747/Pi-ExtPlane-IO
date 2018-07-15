CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11 -L ./lib
LDFLAGS  = -L ./common

HI="\\e[32m"
NORMAL="\\e[37m"


#lib
libSRC=$(wildcard common/*.cpp)
libOBJ=$(patsubst %.cpp, %.o, $(libSRC))
libA=common/libPiExtPlane.a


# testUI
TestUISRC=$(wildcard testUI/*.cpp)
TestUIOBJ=$(patsubst %.cpp, %.o, $(TestUISRC))
TestUILIB=PiExtPlane pthread 
TestUIEXE=bin/testUI

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

#testLED
TestLEDSRC=$(wildcard testLED/*.cpp)
TestLEDOBJ=$(patsubst %.cpp, %.o, $(TestLEDSRC))
TestLEDLIB=wiringPi
TestLEDEXE=bin/testLED

#testSevenSegment
TestSevenSegmentSRC=$(wildcard testSevenSegment/*.cpp)
TestSevenSegmentOBJ=$(patsubst %.cpp, %.o, $(TestSevenSegmentSRC))
TestSevenSegmentLIB=wiringPi
TestSevenSegmentEXE=bin/testSevenSegment

#testAlphaSegment
TestAlphaSegmentSRC=$(wildcard testAlphaSegment/*.cpp)
TestAlphaSegmentOBJ=$(patsubst %.cpp, %.o, $(TestAlphaSegmentSRC))
TestAlphaSegmentLIB=wiringPi
TestAlphaSegmentEXE=bin/testAlphaSegment

world: all

.cpp.o:
	@mkdir -p bin
	@/bin/echo -e "$(HI)[CPP] $<$(NORMAL):"
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
$(libA): $(libOBJ)
	@/bin/echo -e "$(HI)[LIB] $<$(NORMAL):"
	ar rvs $(libA) $(libOBJ)
	
$(TestUIEXE): $(TestUIOBJ) $(libA)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) $(LDFLAGS) $(TestUIOBJ) $(patsubst %, -l%, $(TestUILIB)) -o $@ 

$(TestMatrixEXE): $(TestMatrixOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestMatrixOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestMatrixLIB))

$(TestServoEXE): $(TestServoOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestServoOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestServoLIB))
	
$(TestADCEXE): $(TestADCOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestADCOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestADCLIB))

$(TestLEDEXE): $(TestLEDOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestLEDOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestLEDLIB))
	
$(TestSevenSegmentEXE): $(TestSevenSegmentOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestSevenSegmentOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestSevenSegmentLIB))

$(TestAlphaSegmentEXE): $(TestAlphaSegmentOBJ)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestAlphaSegmentOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestAlphaSegmentLIB))
	
all: $(libA) $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE) $(TestLEDEXE) \
	$(TestSevenSegmentEXE) $(TestAlphaSegmentEXE) $(TestUIEXE)

install: $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE) $(TestLEDEXE) \
	$(TestSevenSegmentEXE) $(TestAlphaSegmentEXE) $(TestUIEXE)

clean:
	rm -f $(libOBJ) $(libA) $(TestMatrixEXE) $(TestMatrixOBJ) $(TestServoEXE) $(TestServoOBJ) $(TestADCOBJ) $(TestUIOBJ) $(TestADCEXE) \
		$(TestLEDEXE) $(TestLEDOBJ) $(TestSevenSegmentEXE) $(TestSevenSegmentOBJ) \
		$(TestUIEXE)
	
distclean: clean
