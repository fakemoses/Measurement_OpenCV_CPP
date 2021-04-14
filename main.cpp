#include <iostream>
#include <fstream>
#include <tuple>
#include <cmath>
#include <bits/stdc++.h>
#include <iterator>
#include <typeinfo>

//include for openCV

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;


// compiling :: g++ -o  output main.cpp `pkg-config --cflags --libs opencv4`

//define default param either here or down in the functions
tuple<Mat, vector<struct finalContours>> getContours(Mat img, int cThr1, int cThr2, bool showCanny, int minArea, int filter, bool draw);
vector<Point> reorder(vector<Point> points);
Mat warpImg(Mat img, vector<Point> points, float w, float h, int pad);
int getIndex(vector<int> v, int K);
float findDist(Point pts1, Point pts2);

//structure for the output
struct finalContours
{
    int len;
    double area;
    vector<Point> approx;
    Rect bbox;
    int i;
};

int main(int argc, char const *argv[])
{
    //VideoCapture video_cap(0, cv::CAP_V4L2);
    VideoCapture video_cap("vid2.mp4", cv::CAP_FFMPEG);
    Mat img, resizedImg,paperImg, warpedImg, insideImg;
    vector<struct finalContours> paperData, insideData;
    vector<Point> biggest, rPoints;
    double x, y, w ,h;

    //paper setting and scaling
    double scale = 3;
    float wP = 210 * scale;
    float hP = 297 * scale;

    int nW, nH;

    //check if error
    if (!video_cap.isOpened())
        throw "Error opening";

    video_cap.set(10, 160);
    video_cap.set(3, 1920);
    video_cap.set(4, 1080);

    namedWindow("Input", cv::WINDOW_AUTOSIZE);
    namedWindow("canvas", cv::WINDOW_AUTOSIZE);
    // for(;;) is also useable here
    while (true)
    {
        //save frame in img using the extraction operator
        video_cap >> img;
        if (img.empty())
            break;

        //use function
        auto data = getContours(img, 100, 100, false, 5000, 4, false);
        //access data from the tuple

        paperImg = get<0>(data);
        paperData = get<1>(data);

        //cout << finalData.size() << endl;

        if (paperData.size() != 0)
        {
            //take the largest contour!
            //here data type is basically vectorpoints
            biggest = paperData[0].approx;

            warpedImg = warpImg(img, biggest, wP, hP, 20);

            //get new img contors from the new warped image

            auto data2 = getContours(warpedImg, 50, 50, false, 2000, 4, false);
            insideImg = get<0>(data2);
            insideData = get<1>(data2);

            if (insideData.size() != 0)
            {
                //cout << insideData.size() << endl;
                for (int i = 0; i < insideData.size(); i++)
                {   

                    polylines(insideImg,insideData[i].approx, true, 2);
                    rPoints = reorder(insideData[i].approx);
                    nW = fabs(findDist(rPoints[0]/scale, rPoints[1] / scale) /10.0);
                    nH = fabs(findDist(rPoints[0]/scale, rPoints[2] / scale) /10.0);
                    //cout <<"nw: "<< nW  << " nH: "<< nH<< endl;

                    //arrowed lines
                    arrowedLine(insideImg,rPoints[0], rPoints[1], Scalar(255, 0, 255),2,8,0,0.05);
                    arrowedLine(insideImg,rPoints[0], rPoints[2], Scalar(255, 0, 255),2,8,0,0.05);

                    x = insideData[i].bbox.x;
                    y = insideData[i].bbox.y;
                    w = insideData[i].bbox.width;
                    h = insideData[i].bbox.height;

                    putText(insideImg, to_string(nW) + " cm", Point(x+30, y),FONT_HERSHEY_COMPLEX_SMALL,1, Scalar(255,0,255), 2);
                    putText(insideImg, to_string(nH) + " cm", Point(x, y+(h/2)),FONT_HERSHEY_COMPLEX_SMALL,1, Scalar(255,0,255), 2);
                }
            }
            imshow("canvas", insideImg);
        }

        //test output
        resize(img, resizedImg, Size(0,0), 0.5, 0.5);
        //imshow("Warped", warpedImg);
        imshow("Input", resizedImg);
        //waitkey is preventing it to end after execution
        int key = (waitKey(0) && 0xFF);

        if (key == 'q')
        {
            break;
        }
    }
    return 0;
}

tuple<Mat, vector<struct finalContours>> getContours(Mat img, int cThr1 = 100, int cThr2 = 100, bool showCanny = false, int minArea = 1000, int filter = 0, bool draw = false)
{

    Mat imgGray, imgCanny, imgBlur, imgDilate, imgErode;
    double area, perim;
    //2d array
    vector<vector<Point>> contours;
    //1d arrays
    vector<int> len;
    Rect bbox;

    struct finalContours tempContours;

    //vector to save the output data
    vector<finalContours> finalOutput;

    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    GaussianBlur(imgGray, imgBlur, Size(5, 5), 1, 0);
    Canny(imgBlur, imgCanny, cThr1, cThr2);

    //create a kernel for dialation and erosion
    Mat kernel = Mat(5, 5, CV_8UC1, cv::Scalar(1));

    dilate(imgCanny, imgDilate, kernel, Point(-1, -1), 3);
    erode(imgDilate, imgErode, kernel, Point(-1, -1), 2);

    if (showCanny)
        imshow("Canny", imgCanny);

    findContours(imgErode, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> approx(contours.size());

    for (int i = 0; i < contours.size(); i++)
    {
        //find area
        area = fabs(contourArea(contours[i], true));

        if (isgreater(area, minArea))
        {
            perim = arcLength(contours[i], true);
            approxPolyDP(contours[i], approx[i], 0.02 * perim, true);
            bbox = boundingRect(approx[i]);

            if (filter > 0)
            {
                if (approx[i].size() == filter)
                {
                    //save in struct
                    tempContours.len = contours.size();
                    tempContours.area = area;
                    tempContours.approx = approx[i];
                    tempContours.bbox = bbox;
                    tempContours.i = i;

                    //append struct to an output vector
                    finalOutput.push_back(tempContours);

                    //sort vector based on the max area
                    std::sort(finalOutput.begin(), finalOutput.end(), [](finalContours a, finalContours b) {
                        return a.area > b.area;
                    });
                }
            }
            else
            {
                //save in struct
                tempContours.len = contours.size();
                tempContours.area = area;
                tempContours.approx = approx[i];
                tempContours.bbox = bbox;
                tempContours.i = i;

                //append struct to an output vector
                finalOutput.push_back(tempContours);

                //sort vector based on the max area
                std::sort(finalOutput.begin(), finalOutput.end(), [](finalContours a, finalContours b) {
                    return a.area > b.area;
                });
            }
        }
    }
    return make_tuple(img, finalOutput);
}

//reorder
vector<Point> reorder(vector<Point> points)
{
    //incase segmentation fault here is to be checked!
    vector<Point> newPoints(points.size());
    vector<int> add(points.size());
    vector<int> diff(points.size());
    int largest, smallest;

    //calculate summation to determine point 0 and 3
    for (int i = 0; i < points.size(); i++)
    {
        add[i] = points[i].x + points[i].y;
    }

    //calculate difference to determine point 0 and 3
    for (int i = 0; i < points.size(); i++)
    {
        diff[i] = points[i].y - points[i].x;
    }

    //assign new points in newPoints
    smallest = *min_element(add.begin(), add.end());
    newPoints[0] = points[getIndex(add, smallest)];

    largest = *max_element(add.begin(), add.end());
    newPoints[3] = points[getIndex(add, largest)];

    smallest = 0;
    largest = 0;

    smallest = *min_element(diff.begin(), diff.end());
    newPoints[1] = points[getIndex(diff, smallest)];

    largest = *max_element(diff.begin(), diff.end());
    newPoints[2] = points[getIndex(diff, largest)];

    return newPoints;
}

//warp images

Mat warpImg(Mat img, vector<Point> points, float w, float h, int pad)
{
    vector<Point> formattedPoints;
    vector<Point2f> pts1;
    vector<Point2f> pts2(points.size());
    Mat matrix, newImg;

    //reorder points since it may be random.
    //To be achieved is:
    //
    //  P0 *-------* P1
    //     .       .
    //     .       .
    //  P2 *-------* P3
    formattedPoints = reorder(points);

    //convert points to float
    for (int j = 0; j < formattedPoints.size(); j++)
    {
        pts1.push_back((Point2f)formattedPoints[j]);
    }

    //create pts2
    pts2[0].x = 0.0;
    pts2[0].y = 0.0;

    pts2[1].x = w;
    pts2[1].y = 0.0;

    pts2[2].x = 0.0;
    pts2[2].y = h;

    pts2[3].x = w;
    pts2[3].y = h;

    //get perspective transform matrix
    matrix = getPerspectiveTransform(pts1, pts2);

    warpPerspective(img, newImg, matrix, Size(w, h));

    return newImg;
}

// check again this part
float findDist(Point pts1, Point pts2)
{
    float res;
    res = (pts2.x - pts1.x) ^ 2 + (pts2.y - pts1.y) ^ (1/2);

    return res;
}

int getIndex(vector<int> v, int K)
{
    auto it = find(v.begin(), v.end(), K);
    int index;

    if (it != v.end())
    {
        index = it - v.begin();
    }

    return index;
}