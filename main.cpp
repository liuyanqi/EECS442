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

void threshold_image(Mat img, IplImage* img1){

	// Get contours of image
	Mat tmp_img;
    IplImage* _pDistImage = cvCreateImage( cvGetSize( img1 ), IPL_DEPTH_32F, 1 );
    _pDistImage->origin = img1->origin;

	try{
		cvtColor(img, tmp_img, CV_BGR2GRAY);
		blur(tmp_img, tmp_img, Size(3, 3));
	} catch (Exception e){
		cout << e.msg << endl;
	}
    
	Mat canny_out;
    int max =0;
    int max_ind =0;
	vector<vector<Point>> contours;
    vector<vector<Point>>  contours1;
    bool ifcontour[img.cols][img.rows];
    cout<<img.rows<<" "<<img.cols<<endl;
    contours1.resize(1);
	vector<Vec4i> hierarchy;
    
    
	Canny(tmp_img, canny_out, 100, 255);
	try {
		findContours(canny_out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
	} catch (Exception e){
		cout << e.msg << endl;
	}
    
    for(int i =0; i< contours.size(); ++i){
        if (contourArea(contours[i]) < 100){
			continue;
		}
        if(contourArea(contours[i]) > max){
            max_ind =i;
            max = contourArea(contours[i]);
            
        }
        for(int j =0 ;j < contours[i].size(); ++j){
            ifcontour[contours[i][j].x][contours[i][j].y] = true;
            contours1[0].push_back(contours[i][j]);
        }

    }
    Mat draw = Mat::zeros(canny_out.size(), CV_8UC3);
    Scalar c = Scalar(255, 255, 255);
    drawContours(draw, contours1, 0, c, 1, 8, hierarchy, 0, Point());
    imshow("Contours", draw);
    
    //start from the largest part of the counter
   /* Point start, current, prev;
    static int dir[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
    start.x = contours[max_ind][0].x;
    start.y = contours[max_ind][0].y;
    
    int nContour = 0;
    for ( prev = current = start ; ; )
    {
        //
        // Choose direction
        for ( int i = 0 ; i < 8 ; i ++ )
        {
            int newX = current.x + dir[i][0];
            int newY = current.y + dir[i][1];
            
            if ( newX > 0 && newX < img1->width-1 &&
                newY > 0 && newY < img1->height-1 &&
                ( newX != current.x || newY != current.y ) &&
                ( newX != prev.x || newY != prev.y ) &&
                ifcontour[newX][newY] )
            {
                bool fEdge = false;
                for ( int j = 0 ; j < 8 ; j ++ )
                {
                    int neighborX = newX + dir[j][0];
                    int neighborY = newY + dir[j][1];
                    if ( ifcontour[neighborX][neighborY]  )
                    {
                        fEdge = true;
                        break;
                    }
                }
                
                if ( fEdge )
                {
                    //
                    // Draw the contour point
                    //
                    cout<<1<<endl;
                    cvCircle( _pDistImage, current, 1, CV_RGB(0,0,0), 1, 8, 0 );
                    //                cvSet2D( dstImage, current.y, current.x, CV_RGB(0,255,0) );
                    
                    //
                    // Move the point
                    //
                    prev = current;
                    current.x = newX;
                    current.y = newY;
                    break;
                }
            }
        }
        
        if ( current.x == start.x && current.y == start.y )
        {
            break;
        }
        
        nContour ++;
        if ( nContour == 10000 ) break;
    }
	
	//for (int i = 0; i < contours1[0].size(); ++i){
		
		// Draw contour with random colour

	//}*/
    //Mat hand = cvarrToMat(_pDistImage);
    
}


int main( int argc, char** argv )
{

    IplImage* img = cvLoadImage("hand1.jpg",CV_LOAD_IMAGE_GRAYSCALE);
    Mat src = imread("hand1.jpg", CV_LOAD_IMAGE_COLOR);
    //threshold_image(src);
    threshold_image(src,img);
    
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