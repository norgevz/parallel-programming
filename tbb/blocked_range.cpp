#include <tbb/task_scheduler_init.h>
#include "tbb/pipeline.h"
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"

#include <bits/stdc++.h>
#include <fstream>
#include <chrono>


#define N (3204)

#define GN (N*10)

long long total = 0;

using namespace std;

ifstream stream_inA;
ifstream stream_inB;


long long A[GN][N];
long long B[GN][N];

void read(){

	stream_inA.open("inputA.txt",std::ios::in);
	if (stream_inA.fail()){
		std::cerr << "Error in: " << "inputA.txt"  << std::endl;
		stream_inA.close();
		return;
	}

	stream_inB.open("inputB.txt",std::ios::in);
	if (stream_inB.fail()){
		std::cerr << "Error in: " << "inputB.txt"  << std::endl;
		stream_inB.close();
		return;
	}

	int it = 0;
	while(!stream_inB.eof()){
		
		for (int i = 0; i < N; ++i){
			stream_inA >> A[it][i];
			stream_inB >> B[it][i];
		}
		it++;
	}
	stream_inA.close();
	stream_inB.close();
	
}

int main(int argc, char const *argv[]){

	
    read();

    tbb::task_scheduler_init init(5);

	auto bodyF = [&]( tbb::blocked_range<int> &r, int in) -> long long  {
		long long result = 0;
		for(int i = r.begin(); i < r.end(); ++i){
			long long  s = 0;
			for(int j = 0; j < N; j++ ){
				s += A[i][j] * B[i][j];
			}
			result += s;
		}
		return result; 
	};

	long long result = tbb::parallel_reduce(tbb::blocked_range<int>(0, N, 100 ),0LL,
	bodyF, std::plus<long long>());

	cout << result << endl;

	return 0;
}
