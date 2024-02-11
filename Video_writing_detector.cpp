// Enter only the path file, not memory address
#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;
int main(int argc,char** argv) {

    // Create a VideoCapture object and open the input file
    // If the input is the web camera, pass 0 instead of the video file name
    VideoCapture cap;
    if (argc >= 2) {
        //
        cap=VideoCapture(argv[1]);
    }
    else {// this else statement is for the exe to run on our local machine only
        cap=VideoCapture ("C:\\Users\\CS\\Desktop\\Hackathon.mp4");
    }
    // Check if camera opened successfully
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }
    int width = cap.get(CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);
    VideoWriter video("output.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), 10, Size(width, height), true);
    uchar extra_initial_conflict = 0b1111, initial_conflict_frames = 10;
    Mat img, bg,conflict, temp1, temp2, gray, kern ,frame;
    img = Mat(height,width,CV_8UC3);
    temp1 = Mat(img.rows, img.cols, CV_8UC1);
    temp2 = Mat(img.rows, img.cols, CV_8UC1);
    frame = Mat(img.rows, img.cols, CV_8UC3);
    gray = Mat(img.rows, img.cols, CV_8UC1);
    bg = Mat(img.rows, img.cols, CV_8UC1);
    conflict = Mat(img.rows, img.cols, CV_8UC1);
    kern = Mat::ones(Size(5, 5), CV_8UC1);
    bool first_frame = true;
    while (1) {

        // Capture frame-by-frame
        if (!cap.read(img))
            break;
        GaussianBlur(img, frame, Size(5, 5), 0);
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        // If the frame is empty, break immediately
        if (first_frame) {
            first_frame = false;
            gray.copyTo(bg);
        }
        absdiff(gray, bg, temp1);
        inRange(temp1, 20, 255, temp2);

        if (initial_conflict_frames > 0) {
            conflict += temp2 & extra_initial_conflict;
            initial_conflict_frames--;
        }
        conflict += temp2 & 0x02;
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
        medianBlur(temp2, temp1, 5);
        inRange(temp1, 128, 255, temp2);
        filter2D(temp2, temp1, -1, kern);

        Canny(temp1, temp2, 0, 255);

        for (int x = 0; x < bg.rows; x++) {
            for (int y = 0; y < bg.cols; y++) {
                if (temp2.at<uchar>(Point(y, x)) != 0)
                {
                    img.at<Vec3b>(Point(y, x))[0] = 0;
                    img.at<Vec3b>(Point(y, x))[1] = 120;
                    img.at<Vec3b>(Point(y, x))[2] = 255;
                }
            }
        }

        video.write(img);

    }

    // When everything done, release the video capture object
    cap.release();

    // Closes all the frames
    cv::destroyAllWindows();

    return 0;
}
