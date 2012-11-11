#include <Windows.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>

struct complex{
	double real, imag;
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
	r.real = a.real*b.real-a.imag*b.imag;
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

const double PI = 3.1415926535;
//快速傅里叶变换
void fft(complex* td, complex* fd, int r){
	int count = 1 << r;
	int i, j, k;
	int bfsize;
	double angle;
	complex *W, *X1, *X2, *X;
	
	W  = new complex[count / 2];
	X1 = new complex[count];
	X2 = new complex[count];

	for(i = 0; i < count/2; i++){
		angle = -i*PI*2/count;
		W[i].real = cos(angle);
		W[i].imag = sin(angle);

	}

	memcpy(X1, td, sizeof(complex)*count);
	for(k = 0; k < r; k++){
		for(j = 0; j < 1<<k; j++){
			bfsize = 1 << (r-k);
			for(i = 0; i < bfsize/2; i++){
				int p = j*bfsize;
				X2[p+i] = add(X1[p+i],X1[p+i+bfsize/2]);
				complex t = mulity(sub(X1[p+i],X1[p+i+bfsize/2]), W[i*(1<<k)]);
				X2[p+i+bfsize/2] = t;
			}
		}
		X = X1;
		X1 = X2;
		X2 = X;
	}
	for(j = 0; j < count; j++){
		int p = 0; 
		for(i = 0; i < r; i++){
			if(j&(1<<i)){
				p+=1<<(r-i-1);
			}
		}
		fd[j] = X1[p];
	}
	delete W;
	delete X1;
	delete X2;
}

void writePixelsToFile(char* fileName, unsigned char* pixels, int width, int height){
	FILE* file = fopen(fileName, "w");
	if(file==NULL) return;
	for(int i = 0; i < height; i ++){
		for(int j = 0; j < width; j++)
			fprintf(file, "%d ", pixels[i*width+j]);
	}
	fclose(file);
}
//二维傅里叶变换
void fourier(unsigned char* pixels, int width, int height){
	int i, j, w, h;
	int wp, hp;
	int lineBytes = (width*8+31)/32*4;
	w = 1;
	h = 1;
	wp = hp = 0;
	while(w*2<=width){
		wp++;
		w*=2;
	}
	while(h*2<=height){
		hp++;
		h*=2;
	}
	complex *td = (complex*)malloc(sizeof(complex)*w*h);
	complex *fd = (complex*)malloc(sizeof(complex)*w*h);
	
	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			td[i*w+j].real = pixels[lineBytes*(height-i-1)+j];
			td[i*w+j].imag = 0;
		}
	}
	
	for(i = 0; i < h; i++){
		fft(&td[w*i], &fd[w*i], wp);
	}
	
	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			td[i+j*h] = fd[i*w+j];
		}
	}
	for(i = 0; i < w; i++){
		fft(&td[h*i], &fd[h*i], hp);
	}

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			double temp = sqrt(fd[j*h+i].real*fd[j*h+i].real+
				              fd[j*h+i].imag*fd[j*h+i].imag)/100;
		
			if(temp>255){
				temp = 255;
			}
			int t = lineBytes*(height - 1 - (i<h/2 ? i+h/2 : i-h/2)) + (j<w/2 ? j+w/2 : j-w/2);
			pixels[t] = temp;
		}
	}
	
	int *mm = (int*)malloc(sizeof(int)*width*height);
	for(i = 0; i < width*height; i++){
		mm[i] = pixels[i];
	}
	writePixelsToFile("E:\\b.txt", (unsigned char*)mm, w, h);
	free(td);
	free(fd);
	free(mm);
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

//模板运算
void TemplateLaplace(unsigned char* pixels, int width, int height, double * tem ,int tem_w,int tem_h,double xishu)
{
	unsigned char* result = new unsigned char[width*height];
	for(int y = tem_h/2; y < height-tem_h/2; y++){
		for(int x = tem_w/2; x < width-tem_w/2; x++){
			int sum = 0;
			int count = 0;
			for(int m = -tem_h/2; m < tem_h/2; m++)
				for(int n = -tem_w/2; n < tem_w/2; n++)
					sum+=pixels[(y+m)*width+x+n]*tem[count++];
			result[y*width+x] = sum*xishu;

			if(result[y*width+x]<0)
				result[y*width+x] = 0;
			else if(result[y*width+x]>255)
				result[y*width+x] = 255;
			
		}
	}
	for(int i = 0; i < width*height; i++){
		pixels[i] = result[i];
	}
}

//直方图均衡
void InteEqualize(unsigned char* image, int width, int height)
{		
	int count[256];
	int size = width*height;
	memset(count, 0, sizeof(count));

	float p[256];
	for(int i = 0; i < size; i++){
		count[image[i]]++;
	}

	for(int i = 0; i < 256; i++){
		p[i] = ((float)count[i])/size;
	}
	for(int i = 0; i < size; i++){
		float t = 0;
		for(int j = 0; j <= image[i]; j++)
			t+=p[j];	
		image[i] =(unsigned int) (t*255);
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
	
	//double tem[] = {1, 1, 1, 1, -8, 1, 1 ,1, 1};
	//TemplateLaplace(pixels, width, height, tem, 3, 3, 1);

	InteEqualize(pixels, width, height);


	file = fopen("my.bmp", "wb");
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