#include <tbb/task_scheduler_init.h>
#include "tbb/pipeline.h"

#include <bits/stdc++.h>
#include <fstream>
#include <chrono>


#define N (3204)

#define GN (N*10)

long long total = 0;

using namespace std;

ifstream stream_inA;
ifstream stream_inB;

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

struct task_t
{
	vector<int> a, b;
	task_t(vector<int> a, vector<int> b):a(a), b(b) {}
	
};


struct task_n
{
	task_n(long long n):n(n) {}
	long long n;
};

class stage1: public tbb::filter{
public:		
	stage1():tbb::filter(tbb::filter::serial_in_order) {}
	void * operator()(void *){		
	
		while(!stream_inB.eof()){
			vector<int> a(N);
			vector<int> b(N);

			for (int i = 0; i < N; ++i){
				stream_inA >> a[i];
				stream_inB >> b[i];
			}
			task_t *task = new task_t(a , b);
  			return task;
		}

		stream_inA.close();
		stream_inB.close();
		return NULL;
	}
};

class stage2 : public tbb::filter{
public:
	stage2() : tbb::filter(tbb::filter::serial_in_order) {}
	void* operator()(void *item){
		task_t *task = static_cast <task_t*> (item);
		long long sum = 0;
		for (int i = 0; i < N; ++i){
			sum += task->a[i] * task->b[i];
		}
		task_n *taskr = new task_n(sum);
		return taskr;
	}
};

class stage3 : public tbb::filter{
public:
	stage3() : tbb::filter(tbb::filter::serial_in_order){}
	void* operator()(void* item){
		task_n *task = static_cast <task_n*> (item);
		total += task->n;

		delete task;
		return NULL;
	}
};


void parallel(int cores){
	tbb::task_scheduler_init init(cores);
	tbb::pipeline pipeline;

	stage1 S1;
	pipeline.add_filter(S1);

	stage2 S2;
	pipeline.add_filter(S2);

	stage3 S3;
	pipeline.add_filter(S3);
	pipeline.run(cores * cores);
	pipeline.clear();
}

int main(int argc, char const *argv[]){

	stream_inA.open("inputA.txt",std::ios::in);
	if (stream_inA.fail()){
		std::cerr << "Error in: " << "inputA.txt"  << std::endl;
		stream_inA.close();
		return 0;
	}

	stream_inB.open("inputB.txt",std::ios::in);
	if (stream_inB.fail()){
		std::cerr << "Error in: " << "inputB.txt"  << std::endl;
		stream_inB.close();
		return 0;
	}
    
	//Uncomment to generate input ;)

    // auto t_s = std::chrono::high_resolution_clock::now();
	// gen_input();
	// auto t_e = std::chrono::high_resolution_clock::now();
	// std::cout << "GenerateInputs Execution time(s): " << std::chrono::duration<double>(t_e-t_s).count() << std::endl;
	
	// auto t_start = std::chrono::high_resolution_clock::now();
	// dp();
	// auto t_end = std::chrono::high_resolution_clock::now();
	// std::cout << "DotProduct Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;

	parallel(5);

	cout << total << endl;


	return 0;
}