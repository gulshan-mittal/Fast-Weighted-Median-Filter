#include "allocint.hpp"

int** int2D(int dim1, int dim2){
	int **ret = new int*[dim1];
	ret[0] = new int[dim1*dim2];
	for(int i=1;i<dim1;i++)
		ret[i] = *ret + dim2*i;
	return ret;
}