CXX=		g++
CXXFLAGS=	-std=c++11 -O3
PROG=		opti
LIBS=		-L/home/cosc/tr18544/linux/Projects/Lib/fftw3/lib -lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


opti: k_mism.cpp LCP.cpp HammingDistance.cpp Fft_wak.cpp Rle.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf opti
