#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cameraconnect.h>
#include <barcodeprocessing.h>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QAbstractSocket>
#include <QNetworkProxy>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CameraConnect *camera;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QSqlQuery query;
    QThread *thread_cam;
    bool startCamera = false;
    QString arrayDataMatrixes[5000];
    QString fromFileInfo = "";
    QString path;
    QString code = "";
    const QString first = "^XA" "^FO 360,50" "^FB400,2,10,C,0" "^ASN,10,10" "^BXN,5,200,,,,_" "^FD_1", end = "^FS" "^XZ";
    int countFreeDataMatrix = 0;

    void LoadFileDataMatrix(QString);
    void Print(QString);


signals:
    void getFrame();

public slots:
    void Paint(cv::Mat);
    void AboutUsShow();
    void OpenDataMatrixDirectory();

private slots:
    void on_StartCamera_clicked();

    void on_typeProduct_activated(const QString &arg1);

    void on_Print_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
