

#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;
int main() {

    // Create a VideoCapture object and open the input file
    // If the input is the web camera, pass 0 instead of the video file name
    VideoCapture cap("C:\\Users\\CS\\Desktop\\Hackathon.mp4");

    // Check if camera opened successfully
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }
    uchar extra_initial_conflict = 0b1111, initial_conflict_frames = 10;
    Mat frame, img,bg,dif,thresh,temp1,conflict,gray,mask,temp2,kern;
    kern = Mat::ones(Size(5,5), CV_8UC1);
    //kern.at<uchar>(2,2) = 0;
    bool first_frame = true;
    while (1) {

        // Capture frame-by-frame
        if (!cap.read(img))
            break;
        GaussianBlur(img, frame, Size(5, 5), 0);
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        // If the frame is empty, break immediately
        if (frame.empty())
            break;
        if (first_frame) {
            first_frame = false;
            gray.copyTo(bg);
            conflict = Mat::zeros(gray.rows, gray.cols, CV_8UC1);
            temp1 = Mat::zeros(gray.rows, gray.cols, CV_8UC1);
            cout << bg.rows<<","<<bg.cols << endl;
        }
        absdiff(gray, bg,dif);
        inRange(dif, 20, 255,thresh );

        imshow("thresh", thresh);
        if (initial_conflict_frames > 0) {
            conflict += thresh&extra_initial_conflict;
            initial_conflict_frames--;
        }
        conflict += thresh&0x02;
        //imshow("conflict", conflict);
        for (int x = 0; x < bg.rows; x++) {
            for (int y = 0; y < bg.cols; y++) {
                uchar val = conflict.at<uchar>(Point(y, x));
                if (val > 0)
                {
                    if (val > 70) {
                        bg.at<uchar>(Point(y, x)) = gray.at<uchar>(Point(y, x));
                        conflict.at<uchar>(Point(y, x)) = 0;
                    }
                    else {
                        conflict.at<uchar>(Point(y, x)) = val - 1;
                    }
                }
            }
        }
        medianBlur(thresh,temp2,5);
        inRange(temp2, 128, 255, temp1);
        filter2D(temp1,temp2,-1,kern);
        
        //imshow("blur", temp1);
        Canny(temp2, mask, 0, 255);
        //cv::imshow("temp1", temp1);
        //cvtColor(temp2, temp1, COLOR_BGR2GRAY,3);
        cv::imshow("mask", mask);
        for (int x = 0; x < bg.rows; x++) {
            for (int y = 0; y < bg.cols; y++) {
                if (mask.at<uchar>(Point(y,x))!= 0)
                {
                    img.at<Vec3b>(Point(y, x))[0]=0;
                    img.at<Vec3b>(Point(y, x))[1]=120;
                    img.at<Vec3b>(Point(y, x))[2]=255;
                }
            }
        }
        cv::imshow("Frame", img);
        //imshow("bg", bg);
        //VideoWriter outVideo= VideoWriter("output.avi", CAP_PROP_FOURCC,10,Size(img.rows,img.cols));
        // Display the resulting frame

        // Press  ESC OR q on keyboard to exit
        char c = (char)waitKey(45);
        if (c == 27 || c == 113)
            break;
    }

    // When everything done, release the video capture object
    cap.release();

    // Closes all the frames
    cv::destroyAllWindows();

    return 0;
}
