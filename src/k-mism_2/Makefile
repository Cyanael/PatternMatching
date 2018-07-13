CXX=		g++
CXXFLAGS=	-std=c++11 -g
PROG=		k-mism
LIBS=		-lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


k-mism: k-mism_2.cpp LCP.cpp HammingDistance.cpp Fft_wak.cpp Tools.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS) 


clean:
	rm -rf *.o

mrproper: clean
	rm -rf k-mism
