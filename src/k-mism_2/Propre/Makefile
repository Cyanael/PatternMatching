CXX=		g++
CXXFLAGS=	-std=c++11 -O3
PROG=		k-mism
LIBS=		-L/home/cosc/tr18544/linux/Projects/Lib/fftw3/lib -lfftw3 -lm


all: clean $(PROG)


k-mism: k-mism.cpp LCP.cpp HammingDistance.cpp Fft_wak.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf k-mism
