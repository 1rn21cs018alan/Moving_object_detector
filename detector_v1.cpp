
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
    const int buf_size = 1;
    Mat prev,dif[buf_size],temp;
    int prev_exists = 0,pointer=0;
    while (1) {

        Mat frame,img;
        // Capture frame-by-frame
        if (!cap.read(img))
            break;
        GaussianBlur(img, frame, Size(5, 5), 0);
        // If the frame is empty, break immediately
        if (frame.empty())
            break;
        if (prev_exists >buf_size) {
            Mat sumdif = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
            for(int i=0;i<buf_size;i++)
                add(sumdif, dif[i], sumdif);
            //imshow("merged", sumdif);

            Mat out,kern;
            kern = Mat::ones(15, 15, CV_8UC1);
            //out = sumdif.clone();
            filter2D(sumdif,out,-1,kern);
            //floodFill(out)
            morphologyEx(out,sumdif,MORPH_CLOSE,kern);
            Canny(sumdif, out,255,10);
            //imshow("filtered", out);
            temp = img.clone();
            for (int y = 0; y < frame.rows; y++) {
                for (int x = 0; x < frame.cols; x++) {
                    Vec3b frame_pixel = temp.at<Vec3b>(Point(x, y));
                    uchar& val = out.at<uchar>(Point(x, y));
                    if (val != 0) {
                        frame_pixel[0] = 0;
                        frame_pixel[1] = 128;
                        frame_pixel[2] = 255;
                    }
                    temp.at<Vec3b>(Point(x, y)) = frame_pixel;
                }
            }
            cv::imshow("out", temp);
            cv::imshow("Frame", img);
        }
        // Display the resulting frame

        // Press  ESC on keyboard to exit
        char c = (char)waitKey(25);
        if (c == 27 || c==113)
            break;
        if (prev_exists >0) {
            absdiff(prev,frame,temp);
            cvtColor(temp,dif[pointer],COLOR_BGR2GRAY);
            inRange(dif[pointer], Scalar(40), Scalar(255), dif[pointer]);
            pointer = (pointer + 1) %buf_size;
        }
        prev = frame.clone();
        prev_exists ++;
    }

    // When everything done, release the video capture object
    cap.release();

    // Closes all the frames
    cv::destroyAllWindows();

    return 0;
}
