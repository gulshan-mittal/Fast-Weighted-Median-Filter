main: *.cpp
	 g++ -std=c++11 *.cpp  -o output `pkg-config --cflags --libs opencv`
