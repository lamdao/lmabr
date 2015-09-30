//---------------------------------------------------------------------------
// lma.cc -- Implementation of Levenberg–Marquardt Algorithm with
//           Bayesian Regularization
//---------------------------------------------------------------------------
// Naming conventions
// - dtype: data type (double/float depends on compile directives)
// - Bx: B Matrix
// - Si: Signals
// - Wn: Weighted noises
// - Dx: D Matrix
// - Di: Dx at step #i
// - Dt: temporary Dx
// - Dn: current Dx
// - Do: old Dx
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <float.h>
//---------------------------------------------------------------------------
#include <typedefs.h>
//---------------------------------------------------------------------------
#include <fx.h>
#include <qr.h>
//---------------------------------------------------------------------------
#define lma_memcpy(dst,src,N) {		\
	for (int __i=0; __i<N; __i++)	\
		dst[__i] = src[__i];		\
}
//---------------------------------------------------------------------------
// Bayesian Regularization info
//---------------------------------------------------------------------------
struct bri_t {
	dtype a;	// alpha
	dtype b;	// beta
	dtype c;	// cost
	dtype w;	// weight
	dtype *nw;	// network weight
	// Calculate new network weights
	dtype calc_nw(dtype *delta) {
		dtype swe = 0.0;
		for (int i = 0; i < DIM; i++) {
			dtype w = nw[i] + delta[i];
			swe += w * w;
			nw[i] = w;
		}
		return swe / 2.0;
	}
	// Calculate trace
	dtype calc_tr(dtype *Hx) {
		dtype tr = 0.0;
		for (int i = 0; i < DIM; i++) {
			tr += Hx[i * DIM + i];
		}
		return tr;
	}
	// Calculate cost function
	dtype calc_cost(dtype chisq) {
		return b * chisq + a * w;
	}
	// Calculate cost function
	dtype calc_cost(dtype chisq, dtype sw) {
		return b * chisq + a * sw;
	}
	// Update alpha/beta hyperparameters
	void update(dtype N, dtype chisq, dtype tr) {
		const dtype W = DIM;
		dtype g = W - a * tr;
		b = fabs(N - g) / (2.0 * chisq);
		a = W / (2.0 * w + tr);
	}
};
//---------------------------------------------------------------------------
// lma_eval -- used by lma_update to calculate the Hessian matrix(Ax),
//             Gradient vector(Gv) and also calculate chi squared.
//---------------------------------------------------------------------------
DFAPI
dtype lma_eval(
		int N,
		dtype *Bx,		// size = NxDIM
		dtype *Si,		// size = N
		dtype *Di,		// size = DIM
		dtype *Wn,		// size = N
		dtype *Jv,		// size = DIM
		dtype *Ax,		// size = DIMxDIM
		dtype *Gv		// size = DIM
		)
{
	int i, j, k;
	dtype chisq = 0.0;

	#define AX(i,j)	Ax[(i)*DIM+(j)]

	for (j = 0; j < DIM; j++) {
		for (k = 0; k <= j; k++)
			AX(j, k) = 0.0;
		Gv[j] = 0.0;
	}

	for (i = 0; i < N; i++, Bx += DIM) {
		dtype w2i = Wn[i];						// sig2i=noise_var/(sig[i]^2)
		dtype dy = Si[i] - fx_eval(Bx, Di, Jv);	// dy=y[i]-f(x[i]+B[i]), Jv=df(x)/dB
		for (j = 0; j < DIM; j++) {
			dtype wt = Jv[j] * w2i;
			for (k = 0; k <= j; k++)
				AX(j, k) += wt * Jv[k];
			Gv[j] += dy * wt;
		}
		chisq += dy * dy * w2i;
	}

	// Copy to upper-right
	for (j = 1; j < DIM; j++)
		for (k = 0; k < j; k++)
			AX(k, j) = AX(j, k);

	#undef AX

	return chisq;
}
//---------------------------------------------------------------------------
#define LAMBDA_SMALL_STEP	0.1
#define LAMBDA_BIG_STEP		10.0
//---------------------------------------------------------------------------
#define LAMBDA_MIN			1.0E-15
#define LAMBDA_MAX			1.0E+12
//---------------------------------------------------------------------------
#define lma_buffer(n)		&buffer[(n)*DIM]
//---------------------------------------------------------------------------
// lma_update
//---------------------------------------------------------------------------
DFAPI
void lma_update(
		int N,
		dtype *Bx,			// size = NxDIM
		dtype *Si,			// size = N
		dtype *Wn,			// size = N
		dtype *Dn,			// size = DIM
		dtype *buffer,		// size = 4 DIM + 2 DIMxDIM
		dtype &lambda,
		dtype &chisq,
		bri_t &br
		)
{
	dtype ochisq;
	dtype *Dt    = lma_buffer(0);		// size = DIM
	dtype *Jv    = lma_buffer(1);		// size = DIM -- Jacobian
	dtype *Gv    = lma_buffer(2);		// size = DIM -- Gradient
	dtype *delta = lma_buffer(3);		// size = DIM
	dtype *Hx    = lma_buffer(4);		// size = DIMxDIM -- Hessian
	dtype *Ax    = &Hx[DIM*DIM];		// size = DIMxDIM

	if (lambda < 0.0) {
		lambda = 0.001;
		chisq = lma_eval(N, Bx, Si, Dn, Wn, Jv, Ax, Gv);
	#ifdef USE_LMA_BR
		br.w = br.calc_nw(Dn);
		br.c = br.calc_cost(chisq);
	#endif
	}

	lma_memcpy(Hx, Ax, DIM*DIM);
	for (int k = 0; k < DIM; k++) {
	#ifdef USE_LMA_BR
		Hx[k*DIM+k] += (lambda + br.a);
	#else
		Hx[k*DIM+k] *= (lambda + 1.0);
	#endif
		delta[k] = Gv[k];
	}

	if (!qr_solve(Hx, delta)) {
		lambda *= LAMBDA_BIG_STEP;
		return;
	}

	for (int k = 0; k < DIM; k++)
		Dt[k] = Dn[k] + delta[k];

#ifdef USE_LMA_BR
	dtype tr = br.calc_tr(Hx);
	dtype nw = br.calc_nw(delta);
#endif

	ochisq = chisq;
	chisq = lma_eval(N, Bx, Si, Dt, Wn, Jv, Hx, delta);
#ifdef USE_LMA_BR
	dtype nc = br.calc_cost(chisq, nw);
	if (nc > br.c) {
		lambda *= LAMBDA_BIG_STEP;
		chisq = ochisq;
	}
	else {
		br.c = nc;
		br.w = nw;
		br.update(N, chisq, tr);
		lambda *= LAMBDA_SMALL_STEP;
		lma_memcpy(Ax, Hx, DIM*DIM);
		lma_memcpy(Gv, delta, DIM);
		lma_memcpy(Dn, Dt, DIM);
	}
#else
	if (isfinite(chisq) && chisq < ochisq) {
		lambda *= LAMBDA_SMALL_STEP;
		lma_memcpy(Ax, Hx, DIM*DIM);
		lma_memcpy(Gv, delta, DIM);
		lma_memcpy(Dn, Dt, DIM);
	} else {
		lambda *= LAMBDA_BIG_STEP;
		chisq = ochisq;
	}
#endif
}
//---------------------------------------------------------------------------
#define MAX_ITERATION		200
//---------------------------------------------------------------------------
// Levenberg–Marquardt Algorithm non-linear regression
//---------------------------------------------------------------------------
DXAPI
void lma_fit(
		int N,
		dtype *Bx,		// size = NxDIM
		dtype *Si,		// size = N
		dtype *Wn,		// size = N
		dtype *Dn,		// size = DIM	-- solution
		dtype *buffer	// size = 6 DIM + 2 DIMxDIM
		)
{
	bri_t br = {0.0, 1.0, 0.0, 0.0, lma_buffer(0)};
	dtype chisq = 1.0e-20, ochisq = 1.0e-20;
	dtype lambda = -1.0;

	dtype *Do = lma_buffer(1);	// size = DIM
	buffer = lma_buffer(2);		// size = 4 DIM + 2 DIMxDIM

	lma_update(N, Bx, Si, Wn, Dn, buffer, lambda, chisq, br);

	for (int i = 0; i < MAX_ITERATION &&
					lambda > LAMBDA_MIN && lambda < LAMBDA_MAX; i++) {
		if (chisq != ochisq) {
			// Do = Dn -- save a backup version of current Dx
			lma_memcpy(Do, Dn, DIM);
			ochisq = chisq;
		}
		lma_update(N, Bx, Si, Wn, Dn, buffer, lambda, chisq, br);
	}

	for (int i = 0; i < DIM; i++) {
		if (!isfinite(Dn[i])) {
			// Dn = Do -- restore the best Dx if the final Dx is unstable
			lma_memcpy(Dn, Do, DIM);
			break;
		}
	}
	buffer[0] = chisq;
}
//---------------------------------------------------------------------------
