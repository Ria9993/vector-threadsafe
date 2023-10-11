#pragma once

#include <atomic>

#ifndef CACHE_LINE
#define CACHE_LINE (16)
#endif

template <typename T>
class ts_vector {
public:
	ts_vector()
		: mSize(0)
		, mCapacity(0)
		, mData(nullptr)
	{
	}
	~ts_vector() {
		delete[] mData;
	}

	inline T operator[](size_t idx) {
		T ret;
		while (mModifyLock.test_and_set());
		{
			ret = mData[idx];
		}
		mModifyLock.clear();
		return ret;
	}

	inline volatile size_t size() {
		return mSize;
	}

	inline void copy_to(void* dest, size_t startIdx, size_t num) {
		while (mModifyLock.test_and_set());
		{
			memcpy(dest, mData + startIdx, num * sizeof(T));
		}
		mModifyLock.clear();
	}

	inline void push_back(T src) {
		if (mCapacity <= mSize) {
			if (mCapacity == 0)
				mCapacity = 16;
			else
				mCapacity = (size_t)(mCapacity * 1.5);

			void* newData = new char[mCapacity * sizeof(T)];
			memcpy(newData, mData, mSize * sizeof(T));
			while (mModifyLock.test_and_set());
			{
				delete[] mData;
				mData = (T*)newData;
			}
			mModifyLock.clear();
		}

		mData[mSize] = src;
		mSize += 1;
	}

private:
	volatile size_t mSize;
	volatile size_t mCapacity;
	T* volatile mData;
	volatile alignas(CACHE_LINE) std::atomic_flag mModifyLock;
};