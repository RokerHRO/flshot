.PHONY: all clean

CXX=c++ -Wall -g -O0 -std=c++14 # -fopenmp

all: flshot

flshot: flshot.o playfield.o
	${CXX}  -o $@ $^ -lfltk -lfltk_images

%.cxx %.h: %.fld
	fluid -c $^

%.o : %.cc %.hh
	${CXX} -c $<

flshot.o : flshot.cxx flshot.h
	${CXX} -c $<

clean:
	rm -vf *.o flshot
