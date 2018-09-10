#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <tbb/task_scheduler_init.h>
#include "tbb/pipeline.h"
#include <chrono>


#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

// Global variables:
string face_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
VideoCapture capture;
VideoWriter oVideoWriter;


struct task_t
{
    Mat frame, frame_gray;
    vector<Rect> faces;
    task_t(Mat frame, Mat frame_gray, vector<Rect> faces):frame(frame), frame_gray(frame_gray), faces(faces) {

    }
};

class stage1: public tbb::filter{
public:     
    stage1():tbb::filter(tbb::filter::serial) {}
    void * operator()(void *){

         while(1){
            Mat frame, frame_gray;
            vector<Rect> faces;
            capture >> frame;

            if (frame.empty()) return NULL;      
            task_t * task = new task_t(frame, frame_gray, faces);

            return task;
        }
    }
};

class stage2 : public tbb::filter{
public:
    stage2() : tbb::filter(tbb::filter::serial) {}
    void* operator()(void *item){
        task_t *task = static_cast <task_t*> (item);
        cvtColor(task->frame, task->frame_gray, COLOR_BGR2GRAY);
        equalizeHist(task->frame_gray, task->frame_gray);

        // Detect faces
        face_cascade.detectMultiScale(task->frame_gray, task->faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        return task;
    }
};

class stage3 : public tbb::filter{
public:
    stage3() : tbb::filter(tbb::filter::serial){}
    void* operator()(void* item){
        task_t *task = static_cast <task_t*> (item);
        Mat &frame = task->frame;
        Mat &frame_gray = task->frame_gray;
        vector<Rect> &faces = task->faces;

        for (size_t ic = 0, ib = 0; ic < faces.size(); ic++) // Iterate through all current elements (detected faces)
        {
            cv::Rect roi_c;
            roi_c.x = faces[ic].x;
            roi_c.y = faces[ic].y;
            roi_c.width = (faces[ic].width);
            roi_c.height = (faces[ic].height);

            int ac = roi_c.width * roi_c.height; // Get the area of current element (detected face)

            cv::Rect roi_b;
            roi_b.x = faces[ib].x;
            roi_b.y = faces[ib].y;
            roi_b.width = (faces[ib].width);
            roi_b.height = (faces[ib].height);

            int ab = roi_b.width * roi_b.height; // Get the area of biggest element, at beginning it is same as "current" element

            if (ac > ab){
                ib = ic;
                roi_b.x = faces[ib].x;
                roi_b.y = faces[ib].y;
                roi_b.width = (faces[ib].width);
                roi_b.height = (faces[ib].height);
            }
            Mat crop,res, gray;
            crop = frame(roi_b);
            resize(crop, res, Size(128, 128), 0, 0, INTER_LINEAR); // This will be needed later while saving images
            cvtColor(crop, gray, CV_BGR2GRAY); // Convert cropped image to Grayscale

            Point pt1(faces[ic].x, faces[ic].y); // Display detected faces on main window - live stream from camera
            Point pt2((faces[ic].x + faces[ic].height), (faces[ic].y + faces[ic].width));
            rectangle(frame, pt1, pt2, Scalar(0, 255, 0), 2, 8, 0);
        }
        
        return task;
    }
};


class stage4 : public tbb::filter{
public:
    stage4() : tbb::filter(tbb::filter::serial){}
    void* operator()(void* item){
        task_t *task = static_cast <task_t*> (item);
        oVideoWriter.write(task->frame);
        delete task;
        return NULL;
    }
};


void process(int num_threads){
    tbb::task_scheduler_init init(num_threads);
    tbb::pipeline pipeline;
    stage1 S1;
    pipeline.add_filter(S1);
    stage2 S2;
    pipeline.add_filter(S2);
    stage3 S3;
    pipeline.add_filter(S3);
    stage4 S4;
    pipeline.add_filter(S4);
    pipeline.run(num_threads * num_threads);
    pipeline.clear();
}


// Function main
int main(int argc, char* argv[])
{

    if(argc < 2){
        fprintf(stderr,"./binary <video_input>");
        exit(-1);
    }

    //Set OpenCV parallelism degree to 0. Zero works better than 1, trust.
    setNumThreads(0);

    
    capture.open(argv[1]);

    double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
    Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
    
    
    oVideoWriter.open("result.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true);

    if (!capture.isOpened())  // check if we succeeded
        return -1;

    // Load the cascade
    if (!face_cascade.load(face_cascade_name)){
        printf("--(!)Error loading\n");
        return (-1);
    }

    auto t_start = std::chrono::high_resolution_clock::now();
    process(8);
    auto t_end = std::chrono::high_resolution_clock::now();

    std::cout << "Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;

    return 0;
}

