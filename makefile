object = src/main.o src/master.o src/worker.o 
simhttp : src/main.o src/master.o src/worker.o
	g++ -g -o $@ src/main.o src/master.o src/worker.o -levent
src/main.o : src/main.cpp include/master.h
	g++ -g -o $@ -c $< --std=c++11
src/master.o : src/master.cpp include/master.h include/worker.h
	g++ -g -o $@ -c $< --std=c++11
src/worker.o : src/worker.cpp include/worker.h
	g++ -g -o $@ -c $< --std=c++11
.PHONY : clean
clean :
	rm simhttp $(object) 