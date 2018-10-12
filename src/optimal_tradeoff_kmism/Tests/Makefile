CXX=		g++
CXXFLAGS=	-std=c++11 -g
PROG=		rle
LIBS=		-lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


rle: HammingDistance.cpp Fft_wak.cpp Rle.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS) 


clean:
	rm -rf *.o

mrproper: clean
	rm -rf opti
