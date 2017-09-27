#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;

class QTimer;
class QImage;


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QImage MatToQImage(const Mat &mat);

    void detectAndDraw( Mat& img, CascadeClassifier& cascade,CascadeClassifier& nestedCascade,double scale, bool tryflip );

    Mat sobel(Mat &src);

public slots:
    void opencamara();

    void getframe();

    void myhoughcircles(Mat &img);

private slots:
    void on_DectButton_clicked();

    void on_CircleButton_clicked();

    void on_SobelButton_clicked();

private:
    Ui::Widget *ui;
    QTimer *timer;
    //QImage *image;
    VideoCapture cam;
    Mat frame;
    char funcchoice;
    CascadeClassifier cascade, nestedCascade;
};

#endif // WIDGET_H
