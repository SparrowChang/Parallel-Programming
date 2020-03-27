allser: 
	gcc -std=gnu99 -O2 -s conduction_org.c -o conduction_org -Wall
	
allmpi: 
	mpicc conduction.c -o conduction
	
runmpi:
	# mpiexec -npernode 1 --hostfile hostfile conduction 100 10
	mpiexec -npernode 1 --hostfile hostfile conduction 4 10

runser:
	# perf record  taskset -c 0 ./conduction 100 10
	# time taskset -c 0 ./conduction 100 10
	time taskset -c 0 ./conduction_org 4 10

report:
	perf report
	
clean: 
	rm -rf conduction conduction_org *.o