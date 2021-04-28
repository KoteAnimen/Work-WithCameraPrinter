/*-------------------------------------------------
#
# Project created by QtCreator
# Author: 沙振宇
# CreateTime: 2017-12-14
# UpdateTime: 2019-12-31
# Info: Qt（C++）调用工业相机Basler的SDK使用示例
# Url:https://shazhenyu.blog.csdn.net/article/details/78844664
# Github:https://github.com/ShaShiDiZhuanLan/Demo_BaslerCamera_Qt
#
#-------------------------------------------------*/
#ifndef CameraConnect_H
#define CameraConnect_H

#include <QObject>
#include <pylon/PylonIncludes.h>

#include <QTimer>
#include <opencv2/opencv.hpp>

#define DOUBLE_MAX 100000
#define DOUBLE_MIN 0

using namespace std;
using namespace Pylon;
using namespace GenApi;
class CameraConnect : public QObject
{
    Q_OBJECT
public:
    explicit CameraConnect(QObject *parent = 0);
    ~CameraConnect();
    CInstantCamera m_basler;
    bool m_isOpenAcquire = false;
    bool m_isOpen = false;
    enum CameraConnection_Type{
        Type_Basler_Freerun, //设置相机的内触发
        Type_Basler_Line1, //设置相机的外触发
        Type_Basler_ExposureTimeAbs, //设置相机的曝光时间
        Type_Basler_GainRaw, //设置相机的增益
        Type_Basler_AcquisitionFrameRateAbs, //设置相机的频率
        Type_Basler_Width, //图片的宽度
        Type_Basler_Height, //图片的高度
        Type_Basler_LineSource, //灯的触发信号
    };
    void initSome();
    void deleteAll();
    void ReInit();
    QStringList cameras();
    int OpenCamera();
    int CloseCamera();

    void setExposureTime(double time); // 设置曝光时间
    int getExposureTime(); // 获取曝光时间
    int getExposureTimeMin(); // 最小曝光时间
    int getExposureTimeMax(); // 最大曝光时间

    void setFeatureTriggerSourceType(QString type); // 设置种类
    QString getFeatureTriggerSourceType(); // 获取种类：软触发、外触发等等

    void setFeatureTriggerModeType(bool on); // 设置模式触发
    bool getFeatureTriggerModeType(); // 获取模式触发
    void SetCamera(CameraConnect::CameraConnection_Type index, double tmpValue = 0.0); // 设置各种参数
    double GetCamera(CameraConnect::CameraConnection_Type index); // 获取各种参数
    long StartAcquire();
    long StopAcquire();
    void setSerial(const QString &value);

signals:
    void sigCameraUpdate(QStringList list);
    void sigCameraCount(int count);
    void FrameReady(cv::Mat);
    void GrabResult(QString);
    void SendBG(QString);

private:
    void UpdateCameraList();
    void CopyToImage(CGrabResultPtr pInBuffer, QImage &OutImage);
    QString Serial;
    QStringList m_cameralist;
    QString m_currentMode;
    int cam_width;
    int cam_heigth;
    cv::Mat mtx;

public slots:
    void onTimerGrabImage();
    long GrabImage();
};

#endif // CameraConnection_H
