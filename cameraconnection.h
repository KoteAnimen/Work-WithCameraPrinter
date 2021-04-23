#ifndef CAMERACONNECTION_H
#define CAMERACONNECTION_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/types.hpp>

#include <pylon/PylonIncludes.h>
#include <pylon/AviCompressionOptions.h>


#include <pylon/usb/BaslerUsbCamera.h>
#include <pylon/usb/_BaslerUsbCameraParams.h>

#include <QMetaType>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QObject>

using namespace Pylon;

using namespace std;
class CameraConnection : public QObject
{
    Q_OBJECT
public:
    CameraConnection(QObject *parent = 0 , int iop = 0);

    bool isOpenCamera();
    bool flag;

    int getExpos() const;
    void setExpos(int value);

    QString getSerialNum() const;
    void setSerialNum(const QString &value);

    bool getLineInv() const;
    void setLineInv(bool value);

    void setOffsetY(int value);

private:
    //CBaslerGigECamera *Camera;
    CBaslerUsbCamera *Camera;
    int expos = 2486;                      // экспозиция
    bool isAbort;                   // стоп граббить
    QString serialNum;
    bool request = 1;
    bool LineInv;
    int offsetY;
    PylonAutoInitTerm autoInitTerm;
    CTlFactory& TlFactory  = CTlFactory::GetInstance();
    ITransportLayer *pTl = TlFactory.CreateTl( Pylon::CBaslerUsbCamera::DeviceClass() );
    DeviceInfoList_t devices;
    CBaslerUsbCamera::StreamGrabber_t *StreamGrabber;
    GrabResult Result;


signals:
    void FrameReady(cv::Mat);
    void FrameReady1(cv::Mat);
    void StopThread();
    void CameraReady();
    void CameraNotOpen();
    void NoCameraPresent();
    void serialNumber(QString);


public slots:
    void Grab();                     // здесь захватывается видео
    void SetAscuitionMode();   // непрерывный режим
    void StopGrab();                 // Остановить прием кадров
    void setReqestFalse();
    void setReqestTrue();
};

#endif // CAMERACONNECTION_H
