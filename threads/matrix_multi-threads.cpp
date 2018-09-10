#include <bits/stdc++.h>
#include <chrono>
#include <tbb/tbb.h>

using namespace std;
//Matrix sizes
const int MX = 2000;
const int N = 5;


//all the matrix
long int **matrix1, **matrix2, **matrix;

void val(){
	long int i, j;
	for(i=0; i<MX; i++){
		for(j=0; j<MX; j++){
			matrix1[i][j] = 4;
			matrix2[i][j] = 5;
			matrix[i][j] = 0;
		}
	}	
}

void multiply(int l , int r){
		
	for(int i = l; i <= r; i++ )
		for(long int j=0; j<MX; j++){
			for(long int k=0; k<MX; k++){
				matrix[i][j] += (matrix1[i][k] * matrix2[k][j]);
			}
		}
	return;
}


void static_partitioning(){

	int len = MX / N;

	vector<thread> ths;

	for( int i = 0; i < N; i++ ){
		ths.push_back(thread(multiply, i * len, min(MX - 1 , (i+1) * len - 1)));
	}

	for(auto &t: ths){
		t.join();
	}

}


queue<int> Q;
mutex m;

void multiply_d(){

	while(true){

		m.lock();

		if(Q.size() == 0){
			m.unlock();
			return;
		}

		int i = Q.front();

		Q.pop();

		m.unlock();

		for(long int j=0; j<MX; j++)
			for(long int k=0; k<MX; k++){
				matrix[i][j] += (matrix1[i][k] * matrix2[k][j]);
			}
	}
}


void dynamic_partitioning(){

	for(int i = 0; i < MX; i++ ){
		Q.push(i);
	}

	vector<thread> ths;

	for( int i = 0; i < N; i++ ){
		ths.push_back(thread(multiply_d));
	}

	for(auto &t: ths){
		t.join();
	}	
}


int main(int argc, char const *argv[]){

	double ttime = clock();
	

	matrix = new long int*[MX];  
	matrix1 = new long int*[MX];
	matrix2 = new long int*[MX];

	for (long int i=0; i < MX; i++){
	    matrix[i] = new long int[MX];
	    matrix1[i] = new long int[MX];
	    matrix2[i] = new long int[MX];
	}

	val();

	// static_partitioning();
	dynamic_partitioning();

	cout << "TTIME: " << (clock() - ttime) / CLOCKS_PER_SEC << endl;
}
