#include "HandRegion.h"
#include <stdio.h>

HandRegion::HandRegion(void)
{
    _pImage = 0;
    _pImageGradient = 0;
    _pImageHandMask = 0;
    _pImageYCrCb = 0;

    // Histogram
    for ( int i = 0 ; i < 16 ; i ++ )
        for ( int j = 0 ; j < 16 ; j ++ )
            for ( int k = 0 ; k < 16 ; k ++ )
                _histPixel[i][j][k] = 0;
    _nTotalPixel = 0;

    _fLearned = false;

}

HandRegion::~HandRegion(void)
{
    if ( _pImage )
    {
        cvReleaseImage( &_pImage );
    }
    if ( _pImageGradient )
    {
        cvReleaseImage( &_pImageGradient );
    }
    if ( _pImageHandMask )
    {
        cvReleaseImage( &_pImageHandMask );
    }
    if ( _pImageYCrCb )
    {
        cvReleaseImage( &_pImageYCrCb );
    }
}

bool HandRegion::LoadSkinColorProbTable()
{
    if ( _SkinColor.LoadLookUpTable( "skin.dis" ) == false )
    {
        if ( _SkinColor.LoadFile( "skin.mgm" ) == false )
        {
            fprintf( stderr, "skin color distribution load error.\n" );
            return false;
        }
        printf("making a lookup table for skin color distribution ");
        _SkinColor.MakeLookUpTable();
        printf("done\n");
        if ( _SkinColor.SaveLookUpTable( "skin.dis" ) == false )
        {
            fprintf( stderr, "skin color distribution look up table save error.\n" );
            return false;
        }
    }
    if ( _NonSkinColor.LoadLookUpTable( "nonskin.dis" ) == false )
    {
        if ( _NonSkinColor.LoadFile( "nonskin.mgm" ) == false )
        {
            fprintf( stderr, "non-skin color distribution load error.\n" );
            return false;
        }
        printf("making a lookup table for non-skin color distribution ");
        _NonSkinColor.MakeLookUpTable();
        printf("done\n");
        if ( _NonSkinColor.SaveLookUpTable( "nonskin.dis" ) == false )
        {
            fprintf( stderr, "non-skin color distribution look up table save error.\n" );
            return false;
        }
    }

    return true;
}

bool HandRegion::LoadHandMask( char * filename )
{
    //
    // Initialize Memory for Image
    //
    if ( !_pImageHandMask )
    {
        _pImageHandMask = cvLoadImage( filename, CV_LOAD_IMAGE_GRAYSCALE );
        if ( _pImageHandMask == 0 )
            return false;
        cvFlip( _pImageHandMask );
    }

    return true;
}

bool HandRegion::InitHandMask( IplImage * srcImage )
{
    //
    // Initialize Memory for Hand Mask Image
    //
    if ( !_pImageHandMask )
    {
        _pImageHandMask = cvCreateImage( cvGetSize( srcImage ), 8, 1 );
        if ( _pImageHandMask == 0 )
            return false;
        _pImageHandMask->origin = srcImage->origin;
        cvSetZero( _pImageHandMask );
    }

    return true;
}

IplImage * HandRegion::GetHandRegion( IplImage * srcImage, IplImage * srcGrayImage, bool fScreenshot )
{
    //
    // Initialize Memory for Image
    //
    if ( !_pImage )
    {
        _pImage = cvCreateImage( cvGetSize( srcImage ), 8, 1 );
        _pImage->origin = srcImage->origin;
    }
    if ( !_pImageGradient )
    {
        _pImageGradient = cvCreateImage( cvGetSize( srcImage ), 8, 1 );
        _pImageGradient->origin = srcImage->origin;
    }
    if ( !_pImageYCrCb )
    {
        _pImageYCrCb = cvCreateImage( cvGetSize( srcImage ), 8, 3 );
        _pImageYCrCb->origin = srcImage->origin;
    }
    IplImage * pScreenshot = 0;
    if ( fScreenshot )
    {
        pScreenshot = cvCreateImage( cvGetSize( srcImage ), 8, 1 );
        pScreenshot->origin = srcImage->origin;
    }

    //
    // Convert BGR -> YCrCb
    //
    cvCvtColor( srcImage, _pImageYCrCb, CV_BGR2YCrCb );

    //
    // Gradient
    //
    //cvCanny( srcGrayImage, _pImageGradient, 100, 200 );

    //
    // Segmentation by Color Distribution
    //
    int nHandRegion = 0;
    for ( int i = 0 ; i < srcImage->height ; i ++ )
    {
        for ( int j = 0 ; j < srcImage->width ; j ++ )
        {
            unsigned char R = srcImage->imageData[ i * srcImage->widthStep + j*3 + 2];
            unsigned char G = srcImage->imageData[ i * srcImage->widthStep + j*3 + 1];
            unsigned char B = srcImage->imageData[ i * srcImage->widthStep + j*3 + 0];

            float P_Skin = _SkinColor.GetProbabilityByLookup( R, G, B );
            float P_NonSkin = _NonSkinColor.GetProbabilityByLookup( R, G, B );


//            cvSetReal2D( _pImage, i, j, P_Skin*255 );

            float P = P_Skin/P_NonSkin;
            if ( P < 0.4 ) {//|| cvGetReal2D( _pImageGradient, i, j ) > 0 ) {
                _pImage->imageData[ i * _pImage->widthStep + j ] = 0;
            }
            else
            {
                _pImage->imageData[ i * _pImage->widthStep + j ] = 255;
                nHandRegion ++;
            }/**/
            if ( fScreenshot )
            {
                P = sqrt(sqrt(P)) * 255;
                if ( P > 255 ) P = 255;
                pScreenshot->imageData[ i * pScreenshot->widthStep + j ] = P;
            }
        }
    }

    if ( fScreenshot )
    {
        cvSaveImage( "screenshot_handprob.png", pScreenshot );
        cvReleaseImage( &pScreenshot );
    }

    return _pImage;
}



void HandRegion::DrawContour( IplImage * dstImage, CvPoint start, IplImage * distImage )
{
    CvPoint current, prev;
    static int dir[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};

    //
    // Check validity
    //
    if ( _pImage == 0 || dstImage == 0 ||
         start.x < 0 || start.x >= _pImage->width ||
         start.y < 0 || start.y >= _pImage->height )
    {
        return;
    }

    //
    // First, drop the start point
    //
    for ( ; start.y > 2 &&
            cvGetReal2D( _pImage, start.y - 1, start.x ) > 0 ;
            start.y = start.y - 1 );

    //
    // Traverse the contour
    //
    int nContour = 0;
    for ( prev = current = start ; ; )
    {
        //
        // Choose direction
        for ( int i = 0 ; i < 8 ; i ++ )
        {
            int newX = current.x + dir[i][0];
            int newY = current.y + dir[i][1];

            if ( newX > 0 && newX < _pImage->width-1 &&
                 newY > 0 && newY < _pImage->height-1 &&
                 ( newX != current.x || newY != current.y ) &&
                 ( newX != prev.x || newY != prev.y ) &&
                 cvGetReal2D( _pImage, newY, newX ) > 0 )
            {
                bool fEdge = false;
                for ( int j = 0 ; j < 8 ; j ++ )
                {
                    int neighborX = newX + dir[j][0];
                    int neighborY = newY + dir[j][1];
                    if ( cvGetReal2D( _pImage, neighborY, neighborX ) == 0 )
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
                    cvCircle( dstImage, current, 1, CV_RGB(0,255,0), 1, 8, 0 );
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
}
