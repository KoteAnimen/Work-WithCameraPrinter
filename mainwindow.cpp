#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int metatype_id = qRegisterMetaType<cv::Mat>("cv::Mat");
    camera = new CameraConnection();
    thread_cam = new QThread();
    camera->moveToThread(thread_cam);
    connect(camera, &CameraConnection::FrameReady, this, &MainWindow::Paint);
    connect(this, &MainWindow::getFrame, camera, &CameraConnection::Grab);
    connect(thread_cam, &QThread::started, camera, &CameraConnection::Grab);
    thread_cam->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Paint(cv::Mat src)
{
    QImage *CamImg = new QImage(src.data, src.cols, src.rows, src.step,QImage::Format_Grayscale8);
    ui->cameraScreen->setPixmap(QPixmap::fromImage(*CamImg).scaled(ui->cameraScreen->size()));
    ui->cameraScreen->update();
    emit getFrame();
    delete CamImg;

}

