//------------------------------------------------------------------------
// A simple GUI using OpenCV API to display DTI fitting results
//------------------------------------------------------------------------
#ifdef USE_GUI
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
//------------------------------------------------------------------------
static bool show_result = false;
static IplImage *rimg = NULL;
static IplImage *pic = NULL;
static char *Dn[] = {
	"S0",
	"Dxx",
	"Dxy",
	"Dxz",
	"Dyy",
	"Dyz",
	"Dzz",
	"S0"
};
//------------------------------------------------------------------------
static
int wx[8] = {0, 1, 2, 3, 2, 3, 3, 0},
	wy[8] = {0, 0, 0, 0, 1, 1, 2, 0};
//------------------------------------------------------------------------
void gui_init(int argc, char **argv, char *buffer, int W, int H)
{
	int scale = 2;
	int offset = 0;
	show_result = argc >= 4 && strcmp(argv[3], "-show") == 0;
	if (argc >= 5) {
		scale = atoi(argv[4]);
		if (scale <= 0 || scale > 5)
			scale = 2;
	}
	if (argc >= 6) {
		offset = atoi(argv[5]);
		if (offset <= 0) offset = 0;
	}
	rimg = cvCreateImageHeader(cvSize(W, H), 8, 1);
	pic = cvCreateImage(cvSize(W*scale, H*scale), 8, 1);
	rimg->imageData = buffer;
	for (int i = 0; i < 8; i++) {
		wy[i] = wy[i] * H * scale + 28 * wy[i];
		wx[i] = wx[i] * W * scale + offset + 60 + 12 * wx[i];
	}
}
//------------------------------------------------------------------------
void gui_show(int n)
{
	if (show_result) {
		cvNamedWindow(Dn[n]);
		cvResize(rimg, pic);
		cvShowImage(Dn[n], pic);
		cvMoveWindow(Dn[n], wx[n], wy[n]);
	}
}
//------------------------------------------------------------------------
void gui_done()
{
	cvReleaseImage(&pic);
	cvReleaseImageHeader(&rimg);
	if (show_result) {
		printf("Press any key to exit...");
		fflush(stdout);
		cvWaitKey(0);
		printf("\n");
	}
}
//------------------------------------------------------------------------
#endif
