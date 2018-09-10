#include <tbb/task_scheduler_init.h>
#include "tbb/pipeline.h"

#include <iostream>
#include <fstream>
#include <chrono>

struct task_t
{
	task_t(int n):n(n) {}
	int n;
};

class stage1: public tbb::filter{
public:		
	stage1():tbb::filter(tbb::filter::serial) {}
	void * operator()(void *){		
		
        while(true){       
  			task_t *task = new task_t(1);
  			return task;
        }
		return NULL;
	}
};
class stage2 : public tbb::filter{
public:
	stage2() : tbb::filter(tbb::filter::parallel) {}
	void* operator()(void *item){
		task_t *task = static_cast <task_t*> (item);
		task->n++;
		return task;
	}
};

class stage3 : public tbb::filter{
public:
	stage3() : tbb::filter(tbb::filter::serial){}
	void* operator()(void* item){
		task_t *task = static_cast <task_t*> (item);
		std::cout << task->n << std::endl;
		delete task;
		return NULL;
	}
};
void parallel(){
	tbb::task_scheduler_init init(4);
	tbb::pipeline pipeline;
	stage1 S1;
	pipeline.add_filter(S1);
	stage2 S2;
	pipeline.add_filter(S2);
	stage3 S3;
	pipeline.add_filter(S3);
	pipeline.run(4);
	pipeline.clear();
}

int main(int argc, char const *argv[]){
    parallel();
	return 0;
}