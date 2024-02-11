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
        //arg[1] is the path of video file to be processed, it needs to passed in command line
        cap=VideoCapture(argv[1]);
    }
    else {
        // this else statement is for the exe to run on our local machine only
        cap=VideoCapture ("C:\\Users\\CS\\Desktop\\Hackathon.mp4");
    }
    // Check if video stream opened successfully
    if (!cap.isOpened()) {
        //handling the error, if it fail to load video file from specified path
        cout << "Error opening video stream or file" << endl;
        return -1;
    }
    // Get the frame dimensions from video stream
    int width = cap.get(CAP_PROP_FRAME_WIDTH);        //frame width of video file
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);      //frame height of video file
    
    //initialising video writer with encoding type XVID and specifying the path of output file 
    //    but over here only file name with extension is specified hence stores in current directory
    VideoWriter video("output.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), 10, Size(width, height), true);   

    /*
        Initializing variables to increase sensitivity at the initial frames
        extra_initial_conflict => stores how much the inital frames affect conflict
        initial_conflict frames => stores no of inital frames where sensitivity has to be increased
    */
    uchar extra_initial_conflict = 0b1111, initial_conflict_frames = 10;

    //Declaring Mat objects necessary for processing
    Mat img, bg,conflict, temp1, temp2, gray, kern ,frame;

    // img is the 3 channel frame of size of video's frame
    img = Mat(height,width,CV_8UC3);

    //temp1,temp2 are 1 channel variants of [img]'s dimensions 
    //    used to hold intermediate results during processing
    temp1 = Mat(img.rows, img.cols, CV_8UC1);
    temp2 = Mat(img.rows, img.cols, CV_8UC1);

    //gray is a 1 channel Mat Object with dimensions of [img] used to store gray scaled image
    gray = Mat(img.rows, img.cols, CV_8UC1);
    
    //bg is a 1 channel Mat Object with dimensions of [img] used to store calculated background
    bg = Mat(img.rows, img.cols, CV_8UC1);
    
    //conflict is a 1 channel Mat Object with dimensions of [img] 
    //    used to store occurances of conflicting pixels
    conflict = Mat(img.rows, img.cols, CV_8UC1);

    //frame is 3 channel Object used to stores the gaussian blurred colour image
    frame = Mat(img.rows, img.cols, CV_8UC3);

    //kernal of single channel of size 5,5 for dialation filter
    kern = Mat::ones(Size(5, 5), CV_8UC1);

    //variable to check if background frame is set or not
    bool first_frame = true;
    
    while (1) {

        // Read the video frame-by-frame and store in [img]
        // If the frame is empty, break immediately
        if (!cap.read(img))
            break;

        //applying gaussian blur for reducing noise in the frame
        GaussianBlur(img, frame, Size(5, 5), 0);

        //converting it to gray image copy to Mat object gray
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        
        if (first_frame) {
            //if there is no background frame then assign current frame as background and start processing
            //this if block runs only once, that is start of processing a video 
            //     after that anyway we will store the current frame to previous frame 
            first_frame = false;    
            gray.copyTo(bg);
        }
        
        //it takes the absolute difference of previous and current frame and store it in temp1
        absdiff(gray, bg, temp1);
        
        //If the difference is between 20 to 255, then considereed as significant change orelse ignored
        inRange(temp1, 20, 255, temp2);

        //This condition increases sensitivity of threasholded values of initial frames in affecting the conflict matrix
        if (initial_conflict_frames > 0) {
            conflict += temp2 & extra_initial_conflict;
            initial_conflict_frames--;
        }

        //Normally, conflict matrix get updated here
        //if there is a mismatched pixel , then 2 is added for the corresponding pixel
        conflict += temp2 & 0x02;
        
        for (int x = 0; x < bg.rows; x++) {
            for (int y = 0; y < bg.cols; y++) {
                uchar val = conflict.at<uchar>(Point(y, x));
                if (val > 0)
                {
                    if (val > 70) {
                        // if mismatch has occured for 70 or more frames, then replace 
                        //    the pixel with current background and set conflict to 0
                        bg.at<uchar>(Point(y, x)) = gray.at<uchar>(Point(y, x));
                        conflict.at<uchar>(Point(y, x)) = 0;
                    }
                    else {
                        conflict.at<uchar>(Point(y, x)) = val - 1;
                    }
                }
            }
        }
        // Thresholded image(not conflict matrix) is blurred and thresholded to reduce noise
        medianBlur(temp2, temp1, 5);
        inRange(temp1, 128, 255, temp2);
        // the now cleaned image is dilated
        filter2D(temp2, temp1, -1, kern);

        //Identifying the edges using canny detector to get overlay
        Canny(temp1, temp2, 0, 255);

        // Applying overlay to image
        for (int x = 0; x < bg.rows; x++) {
            for (int y = 0; y < bg.cols; y++) {
                if (temp2.at<uchar>(Point(y, x)) != 0)
                {
                    // wherever canny image shows edge(value is 255 there),
                    //    set the color image pixel to BGR(0,120,255) which is the value of orange
                    img.at<Vec3b>(Point(y, x))[0] = 0;
                    img.at<Vec3b>(Point(y, x))[1] = 120;
                    img.at<Vec3b>(Point(y, x))[2] = 255;
                }
            }
        }

        //writing the overlayed img frame to the video writer stream
        video.write(img);

    }

    // When everything done, release the video capture object
    cap.release();

    // Closes all the frames
    cv::destroyAllWindows();

    return 0;
}
