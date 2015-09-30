#include <cstdlib>
#include <cstring>
//------------------------------------------------------------------------
#include <typedefs.h>
#include <dfile.h>
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
DXAPI
void lma_fit(
		int N,			//
		dtype *Bx,		// size = NxDIM
		dtype *Sn,		// size = N
		dtype *Wn,		// size = N
		dtype *Dn,		// size = DIM	-- solution
		dtype *buffer	// size = 6 DIM + 2 DIMxDIM
		);
//------------------------------------------------------------------------
void show_results(dtype *Dx, dtype *Nw)
{
	puts("---");
	for (int i = 0; i < DIM; i++) {
		printf("% 9.5f ", Dx[i]);
	}
	puts("");
	for (int i = 0; i < DIM; i++)
		printf("% 9.5f ", Nw[i]);
	puts("");
}
//------------------------------------------------------------------------
// lma_run -- Execute LMA fitting for the whole DTI packed voxels volume
//
// This function can be invoked from high-level languages, such as Matlab,
// IDL, Python... if lmabr is compiled as shared lib (MS Windows .dll file,
// Linux .so file, Mac OSX .dylib file)
//------------------------------------------------------------------------
DLAPI
vector<dtype> lma_run(
				dtype *Bx,		// B matrix
				dtype *Si,		// DTI signals
				dtype *Wt,		// Weighted noise
				int Ns,			// Number of data-points / fitting sample
				int total		// Number of voxels
				)
{
	static dtype D1[DIM] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0};

	int nc = 0;	// number of negative coeffs
	vector<dtype> Rx(DIM * total);	// results

	#pragma omp parallel for default(shared) reduction(+:nc)
	for (int n = 0; n < total; n++) {
		dtype buffer[	// Computational buffer
			6*DIM +		// - Preserve for Nw, Do, Dt, Jv, Gv, delta in LMA
			2*DIM*DIM	// - Preserve for Hx, Ax in LMA
		];
		dtype *Nw = buffer;
		dtype *Sn = &Si[n * Ns];
		dtype *Wn = &Wt[n * Ns];
		dtype *Dn = &Rx[n * DIM];

		// Initialize with guess values (all 1.0)
		memcpy(Dn, D1, sizeof(D1));
	#ifdef USE_LMA_BR
		// Initialize network weights (all 0.0)
		memset(Nw, 0, DIM * sizeof(dtype));
	#endif
		// Execute fitting
		lma_fit(Ns, Bx, Sn, Wn, Dn, buffer);
		//if (chisqs != NULL) chisqs[n] = buffer[2*DIM];

	#ifdef SHOW_TEST_RESULT
		show_results(Dn, Nw);
	#endif
		for (int i = 0; i < DIM; i++) {
			if (Dn[i] < 0) {
				Dn[i] = 0;
				nc++;
			}
		}
	}
	printf("Number of negative coeffs = %d (~%4.1f%%)\n",nc,100.*nc/DIM/total);

	return Rx;
}
//------------------------------------------------------------------------
#ifndef SHARED_LIB
//------------------------------------------------------------------------
void usage(char *prog)
{
	printf(
		"Usage:\n"
		"  %s <si.dat> <bx.dat> [nstd]\n"
		"     si.dat -- DTI data organized as stream of packed voxels as described in:\n"
		"               http://www.researchgate.net/publication/261291974\n"
		"     bx.dat -- B matrix file\n"
		"     nstd   -- Noise std used to overwrite precalculated noise std stored\n"
		"               in si.dat (optional)\n"
		"  Please try:\n"
		"    $ %s samples/masked_brain.dat samples/bx.dat\n\n",
		prog, prog);
	exit(-1);
}
//------------------------------------------------------------------------
#define show_params() {											\
	printf("  Dataset = %s\n", argv[1]);						\
	printf("  - Volumes = %d [%d x %d x %d]\n", V, W, H, S);	\
	printf("  - Number of voxels = %d\n", Nv);					\
	printf("  - Noise std = %.4f\n", RMS);						\
	printf("  Bmatrix = %s\n\n", argv[2]);						\
}
//------------------------------------------------------------------------
int main(int argc, char** argv) 
{
	dfile dfp(argv[1]);	// DTI data organized as stream of packed voxels
						// as described in:
						// http://www.researchgate.net/publication/261291974
	dfile bfp(argv[2]); // B matrix file

	if (!dfp || !bfp) {
		usage(argv[0]);
	}
	// Load meta information from header of data file
	int  S    = dfp.get<int>();		// Number of slices volume
	int  V    = dfp.get<int>();		// Number of volumes
	int  W    = dfp.get<int>();		// Image width (or slice width)
	int  H    = dfp.get<int>();		// Image height (or slice height)
	int  bpp  = dfp.get<int>();		// Bytes per pixel
	int  Ns   = dfp.get<int>();		// Number of data-points / fitting sample
	int  Nv   = dfp.get<int>();		// Number of brain voxels in volume
	dtype RMS = dfp.get<float>();	// Noise std

	if (argc >= 4) {		// Override RMS with value from command line
		dtype r = atof(argv[3]);
		if (r >= 0) RMS = r;
	}

	if (argc >= 5) {		// Override Nv with value from command line
		int n = atoi(argv[4]);
		if (n >= 1 && n <= Nv) Nv = n;
	}
	show_params();

	int Npts = Ns * Nv;		// Total number of data points
	vector<dtype> Si = dfp.get<dtype>(Npts);	// DTI signals
	vector<dtype> Bx = bfp.get<dtype>(Ns * DIM);// B matrix
	vector<dtype> Wt(Npts);						// Weighted noise

	// Adjust B matrix to avoid overflow
	for (int n = 0; n < Ns * DIM; n++) {
		if (n % DIM != 0)
			Bx[n] /= (dtype)1000.0;
	}

	// Adjust signals to avoid overflow and calculate weighted noise
	#pragma omp parallel for
	for (int i = 0; i < Npts; i++) {
		dtype s = Si[i] = Si[i] / (dtype)1000.0;
		s = RMS / s;
		Wt[i] = s * s;
	}

	// Execute fitting for whole volume
	vector<dtype> results = lma_run(&Bx[0], &Si[0], &Wt[0], Ns, Nv);

	// save results
	dfile rfp("results.dat", false);
	rfp.put(results);
	return 0;
}
//------------------------------------------------------------------------
#endif	// SHARED_LIB
