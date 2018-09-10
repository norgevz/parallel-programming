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
 */

/* 

   Author: Marco Aldinucci.   
   email:  aldinuc@di.unipi.it
   marco@pisa.quadrics.com
   date :  15/11/97


/*Modified by:

****************************************************************************
 *  Author: Norge Vizcay <norge4522@gmail.com>
 *         
 *  Copyright: GNU General Public License
 *  Description: This program simply computes the mandelbroat set. 
 *  File Name: mandel.cpp
 *  Version: 1.0 (1/06/2018)
 *  Compilation Command: make
 ****************************************************************************
*/

#include <stdio.h>
#include "marX2.h"
#include <sys/time.h>
#include <math.h>

#include <iostream>
#include <chrono>

#include <bits/stdc++.h>

using namespace std;

const int THREADS = 6;

const int DIM = 800;
const int ITERATION = 1024;

int dim = DIM, niter = ITERATION;
double init_a = -2.125, init_b = -1.5, range = 3.0;
double step;

double diffmsec(struct timeval a, struct timeval b)
{
  long sec = (a.tv_sec - b.tv_sec);
  long usec = (a.tv_usec - b.tv_usec);

  if (usec < 0)
  {
    --sec;
    usec += 1000000;
  }
  return ((double)(sec * 1000) + (double)usec / 1000.0);
}

template <typename T>
class m_queue
{
private:
  std::mutex d_mutex;
  std::condition_variable d_condition;
  std::deque<T> d_queue;
  bool qend;

public:
  m_queue() : qend(false) {}
  void nomorewriters()
  {
    qend = true;
    this->d_condition.notify_all();
    return;
  }

  void push(T const &value)
  {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }

  T pop()
  {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=] { return !this->d_queue.empty(); });
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
    return rc;
  }
};

struct input
{
  double step, init_a, init_b, i;

  input(double step, double init_a, double init_b, int i) : step(step), init_a(init_a), init_b(init_b), i(i)
  {
  }
};

struct output
{
  unsigned char *M;
  int i;

  output(unsigned char *M, int i) : M(M), i(i)
  {
  }
};

m_queue<input *> input_q;
m_queue<output *> output_q;

void emmit()
{
  for (int i = 0; i < dim; i++)
  {
    input_q.push(new input(step, init_a, init_b, i));
  }

  for (int i = 0; i < THREADS; i++)
  {
    input_q.push(NULL);
  }
}

int n_nulls = 0;
mutex mut;

void worker()
{

  input *data;

  while ((data = input_q.pop()) != NULL)
  {
    unsigned char *M = (unsigned char *)malloc(dim);

    double im = data->init_b + (data->step * data->i);

    for (int j = 0; j < dim; j++)
    {
      double a, cr, b;
      a = cr = data->init_a + step * j;
      b = im;
      int k = 0;

      for (; k < niter; k++)
      {
        double a2 = a * a;
        double b2 = b * b;

        if ((a2 + b2) > 4.0)
          break;
        b = 2 * a * b + im;
        a = a2 - b2 + cr;
      }
      M[j] = (unsigned char)255 - ((k * 255 / niter));
    }
    output_q.push(new output(M, data->i));
  }

  output_q.push(NULL);
}

int n_nulls_collector = 0;
void collector()
{

  output *data;

  while ((data = output_q.pop()) != NULL)
  {

#if !defined(NO_DISPLAY)
    ShowLine(data->M, dim, data->i);
#endif
  }
}

// stats
struct timeval t1, t2;
int retries = 1;
double avg = 0, var, *runs;

int main(int argc, char **argv)
{

  if (argc < 3)
  {
    printf("Usage: seq size niterations\n\n\n");
  }
  else
  {
    dim = atoi(argv[1]);
    niter = atoi(argv[2]);
    step = range / ((double)dim);
    //retries = atoi(argv[3]);
  }
  runs = (double *)malloc(retries * sizeof(double));

  printf("Mandebroot set from (%g+I %g) to (%g+I %g)\n",
         init_a, init_b, init_a + range, init_b + range);
  printf("resolution %d pixel, Max. n. of iterations %d\n", dim * dim, ITERATION);

  step = range / ((double)dim);

#if !defined(NO_DISPLAY)
  SetupXWindows(dim, dim, 1, NULL, "Sequential Mandelbroot");
#endif

  for (int r = 0; r < retries; r++)
  {

    // Start time
    gettimeofday(&t1, NULL);

    vector<thread> th2;

    th2.push_back(thread(emmit));

    for (int i = 0; i < THREADS; i++)
    {
      th2.push_back(thread(worker));
    }

    th2.push_back(thread(collector));

    gettimeofday(&t2, NULL);

    // cerr << "ttime --> " << diffmsec(t2,t1) / 1000 << endl;
    for (auto &t : th2)
      t.join();

    // Stop time
    gettimeofday(&t2, NULL);
    cerr << "ttime --> " << diffmsec(t2, t1) / 1000 << endl;

    avg += runs[r] = diffmsec(t2, t1);
    printf("Run [%d] DONE, time= %f (ms)\n", r, runs[r]);
  }

  avg = avg / (double)retries;
  var = 0;

  for (int r = 0; r < retries; r++)
  {
    var += (runs[r] - avg) * (runs[r] - avg);
  }

  var /= retries;
  printf("Average on %d experiments = %f (ms) Std. Dev. %f\n\nPress a key\n", retries, avg, sqrt(avg));

#if !defined(NO_DISPLAY)
  getchar();
  CloseXWindows();
#endif

  return 0;
}
