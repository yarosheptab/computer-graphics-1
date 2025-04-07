CXX = clang++
CXXFLAGS = -std=c++17
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio

all: main

main: main.cpp model.cpp gif_writer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS) 