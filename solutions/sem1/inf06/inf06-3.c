#include <stdio.h>
#include <immintrin.h>
#include <stdlib.h>


int main(){
	int N, M;
	scanf("%d %d", &N, &M);
	int real_M = M/4*4+4;
	float input;
	float* A = (float*)aligned_alloc(16, sizeof(float)*N*real_M);
	float* B = (float*)aligned_alloc(16, sizeof(float)*N*real_M);
	float* R = (float*)aligned_alloc(16, sizeof(float)*N*real_M);


	for (size_t i = 0; i < N; ++i) {
		for (size_t j = 0; j < real_M; ++j) {
			if (j >= M) {
				*(A+(i*real_M+j)) = 0.;
				continue;
			}
			scanf("%f", &input);
			*(A+(i*real_M+j)) = input;
		}
	}

	size_t count = 0;
	for(size_t i = 0; i < real_M; ++i) {
		for (size_t j = 0; j < N; ++j ){
			if (++count > N*M) {
				*(B+(i+j*real_M)) = 0.;
				continue;
			}
			scanf("%f", &input);
			*(B+(i+j*real_M)) = input;
		}
	}


	__m128 v1, v2;
	float buff[4];

	for (size_t i = 0; i<N; ++i) {
		for(size_t j = 0; j<N; ++j) {
			for (size_t k = 0; k < real_M; k+=4) {
				v1 = _mm_load_ps(&A[i*real_M + k]);
				v2 = _mm_load_ps(&B[j*real_M + k]);
				v2  = _mm_dp_ps(v1, v2, 0xFF);
				_mm_store_ps(buff, v2);
				*(R+(i*N+j)) += buff[0];
			}
		}
	}
	for (size_t i =0; i<N*N; ++i) {
		printf("%.4f ", *(R+i));
		if ((i+1)%N == 0)
			printf("\n");
	}

	free(A); free(B); free(R);
	return 0;
}