CXX=		g++
CXXFLAGS=	-std=c++11 -O3
PROG=		test
LIBS=		-L/home/cosc/tr18544/linux/Projects/Lib/fftw3/lib -lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


test: lancement.cpp HamDist.cpp Fft_wak.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf test
