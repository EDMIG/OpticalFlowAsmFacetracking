// minimal.cpp: Display the landmarks of a face in an image.
//              This demonstrates stasm_search_single.

#include <stdio.h>
#include <stdlib.h>
#include "opencv/highgui.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "stasm_lib.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
using namespace std;
using namespace cv;

void applyoptflow(Mat &frame, Mat &lastframe, float * landmarks, int NUM, float * outpts)
{
    Mat status;
    Mat errout;
    vector<Point2f> pointin;
    vector<Point2f> pointout;
    for(int i=0;i<NUM;i++)
    {
        pointin.push_back(Point2f(landmarks[2*i],landmarks[2*i+1]));
    }
    calcOpticalFlowPyrLK(lastframe, frame, pointin, pointout, status, errout );
    for(int i=0;i<NUM;i++)
    {
        outpts[2*i]=pointout[i].x;
        outpts[2*i+1]=pointout[i].y;
        //cout<<landmarks[2*i]<<endl;
    }
    //cout<<"test: "<<pointin[50].x<<' '<<pointout[50].x<<endl;
}


int main(int argc, char ** argv)
{
    static const char* const path = "../data/testface.jpg";

    cout<<"change begin..."<<endl;
    cv::Mat_<unsigned char> img(cv::imread(path, CV_LOAD_IMAGE_GRAYSCALE));

    if (!img.data)
    {
        printf("Cannot load %s\n", path);
        exit(1);
    }

    int foundface;
    float landmarks[2 * stasm_NLANDMARKS]; // x,y coords (note the 2)
    float markshot[2 * stasm_NLANDMARKS]; //for save the first frame landmard
    float savedmarks[2 * stasm_NLANDMARKS];
    float landmarks_sh[2 * stasm_NLANDMARKS]; // x,y coords (note the 2)
    
    //******************adding video******************
    if (argc==1)
    {


    const string mp4path="../data/demo.mp4";
    //VideoCapture vdread("../data/demo.mp4");
    VideoCapture vdread(0);
    if(!vdread.isOpened()){
        cout<<"video file not open"<<endl;
        return -1;
    }

    bool sign=true;
    bool firstsearch=true;
    Mat lastframe;
    while(sign){
        Mat frame;
        vdread >> frame;
        //****add the stop sign
        if (!frame.data){
            sign=false;
            break;
        }
        //to draw, ge the gray frame

        //test if stable on a fray image
        //frame=imread("minimal.bmp");

        Mat framelt;
        Mat gray2;
        resize(frame,framelt,Size(640,360));
        cvtColor(framelt,gray2,CV_RGB2GRAY);
        

        
        //set frame to get position
        if (!stasm_search_single(&foundface, landmarks,
                             (const char*)gray2.data, gray2.cols, gray2.rows, path, "../data"))
        {
            printf("Error in stasm_search_single: %s\n", stasm_lasterr());
            exit(1);
        }
        

        if (!foundface)
             printf("No face found in %s\n", path);
        else
        {
            //cout<<landmarks[50]<<' '<<savedmarks[50]<<endl;
            
            // draw the landmarks on the image as white dots (image is monochrome)
            stasm_force_points_into_image(landmarks, gray2.cols, gray2.rows);
            stasm_force_points_into_image(savedmarks, gray2.cols, gray2.rows);
            //check if this is the first frame, if not, points position can be edited
            if(!firstsearch) 
            {
                float dir[2];
                for (int i=0;i<stasm_NLANDMARKS*2;i++)
                {
                    dir[0]=landmarks[i*2]-savedmarks[i*2];
                    dir[1]=landmarks[i*2+1]-savedmarks[i*2+1];
                    float step=dir[0]*dir[0]+dir[1]*dir[1];
                    //cout<<step<<' ';
                    // if (step>5)
                    // {
                    //     float step_x,step_y;
                    //     step_x=dir[0]/sqrt(step)*2;
                    //     step_y=dir[1]/sqrt(step)*2;
                    //     //cout<<step_x<<' '<<step_y<<endl;
                    //     landmarks[i*2]=savedmarks[i*2]+step_x;
                    //     landmarks[i*2+1]=savedmarks[i*2+1]+step_y;
                    // }
                    landmarks[i]=(savedmarks[i]*1.2+landmarks[i]*0.8)/2;

                }
            }


            //now apply the optical flow
            std::vector<Point2f> pointout; 
            if(!firstsearch)
            {
                applyoptflow(framelt,lastframe,savedmarks,stasm_NLANDMARKS,markshot);

            }
            if (!firstsearch)
            {    
                for(int i=0;i<stasm_NLANDMARKS*2;i++)
                {
                    landmarks_sh[i]=(landmarks[i]*0.5+markshot[i]*1.5)/2;
                }
            }                
            for (int i = 0; i < stasm_NLANDMARKS; i++)
            {   
                //gray2(Range(cvRound(landmarks[2*i+1]),cvRound(landmarks[2*i+1])+1),Range(cvRound(landmarks[2*i]),cvRound(landmarks[2*i])+1)) = 255;
                //void circle(Mat& img, Point center, int radius, const Scalar& color, int thickness=1, int lineType=8, int shift=0)¶
                circle(gray2,Point2f(landmarks_sh[2*i],landmarks_sh[2*i+1]),1,Scalar(255,255,255));
                //cout<<i<<' '<<cvRound(landmarks[i*2+1])<<" "<<cvRound(landmarks[i*2])<<endl;
            }
            
            memcpy(savedmarks,landmarks,sizeof(float)*stasm_NLANDMARKS*2);
            //cout<<"saved? "<<landmarks[50]<<' '<<savedmarks[50]<<endl;
        }



        imshow("frame",gray2);
        //if (!firstsearch)
        //    imshow("lastframe",framelt);
        int key=waitKey(10);
        if (key==27) break;
        //write positions to a file

        //not first time any more
        firstsearch=false;
        lastframe=framelt;



    }
    cout<<"finish the video..."<<endl;
    




    // if (!stasm_search_single(&foundface, landmarks,
    //                          (const char*)img.data, img.cols, img.rows, path, "../data"))
    // {
    //     printf("Error in stasm_search_single: %s\n", stasm_lasterr());
    //     exit(1);
    // }

    // if (!foundface)
    //      printf("No face found in %s\n", path);
    // else
    // {
    //     // draw the landmarks on the image as white dots (image is monochrome)
    //     stasm_force_points_into_image(landmarks, img.cols, img.rows);
    //     for (int i = 0; i < stasm_NLANDMARKS; i++)
    //         img(cvRound(landmarks[i*2+1]), cvRound(landmarks[i*2])) = 255;
    // }
    }

    //cv::imwrite("minimal.bmp", img);
    //cv::imshow("stasm minimal", img);
    //cv::waitKey();
    return 0;
}
