__kernel void simpleMultiply(

int elements,
__global float* inputA,
__global float* inputB,
__global float* outputC) {

int row = get_global_id(0);

int col = get_global_id(1);

float sum = 0.0;
int i=0;

for (i=0; i<elements; i++) {sum = sum + inputA[row*elements+i]*inputB[i*elements+col];}

outputC[row*elements+col]=sum;

}