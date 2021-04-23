#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"

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


    //подключаемся к БД
    db.setDatabaseName("Driver={SQL Server};Server=DESKTOP-CR5MEE4\\SQLEXPRESS;Trusted_Connection=Yes;Database=Dairy;");
    db.setUserName("DESKTOP-CR5MEE4\\Kote_Animen");
    db.setPassword("");
    if(!db.open())
    {
        QMessageBox::StandardButton ErrorOpenFile;
        ErrorOpenFile = QMessageBox::critical(this,
                                              QString::fromUtf8("Ошибка"),
                                              QString::fromUtf8("<font size='16'>Отсутствует подключение к базе данных!</font>"));
    }
    query.prepare("SELECT nomenclature FROM dbo.nomenclature");
    query.exec();
    while(query.next())
    {
        ui->typeProduct->addItem(query.value(0).toString(), 0);
    }

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


void MainWindow::on_StartCamera_clicked()
{
    thread_cam->start();
}
