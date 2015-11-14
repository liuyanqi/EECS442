#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "MixGaussian.h"

using namespace cv;
using namespace std;
#define w  400

struct contoursCmpY {
    bool operator()(const Point &a,const Point &b) const {
        if (a.y == b.y)
            return a.x < b.x;
        
        return a.y < b.y;
    }
} contoursCmpY_;

void MyEllipse( Mat img, double angle )
{
    int thickness = 2;
    int lineType = 8;
    
    ellipse( img,
            Point( w/2, w/2 ),
            Size( w/4, w/16 ),
            angle,
            0,
            360,
            Scalar( 255, 0, 0 ),
            thickness,
            lineType );
}

void change_pixel(Mat img){
    for(int y=0;y<img.rows;y++)
    {
        for(int x=0;x<img.cols;x++)
        {
            // get pixel
            Vec3b color = img.at<Vec3b>(Point(x,y));
            
            if(color.val[2] > 50){
                color.val[0] =0;
                color.val[1] =0;
                color.val[2] =0;
            }
            else{
                color.val[0] = 255;
                color.val[1] =255;
                color.val[2] =255;
            }
            
            // set pixel
            img.at<Vec3b>(Point(x,y)) = color;
        }
    }
}

void skin_thres(Mat &img){
    
    int y =0;
    int x =0;
    Size size(640,480);
    resize(img,img,size);
    blur(img, img, Size(3, 3));
    for(y = 0; y < img.rows; y++){
        for(x = 0; x< img.cols; x++){
            Vec3b color = img.at<Vec3b>(Point(x,y));
            if(abs(color.val[0]-color.val[1]) < 20  && abs(color.val[2]-color.val[1])<20 
            && abs(color.val[0]-color.val[2])<20 ){
                color.val[0] = 0;
                color.val[1] = 0;
                color.val[2] = 0;
                img.at<Vec3b>(Point(x,y)) = color; 
            }
            else{
                color.val[0] = 255;
                color.val[1] = 255;
                color.val[2] = 255;
                img.at<Vec3b>(Point(x,y)) = color; 
            }
        }
    }
    //namedWindow("skin", WINDOW_NORMAL);
    //imshow("skin", img);
}

void skin_color(Mat img){
    int max = 0; 
    int min =255;
    int i =0;
    int y =0;
    int x =0;
    for(y=0;y<img.rows;y++)
    {
        for(x=0;x<img.cols;x++)
        {
           Vec3b color = img.at<Vec3b>(Point(x,y));
            
            if((color.val[0] >95) && (color.val[1]>40) && (color.val[2]>20) && (abs(color.val[0]-color.val[1])>15) && (color.val[0]>color.val[1]) && (color.val[0]>color.val[2])){
                
                for(i =0 ; i<3; i++){
                    if(color.val[i] > max) max = color.val[i];
                    if(color.val[i] < min) min = color.val[i];
                }
                if((max-min) > 15){
                    color.val[0] =0;
                    color.val[1] =0;
                    color.val[2] =0;
                    img.at<Vec3b>(Point(x,y)) = color; 
                }
            }
            else {
                color.val[0] =255;
                color.val[1] =255;
                color.val[2] =255;
                img.at<Vec3b>(Point(x,y)) = color; 
            }
            
            max =0;
            min = 255;
        }
    }

                
}

void threshold_image(Mat img, Mat draw){


    
	Mat canny_out;
    int max =0;
    int max_ind =0;
	vector<vector<Point>> contours;
    vector<vector<Point>>  contours1;
    bool ifcontour[img.cols][img.rows];
    cout<<img.rows<<" "<<img.cols<<endl;
    contours1.resize(1);
	vector<Vec4i> hierarchy;
    
    
	Canny(img, canny_out, 100, 255);
	try {
		findContours(canny_out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
	} catch (Exception e){
		cout << e.msg << endl;
	}

    Scalar c = Scalar(255, 255, 255);
    for(int i =0; i< contours.size(); ++i){

        if(contours[i].size() <100){
            continue;
        }
        if(contourArea(contours[i]) > max){
            max_ind =i;
            max = contourArea(contours[i]);
            
        }
        /*
        for(int j =0 ;j < contours[i].size(); ++j){
            ifcontour[contours[i][j].x][contours[i][j].y] = true;
            contours1[0].push_back(contours[i][j]);
        }*/
        drawContours(draw, contours, i, c, 1, 8, hierarchy, 0, Point());
    }
   
    
   
    //imshow("Contours", draw);

    
}


int main( int argc, char** argv )
{

    IplImage* img = cvLoadImage("hand1.jpg",CV_LOAD_IMAGE_GRAYSCALE);
    Mat src = imread("hand3.jpg", CV_LOAD_IMAGE_COLOR);

    Mat ori = src;
    Size size(640,480);
    resize(ori,ori,size);
    
    Mat draw = Mat::zeros(ori.size(), CV_8UC3);
    skin_thres(src);
    
    threshold_image(src,draw);
    
    
    Mat im3(ori.rows, ori.cols+draw.cols, CV_8UC3);
    Mat left(im3, Rect(0, 0, ori.cols, ori.rows));
    ori.copyTo(left);
    Mat right(im3, Rect(ori.cols, 0, draw.cols, draw.rows));
    draw.copyTo(right);
    imshow("im3", im3);
     
    while(1){ //Create infinte loop for live streaming
        
        
        //find_contour(img);
       int key = cvWaitKey(10);     //Capture Keyboard stroke
        if (char(key) == 27){
            break;      //If you hit ESC key loop will break.
        }
    }

    cvDestroyWindow("Camera_Output"); //Destroy Window
    return 0;
}