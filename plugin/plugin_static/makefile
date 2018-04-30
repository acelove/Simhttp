CFLAGS=-I../../include -I../../lib/http-parser
plugin_static.so : plugin_static.o  ../../src/plugin.o
	g++ -o $@ plugin_static.o ../../src/plugin.o  -shared -lpthread -levent

plugin_static.o : plugin_static.cpp ../../include/plugin.h  ../../lib/http-parser/http_parser.h
	g++ -fPIC -o $@ -c $< $(CFLAGS)

../../src/plugin.o : ../../src/plugin.cpp  ../../include/plugin.h
	g++ -fPIC -o $@ -c $< $(CFLAGS)

.PHONY : clean
clean:
	-rm ../../src/plugin.o *.o *.so