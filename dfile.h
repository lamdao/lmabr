#ifndef __LMA_DFILE_H
#define __LMA_DFILE_H
//------------------------------------------------------------------------
#include <cstdio>
#include <vector>
//------------------------------------------------------------------------
// Data file control
//------------------------------------------------------------------------
class dfile
{
	FILE *fp;
public:
	dfile(const char *name, bool readonly = true) : fp(0) {
		if (readonly)
			fp = fopen(name, "rb");
		else
			fp = fopen(name, "w+b");
	}
	~dfile() {
		if (fp) fclose(fp);
	}

	operator bool () { return fp != 0; }

	template<typename T>
	T get() {
		T v;
		fread(&v, sizeof(v), 1, fp);
		return v;
	}

	template<typename T>
	std::vector<T> get(int n) {
		std::vector<T> v(n);
		fread(v.data(), sizeof(T), n, fp);
		return v;
	}

	template<class T>
	void put(const T &v) {
		fwrite(&v, sizeof(T), 1, fp);
	}

	template<class T>
	void put(const std::vector<T> &v) {
		fwrite(v.data(), sizeof(T), v.size(), fp);
	}
};
//------------------------------------------------------------------------
#endif
