//--------------------------------------------------------------------------
// dfile - Data file management
//--------------------------------------------------------------------------
// Author: Lam H. Dao <daohailam(at)yahoo(dot)com>
//--------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//--------------------------------------------------------------------------
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

	template<typename T>
	void put(const T &v) {
		fwrite(&v, sizeof(T), 1, fp);
	}

	template<typename T>
	void put(const std::vector<T> &v) {
		fwrite(v.data(), sizeof(T), v.size(), fp);
	}
};
//------------------------------------------------------------------------
#endif
