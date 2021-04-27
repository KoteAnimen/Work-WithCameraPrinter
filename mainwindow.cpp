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

    QAction *aboutUs = ui->AboutUs;
    QAction *openDataMatrixDirectory = ui->Open;
    QAction *exit = ui->Exit;
    // Создаем объект класса QMenu (меню)
    QMenu *file1;
    QMenu *file2;
    file1 = ui->menu;
    file2 = ui->menu_2;
    // Помещаем действие "Quit" (Выход) в меню с помощью метода addAction()
    file1->addAction(openDataMatrixDirectory);
    file1->addAction(exit);
    file2->addAction(aboutUs);
    connect(aboutUs, &QAction::triggered, this, &MainWindow::AboutUsShow);
    connect(openDataMatrixDirectory, &QAction::triggered, this, &MainWindow::OpenDataMatrixDirectory);
    connect(exit, &QAction::triggered, qApp, &QApplication::quit);
    QSettings settings("settings.ini", QSettings::IniFormat);
    path = settings.value("path").toString();

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
        ui->typeProduct->addItem(query.value("nomenclature").toString().trimmed());
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

void MainWindow::LoadFileDataMatrix(QString nameDataMatrix)
{
    int i = 0;
    QFile file(path + QString("/") + nameDataMatrix + QString(".csv"));
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            fromFileInfo = in.readLine();
            arrayDataMatrixes[i] = fromFileInfo;
            i++;
        }
    }
    else
    {
        QMessageBox::StandardButton ErrorOpenFile;
        ErrorOpenFile = QMessageBox::critical(this,
                                              QString::fromUtf8("Ошибка"),
                                              QString::fromUtf8("<font size='14'>Файл с DataMatrix не найден. Возможно файла не существует или укажите другую директорию</font>"));    }
}

void MainWindow::AboutUsShow()
{
    QMessageBox::StandardButton ErrorOpenFile;
    ErrorOpenFile = QMessageBox::information(this,
                                          QString::fromUtf8("О программе"),
                                          QString::fromUtf8("<font size='14'>Моя программа для работы с БД, принтером и камерой</font>"));
}

void MainWindow::OpenDataMatrixDirectory()
{
    path = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Открыть директорию"),QDir::currentPath(), QFileDialog::ShowDirsOnly);
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.setValue("path", path);
}

void MainWindow::on_typeProduct_activated(const QString &arg1)
{
    if(path != "")
    {
       LoadFileDataMatrix(arg1);
       QString code;
       int countCode;
       query.prepare("SELECT code FROM dbo.nomenclature WHERE nomenclature = '" + ui->typeProduct->currentText() + QString("'") );
       query.exec();
       while(query.next())
       {
           code = query.value(0).toString();
       }
       query.prepare("SELECT COUNT(nomenclature_code) FROM dbo.products WHERE nomenclature_code = '" + code +QString("'"));
       query.exec();
       while(query.next())
       {
           countCode += query.value(0).toInt();
       }
       ui->freeStickers->setText("Количество оставшихся этикеток: " + QString::number(5000 - countCode));

    }
    else
    {
        QMessageBox::StandardButton ErrorOpenFile;
        ErrorOpenFile = QMessageBox::warning(this,
                                              QString::fromUtf8("Не указана директория"),
                                              QString::fromUtf8("<font size='14'>Укажите директорию c DataMatrix</font>"));
    }
}
