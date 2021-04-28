#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    camera = new CameraConnect();
    qRegisterMetaType<cv::Mat>("cv::Mat");
    thread_cam = new QThread();
    camera->moveToThread(thread_cam);    
    connect(camera, &CameraConnect::FrameReady, this, &MainWindow::Paint);
    connect(this, &MainWindow::getFrame, camera, &CameraConnect::GrabImage);
    connect(thread_cam, &QThread::started, camera, &CameraConnect::GrabImage);

    //связываем действия с уже созданными в дизайнере
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
    //соединяем действия с со слотами с реализацией
    connect(aboutUs, &QAction::triggered, this, &MainWindow::AboutUsShow);
    connect(openDataMatrixDirectory, &QAction::triggered, this, &MainWindow::OpenDataMatrixDirectory);
    connect(exit, &QAction::triggered, qApp, &QApplication::quit);
    //считываем путь в котором указана директория с файлами DataMatrix
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

//отображаем изображение с камеры
void MainWindow::Paint(cv::Mat src)
{
    QImage *CamImg = new QImage(src.data, src.cols, src.rows, src.step,QImage::Format_Grayscale8);
    ui->cameraScreen->setPixmap(QPixmap::fromImage(*CamImg).scaled(ui->cameraScreen->size()));
    ui->cameraScreen->update();
    emit getFrame();
    delete CamImg;
}

//функция печати
void MainWindow::Print(QString str)
{
    QByteArray array = str.toUtf8();
    QAbstractSocket *socket;
    socket = new QAbstractSocket(QAbstractSocket::TcpSocket, this);
    socket->setProxy(QNetworkProxy::NoProxy);
    socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024);
    socket->connectToHost("192.168.0.35", 9100, QAbstractSocket::ReadWrite);
    if (socket->waitForConnected(1000))
    {        
    if (socket->write(array) != -1)
    {

    while (socket->bytesToWrite() > 0)
    {
    socket->flush();
    }
    }
    else
    {
    QMessageBox::StandardButton ErrorOpenFile;
    ErrorOpenFile = QMessageBox::critical(this,
    QString::fromUtf8("Ошибка"),
    QString::fromUtf8("<font size='14'>Ошибка печати!</font>"));
    }
    }
    else
    {
    QMessageBox::StandardButton ErrorOpenFile;
    ErrorOpenFile = QMessageBox::critical(this,
    QString::fromUtf8("Ошибка"),
    QString::fromUtf8("<font size='14'>Ошибка доступа к принтеру! Проверьте питания принтера!</font>"));
    }
    socket->disconnectFromHost();
}

//стартуем камеру
void MainWindow::on_StartCamera_clicked()
{
    camera->initSome();
    camera->setSerial("22310683");
    camera->OpenCamera();
    camera->setFeatureTriggerModeType(true);
    thread_cam->start();
}

//загружаем в массив DataMatrixы из файла
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
//тут выскакивает MessageBox с инфой о программе
void MainWindow::AboutUsShow()
{
    QMessageBox::StandardButton ErrorOpenFile;
    ErrorOpenFile = QMessageBox::information(this,
                                          QString::fromUtf8("О программе"),
                                          QString::fromUtf8("<font size='14'>Моя программа для работы с БД, принтером и камерой</font>"));
}

//функция, где мы выбираем директорию с файлами DataMatrix
void MainWindow::OpenDataMatrixDirectory()
{
    path = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Открыть директорию"),QDir::currentPath(), QFileDialog::ShowDirsOnly);
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.setValue("path", path);
}

//функция, где идет подсчет свободных DataMatrix для конкретного продукта
void MainWindow::on_typeProduct_activated(const QString &arg1)
{
    if(path != "")
    {
       LoadFileDataMatrix(arg1);
       //***сделано по тупому, нужно было сделать через "комплексный" запрос
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
           countCode = query.value(0).toInt();
       }
       countFreeDataMatrix = 5000 - countCode;
       ui->freeStickers->setText("Количество оставшихся этикеток: " + QString::number(countFreeDataMatrix));
       //***

    }
    else
    {
        QMessageBox::StandardButton ErrorOpenFile;
        ErrorOpenFile = QMessageBox::warning(this,
                                              QString::fromUtf8("Не указана директория"),
                                              QString::fromUtf8("<font size='14'>Укажите директорию c DataMatrix</font>"));
    }
}

//событие печати DataMatrix(нужно доделать)
void MainWindow::on_Print_clicked()
{
    int countStickers = ui->countStickers->value();
    int i = 0;
    if(countFreeDataMatrix > 0 && countFreeDataMatrix - countStickers > 0){

        while(i < countStickers)
        {
            Print(first + arrayDataMatrixes[ i + (countFreeDataMatrix - 5000)*(-1)] + end);
            query.prepare("INSERT INTO dbo.products(nomenclature_code, datescan, dateexp) VALUES(?, ?, ?)");
            query.addBindValue(code);
            query.addBindValue(QDateTime::currentDateTime());
            query.addBindValue(ui->date->dateTime());
            query.exec();
            countFreeDataMatrix--;
            ui->freeStickers->setText("Количество оставшихся этикеток: " + QString::number(countFreeDataMatrix));
            i++;
        }
    }

    else
    {
        QMessageBox::StandardButton ErrorOpenFile;
        ErrorOpenFile = QMessageBox::critical(this,
                                              QString::fromUtf8("Ошибка"),
                                              QString::fromUtf8("<font size='14'>Не хватает DataMatrix для печати</font>"));

    }
}
