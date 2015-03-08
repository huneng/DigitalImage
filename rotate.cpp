#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MMAX(a, b) ((a)>(b) ? (a):(b))
#define MMIN(a, b) ((a)<(b) ? (a):(b))

#define PI 3.1415926


void rotate(cv::Mat &src, cv::Mat &dst, double angle, int cx, int cy)
{
    assert(src.channels() == 1);
    assert(0 <= cx && cx < src.cols);
    assert(0 <= cx && cx < src.rows);

    int x0, x1, y0, y1;
    x0 = -cx;
    x1 = src.cols-cx-1;
    y0 = -cy;
    y1 = src.rows-cy-1;

    int vertex[4][2];
    double cosa = cos(angle);
    double sina = sin(angle);
    double tana2 = tan(angle/2+DBL_MIN);

    int t = 0;

    t = x0-tana2*y1 + 0.5;
    vertex[0][1] = sina*t + y1 + 0.5;
    vertex[0][0] = t-tana2*vertex[0][1] + 0.5;

    t = x1-tana2*y1 + 0.5;
    vertex[1][1] = sina*t + y1 + 0.5;
    vertex[1][0] = t-tana2*vertex[1][1] + 0.5;

    t = x1-tana2*y0 + 0.5;
    vertex[2][1] = sina*t + y0 + 0.5;
    vertex[2][0] = t-tana2*vertex[2][1] + 0.5;

    t = x0-tana2*y0 +0.5;
    vertex[3][1] = sina*t + y0 + 0.5;
    vertex[3][0] = t-tana2*vertex[3][1] + 0.5;


    int maxx, minx, maxy, miny;

    maxx = MMAX(MMAX(vertex[0][0], vertex[1][0]), MMAX(vertex[2][0], vertex[3][0]));
    minx = MMIN(MMIN(vertex[0][0], vertex[1][0]), MMIN(vertex[2][0], vertex[3][0]));
    maxy = MMAX(MMAX(vertex[0][1], vertex[1][1]), MMAX(vertex[2][1], vertex[3][1]));
    miny = MMIN(MMIN(vertex[0][1], vertex[1][1]), MMIN(vertex[2][1], vertex[3][1]));

    int cols = abs(maxx-minx)+4;
    int rows = abs(maxy-miny)+4;

    printf("rows %d cols %d\n", rows, cols);

    dst = cv::Mat(rows, cols, src.type(), cv::Scalar::all(255));

    for(int y = -cy; y < src.rows-cy; y++)
    {
        for(int x = -cx; x < src.cols-cx; x++)
        {
            int t = x-tana2*y;
            int y_ = sina*t+y + 0.5;
            int x_ = t-tana2*y_ + 0.5;

            x_ = x_-minx+1;
            y_ = y_-miny+1;

            assert(0<=y_ && y_< rows);
            assert(0<=x_ && x_< cols);
            dst.at<uchar>(y_, x_) = src.at<uchar>(y+cy, x+cx);
        }
    }

    cv::imshow("dst", dst);
    cv::waitKey();
}


int main(int argc, char **argv)
{
    cv::Mat img = cv::imread(argv[1], 0);
    cv::Mat dst;

    for(int i = 1; i <= 10; i++)
        rotate(img, dst, PI, img.cols/2, img.rows/2);

    return 0;
}
