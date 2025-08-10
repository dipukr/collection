#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
struct str {char *data; int length;};
struct array_bool {bool *data; int length;};
struct array_int {int *data; int length;};
struct array_num {double *data; int length;};
struct array_str {struct str *data; int length;};

void main(int argc, const char **argv){struct array_int *data = (struct array_int*) malloc(sizeof(struct array_int));data->length=7;data->data=(int*)malloc(7*sizeof(int));int __data__[]={10, 20, 30, 40, 50, 60, 70};for(int __i=0;__i<data->length;__i++)data->data[__i]=__data__[__i];struct array_bool *marked = (struct array_bool*) malloc(sizeof(struct array_bool));marked->length=data->length;marked->data=(bool*)malloc(data->length*sizeof(bool));for(int __i=0;__i<marked->length;__i++)marked->data[__i]=false;int iter=0;while (iter<data->length){printf("%d\n",data->data[iter]);iter+=1;} } 