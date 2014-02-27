CXX=g++
SDL_INCLUDE = -I/usr/include/SDL2 -D_REENTRANT
SDL_LIB = -L/usr/lib/x86_64-linux-gnu -lSDL2 -lSDL2_image -lSDL2_ttf
CXXFLAGS = -Wall -c -std=c++11 $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)
EXE = sdl_output

all: $(EXE)

debug: CXXFLAGS += -g
debug: $(EXE)

$(EXE): graphics.o main.o
	$(CXX) $^ $(LDFLAGS) -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

graphics.o: graphics.cpp ../include/graphics.h
	$(CXX) $(CXXFLAGS) $< -o $@

physics.o : physics.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm *.o && rm $(EXE)
