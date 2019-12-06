#pragma OPENCL EXTENSION cl_intel_printf : enable
__kernel void floyd(__global int *mat, __local int *intermediate) {
    int idx = get_global_id(0);
    int n = get_global_size(0);
    int i = idx / n;
    int j = idx % n;
    int k;
    for (k = 0; k < n; k++) {
        *intermediate = (mat[idx]) < (mat[i * n + k] + mat[k * n + j])
                           ? (mat[idx]) : (mat[i * n + k] + mat[k * n + j]);
    }
}

__kernel void writeElem(__global int *mat, __local int *intermediate) {
    int idx = get_global_id(0);
    mat[idx] = *intermediate;
}