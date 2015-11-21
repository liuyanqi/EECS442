#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

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

/* 
  Draws an ellipse, used for testing webcam drawing
 */
void MyEllipse( Mat img, double angle ){
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

/*
  Deprecated
 */
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

/* 
  Skin thresholding model, turns image into binary image.
   White = Skin;
   Black = Not-skin;
 */
void skin_thres(Mat &img){
    int y = 0;
    int x = 0;
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

/*
  Deprecated
 */
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

/*
  Takes in an image "img" and draws it's major contours on the image "draw"
 */
void skin_contours(Mat img, Mat draw){
	Mat canny_out;
    int max =0;
    int max_ind =0;
	vector<vector<Point>> contours;
    vector<vector<Point>>  contours1;
    vector<bool> ifcontour(img.cols, img.rows);
    contours1.resize(1);
	vector<Vec4i> hierarchy;

	// Output image size for debugging
	cout << "Image size: " << img.rows << " " << img.cols <<endl;
    
	// Extract contours
	Canny(img, canny_out, 100, 255);
	try {
		findContours(canny_out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
	}catch (Exception e){
		cout << e.msg << endl;
	}

	// Draw white contours
    Scalar c = Scalar(255, 255, 255);
    for(int i =0; i< contours.size(); ++i){

		// Don't include contours that are tiny - noise
        if(contours[i].size() < 50){
            continue;
        }

		// Record the largest contour
        if(contourArea(contours[i]) > max){
            max_ind =i;
            max = contourArea(contours[i]);
        }

        drawContours(draw, contours, i, c, 1, 8, hierarchy, 0, Point());
    }
}


int main( int argc, char** argv )
{
	// Load image
    Mat src = imread("hand2.jpg", CV_LOAD_IMAGE_COLOR);

    Mat ori = src;
    Size size(640,480);
    resize(ori,ori,size);    
    Mat draw = Mat::zeros(ori.size(), CV_8UC3);
    
	// Change to binary image, indicating skin
	skin_thres(src);

	// Draws skin contours to "draw"
    skin_contours(src,draw);    
    
	// Display regular image to contour side-by-side
    Mat im3(ori.rows, ori.cols+draw.cols, CV_8UC3);
    Mat left(im3, Rect(0, 0, ori.cols, ori.rows));
    ori.copyTo(left);
    Mat right(im3, Rect(ori.cols, 0, draw.cols, draw.rows));
    draw.copyTo(right);
    imshow("im3", im3);
     
	//Create infinte loop for live streaming 
    while(1){        
        //find_contour(img);

		//Capture Keyboard stroke
		int key = cvWaitKey(10);    

		//If you hit ESC key loop will break
        if (char(key) == 27){
            break; 
        }
    }

	//Destroy Window
    cvDestroyWindow("Camera_Output"); 
    return 0;
}