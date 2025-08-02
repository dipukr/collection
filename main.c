#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonstd.h>
#define N 10000

void main(const int argc, const char **argv)
{
	register int v = 10;
	int *p = &v;
	printf("%d\n", v);
}

void merge(int[] data, int[] temp, int low, int mid, int high)
{
	int i = low;
	int j = mid + 1;
	int k = low;
	while (i <= mid || j <= high) {
		if (data[i] < data[j]) {
			temp[k++] = data[i++];
		} else {
			temp[k++] = data[j++];
		}
	}   
}

