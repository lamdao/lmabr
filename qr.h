//------------------------------------------------------------------------
// This is a porting version of QR solver from Jama library.
// The full credit belongs to Jama's developers.
//------------------------------------------------------------------------
DFAPI
void qr_init(
		dtype *qr,		// size = DIMxDIM
		dtype *Rdv		// size = DIM
		)
{
	#define QR(i,j)	qr[((i)*DIM+(j))]

	for (int k = 0; k < DIM; k++) {
		dtype &qrkk = QR(k,k);
		dtype nrm = 0.0;
		for (int i = k; i < DIM; i++) {
			dtype d = QR(i,k);
			nrm += d * d;
		}
		nrm = sqrt(nrm);

		if (nrm != 0.0) {
			if (qrkk < 0.0) {
				nrm = -nrm;
			}
			for (int i = k; i < DIM; i++) {
				QR(i,k) /= nrm;
			}
			qrkk += 1.0;

			for (int j = k + 1; j < DIM; j++) {
				dtype s = 0.0;
				for (int i = k; i < DIM; i++) {
					s += QR(i,k) * QR(i,j);
				}
				s = -s / QR(k,k);
				for (int i = k; i < DIM; i++) {
					QR(i,j) += s * QR(i,k);
				}
			}
		}
		Rdv[k] = -nrm;
	}
	#undef QR
}
//------------------------------------------------------------------------
DFAPI
bool qr_solve(
		dtype *qr,		// size = DIMxDIM
		dtype *X		// size = DIM
		)
{
	#define QR(i,j)	qr[((i)*DIM+(j))]

	dtype Rdv[DIM];
	qr_init(qr, Rdv);
	for (int k = 0; k < DIM; k++) {
		if (Rdv[k] == 0.0) {
			return false;
		}
	}

	for (int k = 0; k < DIM; k++) {
		dtype s = 0.0;
		for (int i = k; i < DIM; i++) {
			s += QR(i,k) * X[i];
		}
		s = -s / QR(k,k);
		for (int i = k; i < DIM; i++) {
			X[i] += s * QR(i,k);
		}
	}

	for (int k = DIM - 1; k >= 0; k--) {
		X[k] /= Rdv[k];

		for (int i = 0; i < k; i++) {
			X[i] -= X[k] * QR(i,k);
		}
	}
	#undef QR

	return true;
}
//------------------------------------------------------------------------
