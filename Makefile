CXX = g++
VARNAME = value
CXXFLAGS = -Wall -g
OBJS	= $(notdir $(addsuffix .o,$(basename $(SRCS))))

TARGET	= fmpart.out

SRCS	=  \
	main.cpp \
	Design_FileParse.cpp \
	FM_Part_Algorithm.cpp \
	Misc_Func.cpp


HDRS	=  \
	Headers.h \
	Design_FileParse.h \
	FM_Part_Algorithm.h \
	Misc_Func.h

$(TARGET) : $(OBJS)
	$(CXX) -o "$@" $(OBJS)

debug: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

wave:  $(OBJS) 
	$(CXX) -o $(TARGET) $(OBJS)

main.o: main.cpp
	$(CXX) -c $< -o $@
Design_FileParse.o: Design_FileParse.cpp
	$(CXX) -c $< -o $@
FM_Part_Algorithm.o: FM_Part_Algorithm.cpp
	$(CXX) -c $< -o $@
Misc_Func.o: Misc_Func.cpp
	$(CXX) -c $< -o $@

clean :
	@$(RM) -f $(OBJS) $(TARGET)

