#include<fftw3.h>

int main(void) {
	if (argc < 2)
		cout << "Usage: ./exec textFile" << endl;

	string file_text = argc[1];
	ifstream stream_text(file_text.c_str());
	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	int N;
	stream_text >> N;
	fftw_complex *in, *fft_text;
	fftw_plan my_plan;

	text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	fft_text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	my_plan = fftw_plan_dft_1d(N, text, fft_text, FFTW_FORWARD, FFTW_ESTIMATE);

	for (int i=0; i<N; i++)
		stream_text >> text[i];
	
	

	fftw_execute(my_plan);

	fftw_destroy_plan(my_plan);
	fftw_free(in);
	fftw_free(fft_text);
	return 0;
}
