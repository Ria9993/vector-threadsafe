#pragma once

#include <atomic>

#ifndef CACHE_LINE
#define CACHE_LINE (64)
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
		const size_t size = mSize;
		if (mCapacity <= size) {
			if (mCapacity == 0)
				mCapacity = 16;
			else
				mCapacity = (size_t)(mCapacity * 1.5);

			void* newData = new char[mCapacity * sizeof(T)];
			memcpy(newData, mData, size * sizeof(T));
			while (mModifyLock.test_and_set());
			{
				delete[] mData;
				mData = (T*)newData;
			}
			mModifyLock.clear();
		}

		while (mSizeLock.test_and_set());
		{
			mData[mSize] = src;
			mSize += 1;
		}
		mSizeLock.clear();
	}

	inline void clear() {
		while (mSizeLock.test_and_set());
		{
			mSize = 0;
		}
		mSizeLock.clear();
	}

	inline void clear_except_last_n(size_t n) {
		while (mSizeLock.test_and_set());
		{
			if (mSize <= n)
				return;

			for (size_t i = 0; i < n; i++) {
				mData[i] = mData[mSize - n + i];
			}

			mSize = n;
		}
		mSizeLock.clear();
	}

private:
	volatile size_t mSize;
	volatile size_t mCapacity;
	T* volatile mData;
	volatile alignas(CACHE_LINE) std::atomic_flag mModifyLock;
	volatile alignas(CACHE_LINE) std::atomic_flag mSizeLock;
};
