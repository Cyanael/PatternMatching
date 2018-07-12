CXX=		g++
CXXFLAGS=	-O3 -std=c++11 
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
	rm -rf hello
