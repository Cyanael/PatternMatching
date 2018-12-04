CXX=		g++
CXXFLAGS=	-std=c++11 -g
PROG=		compar
LIBS=		-L/home/cosc/tr18544/linux/Projects/Lib/fftw3/lib -lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


compar: k_mism.cpp LCP.cpp HammingDistance1.cpp Fft_wak.cpp Rle.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf opti
