all:
	gcc serial_wave.c -lm -o serial_wave
	nvcc cuda_wave.cu -o cuda_wave

runs:
	time taskset -c 0 ./serial_wave 10000 10000

runc:
	time taskset -c 0 ./cuda_wave 10000 10000
		
clean:
	rm serial_wave
	rm cuda_wave


