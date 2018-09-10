#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
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
    int tot = 0;
    while(1){
        Mat frame;
        capture >> frame;
        if (frame.empty()) break;
        tot++;

        std::vector<Rect> faces;
        Mat frame_gray;        

        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
        
        equalizeHist(frame_gray, frame_gray);

        // Detect faces
        face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

       
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
        
        oVideoWriter.write(frame);

    }

    cerr << tot << endl;

    auto t_end = std::chrono::high_resolution_clock::now();

    std::cout << "Execution time(s): " << std::chrono::duration<double>(t_end-t_start).count() << std::endl;

    return 0;
}

