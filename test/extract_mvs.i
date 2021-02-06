%module extract_mvs

%{
#define SWIG_FILE_WITH_INIT
#include "extract_mvs.h"
%}

%include "typemaps.i"
%include "numpy.i"

%init %{
import_array();
%}

%apply (int **ARGOUTVIEWM_ARRAY4, int *DIM1, int *DIM2, int *DIM3, int *DIM4) {(int **out, int *out_dim1, int *out_dim2, int *out_dim3, int *out_dim4)}
%apply (int **ARGOUTVIEWM_ARRAY4, int *DIM1, int *DIM2, int *DIM3, int *DIM4) {(int **out_source, int *out_source_dim1, int *out_source_dim2, int *out_source_dim3, int *out_source_dim4)}


%apply (int **ARGOUTVIEWM_ARRAY3, int *DIM1, int *DIM2, int *DIM3) {(int **out, int *out_dim1, int *out_dim2, int *out_dim3)}
%apply (int **ARGOUTVIEWM_ARRAY3, int *DIM1, int *DIM2, int *DIM3) {(int **out_source, int *out_source_dim1, int *out_source_dim2, int *out_source_dim3)}

%apply int *OUTPUT {int *result};

%include "extract_mvs.h"