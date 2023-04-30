main: main.o helper.o output.o
	g++ -std=c++17 main.o helper.o output.o -o main

main.o: main.cpp
	g++ -std=c++17 -c main.cpp

helper.o: helper.cpp
	g++ -std=c++17 -c helper.cpp

output.o: output.cpp
	g++ -std=c++17 -c output.cpp

clean: 
	rm *.o main