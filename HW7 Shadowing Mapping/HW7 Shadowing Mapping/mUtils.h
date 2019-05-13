#ifndef _UTILS_H_
#define _UTILS_H_

namespace mUtils {
#include <vector>

	using std::vector;
	template<typename T>
	T& max(T& a, T& b) {
		return a > b ? a : b;
	}

	template<typename T>
	T& max(T& a, T& b, T& c) {
		return max(max(a, b), c);
	}

	template<typename T>
	T& max(vector<T>& arr) {
		int m = 0;
		for (int i = 0; i < arr.size(); i++)
			if (arr[i] > arr[m]) m = i;
		return arr[m];
	}

	template<typename T>
	T& min(T& a, T& b) {
		return a < b ? a : b;
	}

	template<typename T>
	T& min(T& a, T& b, T& c) {
		return min(min(a, b), c);
	}

	template<typename T>
	T& min(vector<T>& arr) {
		int m = 0;
		for (int i = 0; i < arr.size(); i++)
			if (arr[i] < arr[m]) m = i;
		return arr[m];
	}

	template<typename T>
	void swap2(T& x, T& y) {
		T t = x;
		x = y;
		y = t;
	}
}

#endif