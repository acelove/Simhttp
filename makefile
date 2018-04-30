object = src/main.o src/master.o src/worker.o src/listener.o src/connection.o src/http.o src/plugin.o lib/http-parser/http_parser.o
CFLAGS=-I./include -I./lib/http-parser/
LIBS = -ldl
simhttp : src/main.o src/master.o src/worker.o src/listener.o src/connection.o src/http.o src/plugin.o lib/http-parser/http_parser.o
	g++ -g -o $@ src/main.o src/master.o src/worker.o src/listener.o src/connection.o src/http.o src/plugin.o lib/http-parser/http_parser.o -levent $(LIBS)
src/main.o : src/main.cpp include/master.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/master.o : src/master.cpp  include/master.h include/worker.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/worker.o : src/worker.cpp include/worker.h include/util.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/listener.o : src/listener.cpp include/listener.h include/util.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/connection.o : src/connection.cpp include/connection.h include/util.h include/http.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/http.o : src/http.cpp include/http.h lib/http-parser/http_parser.h include/connection.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
src/plugin.o : src/plugin.cpp include/plugin.h
	g++ -g -o $@ -c $< --std=c++11 $(CFLAGS)
.PHONY : clean
clean :
	-rm simhttp $(object) 