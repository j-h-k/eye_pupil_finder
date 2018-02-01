default: cpp run_cpp py

cpp: main.cpp
	g++ -std=c++11 main.cpp -lpthread

run_cpp: cpp
	./a.out &

py:
	sleep 1 && python3 main.py


clean:
	rm -f *.o a.out
