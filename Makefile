.PHONY: all clean

CXX=c++ -Wall -g -O0 -std=c++14 # -fopenmp

all: flbaby flshot

flbaby: flbaby.o mywindow.o
	${CXX}  -o $@ $^ -lfltk

flshot: flshot.o playfield.o
	${CXX}  -o $@ $^ -lfltk -lfltk_images

%.cxx %.h: %.fld
	fluid -c $^

%.o : %.cc %.hh
	${CXX} -c $<

flbaby.o : flbaby.cxx flbaby.h
	${CXX} -c $<

flshot.o : flshot.cxx flshot.h
	${CXX} -c $<

clean:
	rm -vf *.o flbaby flshot
