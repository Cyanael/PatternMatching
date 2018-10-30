CXX=		g++
CXXFLAGS=	-std=c++11 -g
PROG=		nbRun
LIBS=		-L/home/cosc/tr18544/linux/Projects/Lib/fftw3/lib -lfftw3 -lm

# .SUFFIXES:.o

# .o:
# 		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
# .c:


all: clean $(PROG)


nbRun: k_mism.cpp HammingDistance.cpp Fft_wak.cpp Rle.cpp Tools.cpp
<<<<<<< Updated upstream
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS) 
=======
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
>>>>>>> Stashed changes


clean:
	rm -rf *.o

mrproper: clean
	rm -rf opti
