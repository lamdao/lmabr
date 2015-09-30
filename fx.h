#define FS0(x)	(x)[0]
#define FXX(x)	(x)[1]
#define FXY(x)	(x)[2]
#define FXZ(x)	(x)[3]
#define FYY(x)	(x)[4]
#define FYZ(x)	(x)[5]
#define FZZ(x)	(x)[6]
//---------------------------------------------------------------------------
#define S0		FS0(D)
#define Dxx		FXX(D)
#define Dyy		FYY(D)
#define Dzz		FZZ(D)
#define Dxy		FXY(D)
#define Dyz		FYZ(D)
#define Dxz		FXZ(D)
//---------------------------------------------------------------------------
#define Bxx		FXX(b)
#define Byy		FYY(b)
#define Bzz		FZZ(b)
#define Bxy		FXY(b)
#define Bxz		FXZ(b)
#define Byz		FYZ(b)
//---------------------------------------------------------------------------
// Evaluate value of F(x) = S0 e^(-bD)
//---------------------------------------------------------------------------
DFAPI
dtype fx_eval(
		dtype *b,		// size = DIM	-- row i-th of B matrix
		dtype *D,		// size = DIM	-- DTI signal
		dtype *J		// size = DIM	-- dy/dD  (Jacobian)
		)
{
	dtype bD = Bxx*Dxx + Byy*Dyy + Bzz*Dzz + Bxy*Dxy + Byz*Dyz + Bxz*Dxz;
	dtype e = exp(-bD);
	dtype y = S0 * e;

	FS0(J) = e;
	FXX(J) = -Bxx * y;
	FYY(J) = -Byy * y;
	FZZ(J) = -Bzz * y;
	FXY(J) = -Bxy * y;
	FXZ(J) = -Bxz * y;
	FYZ(J) = -Byz * y;

#ifdef LMA_DEBUG
	if (isnan(e)) {
		printf("e[nan], bD = %e\n", bD);
	}
	else if (!finite(e)) {
		printf("e[inf], bD = %e\n", bD);
	}
#endif
	return y;
}
//---------------------------------------------------------------------------
#undef FS0
#undef FXX
#undef FXY
#undef FXZ
#undef FYY
#undef FYZ
#undef FZZ
//---------------------------------------------------------------------------
#undef S0
#undef Dxx
#undef Dyy
#undef Dzz
#undef Dxy
#undef Dyz
#undef Dxz
//---------------------------------------------------------------------------
#undef Bxx
#undef Byy
#undef Bzz
#undef Bxy
#undef Bxz
#undef Byz
