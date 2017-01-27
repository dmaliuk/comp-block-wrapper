CPP_FLAGS := -std=c++14 -O3 -g
CPP_FILES := $(wildcard *.cpp)
EXE_FILES := $(CPP_FILES:.cpp=)
$(info $(EXE_FILES))

all: $(EXE_FILES)

%.o:%.cpp
	g++ $(CPP_FLAGS) $< -c
	objdump -M intel -S $@ > obj.asm

%:%.o
	g++  $(CPP_FLAGS) $< -o $@

clean:
	rm -f *.o $(EXE_FILES)
