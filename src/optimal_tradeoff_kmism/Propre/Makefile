CXX=		g++
CXXFLAGS=	-std=c++11 -g
PROG=		opti
LIBS=		-lfftw3 -lm


all: clean $(PROG)


opti: k_mism.cpp LCP.cpp HammingDistance.cpp Fft_wak.cpp Rle.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS) 


clean:
	rm -rf *.o

mrproper: clean
	rm -rf opti
