main: main_raw.o helper_raw.o output_raw.o
	g++ -std=c++17 main_raw.o helper_raw.o output_raw.o -o main_raw

main.o: main_raw.cpp
	g++ -std=c++17 -c main_raw.cpp

helper.o: helper_raw.cpp
	g++ -std=c++17 -c helper_raw.cpp

output.o: output_raw.cpp
	g++ -std=c++17 -c output_raw.cpp

clean: 
	rm *.o main