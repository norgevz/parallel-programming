/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 *
 ****************************************************************************
 *  Authors: Norge Vizcay <norge4522@gmail.com>
 *         
 *  Copyright: GNU General Public License
 *  Description: This program generates two files (inputA.txt and inputB.txt) that will
 *  be the input stream to apply dot product over fixed vector size and write the results in
 *  a output file (output.txt)
 *  Compilation Command: g++ -std=c++1y dot_product_stream.cpp -o exe
 ****************************************************************************
*/

#include <iostream>
#include <fstream>
#include <chrono>
#include <tbb/tbb.h>

#define N (3204)

#define GN (N*10)


void gen_input(){

	std::ofstream A_out;
	A_out.open("inputA.txt",std::ios::out);
	if (A_out.fail()){
		std::cerr << "Error in: " << "inputA.txt"  << std::endl;
		A_out.close();
		return;
	}
	std::ofstream B_out;
	B_out.open("inputB.txt",std::ios::out);
	if (B_out.fail()){
		std::cerr << "Error in: " << "inputB.txt"  << std::endl;
		B_out.close();
		return;
	}
	for (int a = 0; a<GN; a++){
		for (int i = 0; i < N; ++i){
			A_out << " ";
			A_out << i+1;
			B_out << " ";
			B_out << i+1;
		}
	}
	A_out.close();
	B_out.close();
}
std::ofstream stream_out;
std::ifstream stream_inA;
std::ifstream stream_inB;
size_t size = N * sizeof(int);
void dp(){

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

	stream_out.open("output1.txt",std::ios::out);
	if (stream_out.fail()){
		std::cerr << "Error in: " << "output1.txt"  << std::endl;
		stream_out.close();
		return;
	}

	while(!stream_inB.eof()){
		int *A = new int[size];
  		int *B = new int[size];
		for (int i = 0; i < N; ++i){
			stream_inA >> A[i];
			stream_inB >> B[i];
		}
		int C=0;
		for (int i = 0; i < N; ++i){
			C += A[i] * B[i];
		}
		delete A;
		delete B;
		stream_out << C;
		stream_out << "\n";
	}
	stream_inA.close();
	stream_inB.close();
	stream_out.close();
}

int A[GN][N];
int B[GN][N];

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

int outp[GN];

int main(int argc, char const *argv[]){
	// auto t_s = std::chrono::high_resolution_clock::now();
	// gen_input();
	// auto t_e = std::chrono::high_resolution_clock::now();
	// std::cout << "GenerateInputs Execution time(s): " << std::chrono::duration<double>(t_e-t_s).count() << std::endl;
	
	auto t_start = std::chrono::high_resolution_clock::now();
	dp();
	auto t_end = std::chrono::high_resolution_clock::now();
	std::cout << "DotProduct Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;


	t_start = std::chrono::high_resolution_clock::now();
	read();
	t_end = std::chrono::high_resolution_clock::now();
	std::cout << "Read Input Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;


	tbb::task_scheduler_init init(4);

	tbb::parallel_for(0, GN, [&](int i){
		int s = 0;
		for(int j = 0; j < N; j++ ){
			s += A[i][j] * B[i][j];
		}
		outp[i] = s;
		
	});

	t_end = std::chrono::high_resolution_clock::now();
	std::cout << "Computation Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;

	stream_out.open("output.txt",std::ios::out);
	if (stream_out.fail()){
		std::cerr << "Error in: " << "output.txt"  << std::endl;
		stream_out.close();
	}

	for( int i = 0; i < GN; i++ ){
		stream_out << outp[i] << "\n";
	}

	stream_out.close();


	return 0;
}
