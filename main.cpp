#include <cstdio>
#include "ts_vector.h"

int main()
{
	ts_vector<int> v;
	for (int i = 0; i < 1000000; i++)
	{
		v.push_back((float)i);
	}

	for (int i = 0; i < 1000000; i++)
	{
		printf("%d\n", v[i]);
	}
	return 0;
}