# LMABR - Levenberg-Marquardt algorithm with Bayesian regularization for nonlinear diffusion tensor estimation

This project implements diffusion tensor estimation for DTI brain dataset using Levenberg-Marquardt multivariate nonlinear regression algorithm (LMA) with Bayesian regularization (BR). The BR helps to overcome the problem in interpolating noisy data of LMA (overfitting noise).

## A. Compile

### 1. Pure LMA

	$ cd lmabr
	$ make

### 2. Pure LMA with multi-threading (using OpenMP)

	$ cd lmabr
	$ make USE_OMP=1

### 3. LMA with BR

	$ cd lmabr
	$ make USE_BR=1

### 4. LMA with BR and multi-threading (using OpenMP)

	$ cd lmabr
	$ make USE_BR=1 USE_OMP=1

### 5. Compile as shared library

	$ cd lmabr
	$ make USE_BR=1 USE_OMP=1 lib

A shared library named lmabr.so will be created. This allows to use LMABR in other high-level languages (Matlab, IDL, Python, ...) as an external library.

## B. Testing with sample data

These test results were obtained on Sony VAIO laptop with CPU Intel Core i7 Q720M 1.60GHz. The LMABR programs were compiled with OpenMP enabled. With BR enabled, number of negative coefficients in tensor fitting result was reduced noticeably.

### 0. Get sample data

Github limits file size to 100MB that causes sample data file (153MB) cannot be stored directly on it. But the sample data file can be downloaded from my dropbox share at https://www.dropbox.com/s/hltcmy04x5w77iy/masked_brain.dat

	$ cd lmabr
	$ ./utils/getsample.sh ./samples

or directly as follows:

	$ cd lmabr
	$ wget https://www.dropbox.com/s/hltcmy04x5w77iy/masked_brain.dat -O samples/masked_brain.dat

### 1. Pure LMA

	$ time ./lma samples/masked_brain.dat samples/bx.dat
	  Dataset = samples/masked_brain.dat
	  - Volumes = 35 [128 x 128 x 70]
	  - Number of voxels = 1146880
	  - Noise std = 28.8725
	  Bmatrix = samples/bx.dat
	
	Number of negative coeffs = 1719154 (~21.4%)
	
	real    0m47.721s
	user    6m19.142s
	sys     0m0.206s

### 2. LMA with BR

	$ time ./lmabr samples/masked_brain.dat samples/bx.dat
	  Dataset = samples/masked_brain.dat
	  - Volumes = 35 [128 x 128 x 70]
	  - Number of voxels = 1146880
	  - Noise std = 28.8725
	  Bmatrix = samples/bx.dat
	
	Number of negative coeffs = 755317 (~ 9.4%)
	
	real    0m26.697s
	user    3m31.380s
	sys     0m0.200s

## C. Futher speedup

LMABR can gain another ~20% speedup in calculation if using fast-math. However, the precision of fitting results could be a problem with critical DTI datasets. This option needs to be investigated furthermore before using in real problems. Enabling fast-math as follow:

	$ cd lmabr
	$ make USE_BR=1 USE_OMP=1 USE_FASTMATH=1
	$ time ./lmabr-fm samples/masked_brain.dat samples/bx.dat
	  Dataset = samples/masked_brain.dat
	  - Volumes = 35 [128 x 128 x 70]
	  - Number of voxels = 1146880
	  - Noise std = 28.8725
	  Bmatrix = samples/bx.dat
	
	Number of negative coeffs = 755319 (~ 9.4%)
	
	real    0m21.630s
	user    2m51.082s
	sys     0m0.194s

## D. Others

The LMABR code was designed to run on GPU at first that's the reason why most of memory buffers are preallocated and accessed linearly. However, this implementation was not suitable for GPU and hurt GPU performace badly (pretty naive for GPU architechure). There were some efforts to tweak the code in other to increase the performance by utilizing GPU shared-memory to store mostly use buffers (e.g. B matrix, Hessian matrix, Jacobian vector of current fitting sample) to gain some speedup in memory accessing. However, it was still naive because it caused massive reduction in number of GPU threads as well. In other words, the current LMABR code can be easily modified to compile and run on GPU as a proof-of-concept. But I personally don't think it's worth to do it that way, because we gain almost nothing or even waste more times. Besides, we can even get the results in under 30 seconds with a 5-year-old laptop with Quadcore i7 Q720M 1.6GHz as showed above. So, a high-end desktop computer can probabily reduce computation time to 5 seconds.

