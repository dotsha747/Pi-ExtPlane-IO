CXXFLAGS =	 -g -Wall -fmessage-length=0 -std=c++11 -L ./lib
LDFLAGS  = -L ./$(BUILDDIR)/common

HI="\\e[32m"
NORMAL="\\e[37m"

BUILDDIR=output


#lib
libSRC=$(wildcard common/*.cpp common/*/*.cpp)
libOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(libSRC))
libA=$(BUILDDIR)/common/libPiExtPlane.a


# testUI
TestUISRC=$(wildcard testUI/*.cpp)
TestUIOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestUISRC))
TestUILIB=PiExtPlane pthread 
TestUIEXE=$(BUILDDIR)/bin/testUI

# testMatrix
TestMatrixSRC=$(wildcard testMatrix/*.cpp)
TestMatrixOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestMatrixSRC))
TestMatrixLIB=wiringPi
TestMatrixEXE=$(BUILDDIR)/bin/testMatrix 

#testServo
TestServoSRC=$(wildcard testServo/*.cpp)
TestServoOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestServoSRC))
TestServoLIB=wiringPi
TestServoEXE=$(BUILDDIR)/bin/testServo 

#testADC
TestADCSRC=$(wildcard testADC/*.cpp)
TestADCOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestADCSRC))
TestADCLIB=
TestADCEXE=$(BUILDDIR)/bin/testADC

#testLED
TestLEDSRC=$(wildcard testLED/*.cpp)
TestLEDOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestLEDSRC))
TestLEDLIB=wiringPi
TestLEDEXE=$(BUILDDIR)/bin/testLED

#testSevenSegment
TestSevenSegmentSRC=$(wildcard testSevenSegment/*.cpp)
TestSevenSegmentOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestSevenSegmentSRC))
TestSevenSegmentLIB=wiringPi
TestSevenSegmentEXE=$(BUILDDIR)/bin/testSevenSegment

#testAlphaSegment
TestAlphaSegmentSRC=$(wildcard testAlphaSegment/*.cpp)
TestAlphaSegmentOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestAlphaSegmentSRC))
TestAlphaSegmentLIB=wiringPi
TestAlphaSegmentEXE=$(BUILDDIR)/bin/testAlphaSegment

#testZ
TestZSRC=$(wildcard testZ/*.cpp)
TestZOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(TestZSRC)) 
TestZLIB=PiExtPlane pthread
TestZEXE=$(BUILDDIR)/bin/testZ

#PiExtPlane
PIEXTPLANEIOSRC=$(wildcard piExtPlaneIO/*.cpp)
PIEXTPLANEIOOBJ=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(PIEXTPLANEIOSRC)) 
PIEXTPLANEIOLIB=PiExtPlane pthread XPlaneExtPlaneClient XPlaneUDPClient tinyxml2
PIEXTPLANEIOEXE=$(BUILDDIR)/bin/piExtPlaneIO


world: all

$(BUILDDIR)/%.o:%.cpp
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[CPP] $<$(NORMAL):"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.cpp.o:
	@mkdir -p bin
	@/bin/echo -e "$(HI)[CPP] $<$(NORMAL):"
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
$(libA): $(libOBJ)
	@/bin/echo -e "$(HI)[LIB] $<$(NORMAL):"
	ar rvs $(libA) $(libOBJ)
	
$(TestUIEXE): $(TestUIOBJ) $(libA)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) $(LDFLAGS) $(TestUIOBJ) $(patsubst %, -l%, $(TestUILIB)) -o $@ 

$(TestMatrixEXE): $(TestMatrixOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestMatrixOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestMatrixLIB))

$(TestServoEXE): $(TestServoOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestServoOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestServoLIB))
	
$(TestADCEXE): $(TestADCOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestADCOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestADCLIB))

$(TestLEDEXE): $(TestLEDOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestLEDOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestLEDLIB))
	
$(TestSevenSegmentEXE): $(TestSevenSegmentOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestSevenSegmentOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestSevenSegmentLIB))

$(TestAlphaSegmentEXE): $(TestAlphaSegmentOBJ)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestAlphaSegmentOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestAlphaSegmentLIB))
	
$(TestZEXE): $(TestZOBJ) $(libA)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(TestZOBJ) $(LDFLAGS) $(patsubst %, -l%, $(TestZLIB))	

$(PIEXTPLANEIOEXE): $(PIEXTPLANEIOOBJ) $(libA)
	@mkdir -p $(dir $@)
	@/bin/echo -e "$(HI)[EXE] $<$(NORMAL):"
	$(CXX) -o $@ $(PIEXTPLANEIOOBJ) $(LDFLAGS) $(patsubst %, -l%, $(PIEXTPLANEIOLIB))	
	
all: $(libA) $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE) $(TestLEDEXE) \
	$(TestSevenSegmentEXE) $(TestAlphaSegmentEXE) $(TestUIEXE) $(TestZEXE) \
	$(PIEXTPLANEIOEXE)

install: $(TestMatrixEXE) $(TestServoEXE) $(TestADCEXE) $(TestLEDEXE) \
	$(TestSevenSegmentEXE) $(TestAlphaSegmentEXE) $(TestUIEXE) $(TestZEXE) \
	$(PIEXTPLANEIOEXE)

clean:
	rm -rf $(BUILDDIR)
	
distclean: clean
