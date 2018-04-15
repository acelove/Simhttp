object = src/main.o src/master.o src/worker.o src/listener.o src/connection.o
CFLAGS=-I./include
simhttp : src/main.o src/master.o src/worker.o src/listener.o src/connection.o
	g++ -g -o $@ src/main.o src/master.o src/worker.o src/listener.o src/connection.o -levent
src/main.o : src/main.cpp include/master.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/master.o : src/master.cpp  include/master.h include/worker.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/worker.o : src/worker.cpp include/worker.h include/util.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/listener.o : src/listener.cpp include/listener.h include/util.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/connection.o : src/connection.cpp include/connection.h include/util.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
.PHONY : clean
clean :
	rm simhttp $(object) 