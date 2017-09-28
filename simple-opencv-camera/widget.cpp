#include "widget.h"
#include "ui_widget.h"
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QDebug>
#include <iostream>

using namespace std;
using namespace cv;
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    timer=new QTimer(this);
    //image=new QImage();
    connect(ui->OnButton,SIGNAL(clicked()),this,SLOT(opencamara()));
    connect(timer,SIGNAL(timeout()),this,SLOT(getframe()));
    funcchoice='o';
}

Widget::~Widget()
{
    delete ui;
}

void Widget::opencamara()
{
    if(ui->OnButton->text()=="ON")
    {
       cam.open(0);
       timer->start(33);
       ui->OnButton->setText("OFF");
    }
    else
    {
        timer->stop();
        cam.release();
        ui->OnButton->setText("ON");
        ui->Video->clear();
        funcchoice='o';
    }
}

void Widget::getframe()
{
    cam>>frame;
    if(frame.empty())
        qDebug()<<"failture!!!";
    switch (funcchoice)
    {
            case 'a': detectAndDraw( frame, cascade, nestedCascade, 1, false );
                      break;
            case 'b': myhoughcircles(frame);
                      break;
            case 'c': frame=sobel(frame);
                      break;
            default : ;
                      break;
    }
    QImage image=MatToQImage(frame);
    //QImage image((const uchar*)frame.data,frame.cols,frame.rows,QImage::Format_RGB888);
    ui->Video->setPixmap(QPixmap::fromImage(image));
}


QImage Widget::MatToQImage(const Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void Widget::detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip )
{
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    cv::resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    t = (double)getTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)getTickCount() - t;
    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                       cvPoint(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30) );
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
    }
}


void Widget::on_DectButton_clicked()
{
    if(!(funcchoice=='a'))
    {
        funcchoice='a';
        if ( !nestedCascade.load( "/home/lxg/ouc/opencv-camera/simple-opencv-camera/haarcascade_eye_tree_eyeglasses.xml" ) )
            cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
        if( !cascade.load( "/home/lxg/ouc/opencv-camera/simple-opencv-camera/haarcascade_frontalface_alt.xml" ) )
        {
            cerr << "ERROR: Could not load classifier cascade" << endl;
            exit(1);
        }
    }
    else
        funcchoice='o';
}


void Widget::on_CircleButton_clicked()
{
    if(funcchoice=='b')
        funcchoice='o';
    else
        funcchoice='b';
}


void Widget::myhoughcircles(Mat &img)
{
    Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        medianBlur(gray, gray, 5);
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                     gray.rows/16, // change this value to detect circles with different distances to each other
                     100, 30, 1, 30 // change the last two parameters
                                    // (min_radius & max_radius) to detect larger circles
                     );

        for( size_t i = 0; i < circles.size(); i++ )
        {
            Vec3i c = circles[i];
            circle( img, Point(c[0], c[1]), c[2], Scalar(0,0,255), 3, LINE_AA);
            circle( img, Point(c[0], c[1]), 2, Scalar(0,255,0), 3, LINE_AA);
        }
}

Mat Widget::sobel(Mat &src)
{
    Mat src_gray, grad;
    int scale = 1;
      int delta = 0;
      int ddepth = CV_16S;

      GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );

      cvtColor( src, src_gray, COLOR_BGR2GRAY );

      Mat grad_x, grad_y;
      Mat abs_grad_x, abs_grad_y;

      //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
      Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );

      //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
      Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );

      convertScaleAbs( grad_x, abs_grad_x );
      convertScaleAbs( grad_y, abs_grad_y );

      addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

      return grad;
}
void Widget::on_SobelButton_clicked()
{
    if(funcchoice=='c')
        funcchoice='o';
    else
        funcchoice='c';

}
