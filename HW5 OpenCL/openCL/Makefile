clean:
	clear && printf '\033[3J'
	- rm -f histogram
	rm -f histogram
serAll: 
	clear && printf '\033[3J'
	g++ -std=c++11 -o histogram_Org histogram_Org.cpp
all: 
	clear && printf '\033[3J'
	g++ -std=c++11 -o histogram histogram.cpp -lOpenCL
test:
	clear && printf '\033[3J'
	time -f "%e" taskset -c 0 ./histogram 1.bmp 2.bmp 3.bmp 4.bmp 5.bmp 6.bmp
	# time -f "%e" taskset -c 0 ./histogram 1.bmp
	# time -f "%e" taskset -c 0 ./histogram 2.bmp
	# time -f "%e" taskset -c 0 ./histogram 3.bmp
	# time -f "%e" taskset -c 0 ./histogram 4.bmp
	# time -f "%e" taskset -c 0 ./histogram 5.bmp
	# time -f "%e" taskset -c 0 ./histogram 6.bmp
	
serTest:
	clear && printf '\033[3J'
	time -f "%e" taskset -c 0 ./histogram_Org 1.bmp 2.bmp 3.bmp 4.bmp 5.bmp 6.bmp
	