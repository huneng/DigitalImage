#include <Windows.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>

struct complex{
	float real, imag;
} ;
struct complex add(complex a, complex b){
	struct complex r;
	r.real = a.real + b.real;
	r.imag = a.imag + b.imag;
	return r;
}

struct complex sub(complex a, complex b){
	struct complex r;
	r.real = a.real - b.real;
	r.imag = a.imag - b.imag;
	return r;
}

struct complex mulity(complex a, complex b){
	struct complex r;
	r.real = a.real*b.real-a.imag*b.real;
	r.imag = a.real*b.imag+a.imag*b.real;
	return r;
}
//bit位转置
void bitrp(complex* fd, int n){
	int i, j, a, b, p;
	for (i = 1, p = 0; i < n; i *= 2){
		p ++;
	}
	for (i = 0; i < n; i ++)
	{
		a = i;
		b = 0;
		for (j = 0; j < p; j ++)
		{
			b = (b << 1) + (a & 1);     // b = b * 2 + a % 2;
			a >>= 1;         // a = a / 2;
		}
		if ( b > i)
		{
			struct complex t = fd[i];
			fd[i] = fd[b];
			fd[b] = t;
		}
	}
}

const float PI = 3.1415926;
//快速傅里叶变换
void fft(complex* fd, int r){
	int n = 1<<r;
	struct complex *w = new complex[n/2];
	float angle;
	for(int i = 0; i < n/2; i++){
		angle = -2*PI*i/n;
		w[i].real = cos(angle);
		w[i].imag = sin(angle);
	}
	bitrp(fd, n);
	for(int i = 1; i < r; i++){
		int group = 1<<r-i;
		for(int j = 0; j < group; j++){
			int member = n/group;
			for(int k = 0; k < member/2; k++){
				int s = j*member;
				struct complex t = mulity(w[k*group], fd[s+k+member/2]);
				fd[s+k] = add(fd[s+k], t);
				fd[s+k+member/2] = sub(fd[s+k], t);
			}
		}
	}
	free(w);
}

//二维傅里叶变换
void fourier(unsigned char* pixels, int width, int height){
	int wc, hc;
	int wr, hr;
	wc = hc = 1;
	wr = hr = 0;
	while(wc<width){
		wc*=2;
		wr ++;
	}
	while(hc<height){
		hc*=2;
		hr ++;
	}

	struct complex* fd = (struct complex*)malloc(sizeof(complex)*wc);
	struct complex* td = (struct complex*)malloc(sizeof(complex)*hc);
	struct complex* timage = (struct complex*)malloc(sizeof(complex)*wc*hc);

	for(int y = 0; y < hc; y++)
		for(int x = 0; x < wc; x++){
			timage[y*hc+x].real = pixels[y*hc+x];	
			timage[y*hc+x].imag = 0;
		}
	
	for(int x = 0; x < wc; x++){
		for(int y = 0; y < hc; y++){
			fd[y]=timage[y*wc+x];
		}
		fft(fd, hr); 
		for(int y = 0; y < hc; y++){
			timage[y*wc+x] = fd[y];
		}
	}
	for(int y = 0; y < hc; y++){
		for(int x = 0; x < wc; x++){
			td[x] = timage[y*wc+x];
		}
		fft(td, wr);
		for(int x = 0; x < wc; x++)
			timage[y*wc+x] = td[x];
	}
	for(int y = 0; y < hc; y++)
		for(int x = 0; x < wc; x++){
			float real = timage[y*wc+x].real*timage[y*wc+x].real;
			float imag = timage[y*wc+x].imag*timage[y*wc+x].imag;
			pixels[y*wc+x] = sqrt(real+imag);
		}
	free(timage);
	free(td);
	free(fd);
}
//中值
unsigned char center_value(unsigned char* array, int length){
	for(int i = 0; i < length/2; i++){
		for(int j = i+1; j< length; j++){
			if(array[j]<array[i]){
				unsigned char t = array[j];
				array[j] = array[i];
				array[i] = t;
			}
		}
	}
	return array[length/2-1];
}
//中值滤波
void MidFilter(unsigned char* image,int width, int height)
{
	unsigned char* result = new unsigned char[width*height];
	unsigned char* rect = new unsigned char[9];
	int tem_w = 3;
	int tem_h = 3;
	for(int i = 0; i < width; i ++){
		if(i<tem_w/2||i>=width-tem_w/2)
			continue;
		for(int j = 0; j < height; j++){
			if(j<tem_h/2||j>=height-tem_h/2)
				continue;
			int count = 0;
			for(int m = -tem_w/2; m <= tem_w/2; m++){
				for(int n = -tem_h/2; n <= tem_h/2; n++){
					rect[count++] = image[(i+m)*height+j+n];
				}
			}
			result[i*height+j] = center_value(rect, count);
		}
	}
	for(int i = 0; i < width*height; i++)
		image[i] = result[i];
	delete result;
	delete rect;
}
unsigned char get_pixel(unsigned char* data, int width, int x, int y) {
	return data[y*width +x];
}

void writeToFile(char* fileName, int width, int height, unsigned char* output){
	FILE* file = fopen(fileName, "w");

	if(!file)
		return;

	fprintf(file, "P6\n");
	fprintf(file, "%d %d\n", width, height);
	fprintf(file, "255\n");

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			fprintf(file, "%c%c%c", get_pixel(output, width, x, y), get_pixel(output, width, x, y), 
             get_pixel(output, width, x, y));
		}

	}


}
int main(){
	FILE *file = fopen("lenna.bmp", "rb");
	if(file==NULL) return 0 ;
	unsigned char* pixels;
	int width, height;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	RGBQUAD* colorTable;
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);
	width = infoHeader.biWidth;
	height = infoHeader.biHeight;
	int biBitCount = infoHeader.biBitCount;
	if(biBitCount==8){
		colorTable = (RGBQUAD*)malloc( sizeof(RGBQUAD)*256);
		fread(colorTable, sizeof(RGBQUAD), 256, file);
	}
	else return 0;
	int lineBytes = (width*biBitCount/8+3)/4*4;
	pixels = (unsigned char*)malloc(sizeof(lineBytes*height)*lineBytes*height);
	fread(pixels, 1, lineBytes*height, file);
	fclose(file);

	fourier(pixels, width, height);

	//MidFilter(pixels, width, height);

	file = fopen("myimage.bmp", "wb");
	if(file==NULL)
		return 0;
	fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
	fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);
	fwrite(colorTable, sizeof(RGBQUAD), 256, file);
	fwrite(pixels, 1, lineBytes*height, file);
	fclose(file);
	free(colorTable);
	free(pixels);
	return 1;
}
