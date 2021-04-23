#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cameraconnection.h>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CameraConnection *camera;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QSqlQuery query;
    QThread *thread_cam;
    bool startCamera = false;


signals:
    void getFrame();

public slots:
    void Paint(cv::Mat);

private slots:
    void on_StartCamera_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
