#include "cameraconnect.h"
#include <QThread>

#include <QDateTime>
#include <QDebug>

CameraConnect::CameraConnect(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

CameraConnect::~CameraConnect()
{
    deleteAll();
}

void CameraConnect::initSome()
{
    qDebug() << "CameraConnection: PylonInitialize initSome" ;
    PylonInitialize();
}

void CameraConnect::deleteAll()
{
    if(m_isOpenAcquire) {
        StopAcquire();
    }
    //关闭摄像头
    CloseCamera();
    //关闭库
    qDebug() << "CameraConnection deleteAll: PylonTerminate" ;
    PylonTerminate();
    qDebug() << "CameraConnection deleteAll: Close" ;
}

void CameraConnect::ReInit()
{
    StopAcquire();
    CloseCamera();
}

QStringList CameraConnect::cameras()
{
    return m_cameralist;
}

void CameraConnect::UpdateCameraList()
{

}

void CameraConnect::CopyToImage(CGrabResultPtr pInBuffer, QImage &OutImage)
{

}

void CameraConnect::setSerial(const QString &value)
{
    Serial = value;
}

void CameraConnect::onTimerGrabImage()
{

}

int CameraConnect::OpenCamera()
{
    if(m_basler.IsOpen()) {
        return -1;
    }
    try {
        CDeviceInfo cInfo;
        String_t str = String_t(Serial.toStdString().c_str());
        // cInfo.SetIpAddress(str);
        cInfo.SetSerialNumber(str);

        m_basler.Attach(CTlFactory::GetInstance().CreateDevice(cInfo),Cleanup_Delete);
        m_basler.Open();
        INodeMap &cameraNodeMap = m_basler.GetNodeMap();

        getFeatureTriggerSourceType();
        m_isOpen = true;
        const CIntegerPtr widthPic = cameraNodeMap.GetNode("Width");
        cam_width =  widthPic->GetValue();
        const CIntegerPtr heightPic = cameraNodeMap.GetNode("Height");
        cam_heigth = heightPic->GetValue();
        emit GrabResult("Camera is opened");
        emit SendBG(QString(m_basler.GetDeviceInfo().GetSerialNumber()));

    } catch (GenICam::GenericException &e) {

        emit GrabResult( "OpenCamera Error" + QString(e.GetDescription()));
        m_isOpen = false;
        return -2;
    }
    return 0;
}

int CameraConnect::CloseCamera()
{
    if(!m_basler.IsOpen()) {
        return -1;
    }
    try {
        StopAcquire();

        m_basler.DestroyDevice();
        m_basler.DetachDevice();
        m_basler.Close();

        emit GrabResult("Camera is closed");

    } catch (GenICam::GenericException &e) {

        emit GrabResult( "CloseCamera Error:" + QString(e.GetDescription()));
        return -2;
    }
    return 0;
}

void CameraConnect::setExposureTime(double time)
{
    SetCamera(Type_Basler_ExposureTimeAbs, time);
}

int CameraConnect::getExposureTime()
{
    return QString::number(GetCamera(Type_Basler_ExposureTimeAbs)).toInt();
}

int CameraConnect::getExposureTimeMin()
{
    return DOUBLE_MIN;
}

int CameraConnect::getExposureTimeMax()
{
    return DOUBLE_MAX;
}

void CameraConnect::setFeatureTriggerSourceType(QString type)
{
    //停止采集
    if(m_isOpenAcquire) {
        StopAcquire();
    }
    if(type == "Freerun") {
        SetCamera(Type_Basler_Freerun);
    } else if(type == "Line1"){
        SetCamera(Type_Basler_Line1);
    }
}

QString CameraConnect::getFeatureTriggerSourceType()
{
    INodeMap &cameraNodeMap = m_basler.GetNodeMap();
    CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode ("TriggerSelector");
    ptrTriggerSel->FromString("FrameStart");
    CEnumerationPtr  ptrTrigger  = cameraNodeMap.GetNode ("TriggerMode");
    ptrTrigger->SetIntValue(1);
    CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode ("TriggerSource");


    String_t str = ptrTriggerSource->ToString();
    m_currentMode = "Software";
    return m_currentMode;
}

void CameraConnect::setFeatureTriggerModeType(bool on)
{
    INodeMap &cameraNodeMap = m_basler.GetNodeMap();
    CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode ("TriggerSelector");
    ptrTriggerSel->FromString("FrameStart");
    CEnumerationPtr  ptrTrigger  = cameraNodeMap.GetNode ("TriggerMode");
    ptrTrigger->SetIntValue(on?1:0);
    CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode ("TriggerSource");
    ptrTriggerSource->FromString("Software");
}

bool CameraConnect::getFeatureTriggerModeType()
{
    INodeMap &cameraNodeMap = m_basler.GetNodeMap();
    CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode ("TriggerSelector");
    ptrTriggerSel->FromString("FrameStart");
    CEnumerationPtr  ptrTrigger  = cameraNodeMap.GetNode ("TriggerMode");
    return ptrTrigger->GetIntValue() == 1;
}

void CameraConnect::SetCamera(CameraConnect::CameraConnection_Type index, double tmpValue)
{
    INodeMap &cameraNodeMap = m_basler.GetNodeMap();
    switch (index) {
    case Type_Basler_Freerun: {
        CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode ("TriggerSelector");
        ptrTriggerSel->FromString("FrameStart");
        CEnumerationPtr  ptrTrigger  = cameraNodeMap.GetNode ("TriggerMode");
#ifdef Real_Freerun
        ptrTrigger->SetIntValue(0);
#else //Software
        ptrTrigger->SetIntValue(1);
        CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode ("TriggerSource");
        ptrTriggerSource->FromString("Software");
#endif
    } break;
    case Type_Basler_Line1: {
        CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode ("TriggerSelector");
        ptrTriggerSel->FromString("FrameStart");
        CEnumerationPtr  ptrTrigger  = cameraNodeMap.GetNode ("TriggerMode");
        ptrTrigger->SetIntValue(1);
        CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode ("TriggerSource");
        ptrTriggerSource->FromString("Line1");
    } break;
    case Type_Basler_ExposureTimeAbs: {
        const CFloatPtr exposureTime = cameraNodeMap.GetNode("ExposureTimeAbs");
        exposureTime->SetValue(tmpValue);
    } break;
    case Type_Basler_GainRaw: {
        const CIntegerPtr cameraGen = cameraNodeMap.GetNode("GainRaw");
        cameraGen->SetValue(tmpValue);
    } break;
    case Type_Basler_AcquisitionFrameRateAbs: {
        const CBooleanPtr frameRate = cameraNodeMap.GetNode("AcquisitionFrameRateEnable");
        frameRate->SetValue(TRUE);
        const CFloatPtr frameRateABS = cameraNodeMap.GetNode("AcquisitionFrameRateAbs");
        frameRateABS->SetValue(tmpValue);
    } break;
    case Type_Basler_Width: {
        const CIntegerPtr widthPic = cameraNodeMap.GetNode("Width");
        widthPic->SetValue(tmpValue);
    } break;
    case Type_Basler_Height: {
        const CIntegerPtr heightPic = cameraNodeMap.GetNode("Height");
        heightPic->SetValue(tmpValue);
    } break;
    case Type_Basler_LineSource: {
        CEnumerationPtr  ptrLineSource = cameraNodeMap.GetNode ("LineSource");
        ptrLineSource->SetIntValue(2);
    } break;
    default:
        break;
    }
}

double CameraConnect::GetCamera(CameraConnect::CameraConnection_Type index)
{
    INodeMap &cameraNodeMap = m_basler.GetNodeMap();
    switch (index) {
    case Type_Basler_ExposureTimeAbs: {
        const CFloatPtr exposureTime = cameraNodeMap.GetNode("ExposureTimeAbs");
        return exposureTime->GetValue();
    } break;
    case Type_Basler_GainRaw: {
        const CIntegerPtr cameraGen = cameraNodeMap.GetNode("GainRaw");
        return cameraGen->GetValue();
    } break;
    case Type_Basler_AcquisitionFrameRateAbs: {
        const CBooleanPtr frameRate = cameraNodeMap.GetNode("AcquisitionFrameRateEnable");
        frameRate->SetValue(TRUE);
        const CFloatPtr frameRateABS = cameraNodeMap.GetNode("AcquisitionFrameRateAbs");
        return frameRateABS->GetValue();
    } break;
    case Type_Basler_Width: {
        const CIntegerPtr widthPic = cameraNodeMap.GetNode("Width");
        return widthPic->GetValue();
    } break;
    case Type_Basler_Height: {
        const CIntegerPtr heightPic = cameraNodeMap.GetNode("Height");
        return heightPic->GetValue();
    } break;
    default:
        return -1;
        break;
    }
}

long CameraConnect::StartAcquire()
{
    m_currentMode = "Software";
    m_isOpenAcquire = true;
    try
    {
        m_basler.StartGrabbing(GrabStrategy_LatestImageOnly);

    }
    catch (GenICam::GenericException &e)
    {

        emit GrabResult("StartAcquire Error:" + QString(e.GetDescription()));

        m_isOpenAcquire = false;
        return -2;
    }
    return 0;
}

long CameraConnect::StopAcquire()
{
    m_isOpenAcquire = false;
    qDebug() << "CameraConnection StopAcquire";
    try
    {
        if (m_basler.IsGrabbing())
        {
            m_basler.StopGrabbing();
        }
    } catch (GenICam::GenericException &e)
    {

        emit GrabResult("StopAcquire Error:" + QString(e.GetDescription()));

        return -2;
    }
    return 0;
}

long CameraConnect::GrabImage()
{
    mtx = cv::Mat::zeros(cam_width, cam_heigth, CV_8UC1);
    m_currentMode = "Software";

    if(!m_basler.IsOpen())
    {
        OpenCamera();
    }
    try  {
        if (!m_basler.IsGrabbing())
        {
            StartAcquire();
        }
        CGrabResultPtr ptrGrabResult;
        if(m_currentMode == "Software")
        {
            if (m_basler.WaitForFrameTriggerReady(1000, TimeoutHandling_Return)) {
                m_basler.ExecuteSoftwareTrigger();
                m_basler.RetrieveResult(5000, ptrGrabResult,TimeoutHandling_Return);
            }
        }
        if(ptrGrabResult == NULL)
        {
            //            CloseCamera();
            //            while(!m_basler.IsOpen())
            //            OpenCamera();
            emit GrabResult("NULL grabresult");
            emit FrameReady(mtx);
            return -5;
        }
        if (ptrGrabResult->GrabSucceeded())
        {
            // emit GrabResult( "what: ptrGrabResult GrabSucceeded");
            if (!ptrGrabResult.IsValid())
            {
                emit GrabResult("GrabResult not Valid Error");
//                emit FrameReady(mtx);
                return -1;
            }
            else
            {
                mtx = cv::Mat(ptrGrabResult->GetHeight(),ptrGrabResult->GetWidth(), CV_8UC1,(uchar*) ptrGrabResult->GetBuffer());
                cv::resize(mtx, mtx, cv::Size(), 0.5, 0.5);
                emit FrameReady(mtx);
//                imshow("SRC", mtx);
            }
        }
        else
        {
            emit GrabResult("Grab Error!!!");
            CloseCamera();
//            emit FrameReady(mtx);
            return -3;
        }
    }
    catch (GenICam::GenericException &e)
    {

        CloseCamera();
        //        while(!m_basler.IsOpen())
        //        OpenCamera();

        emit GrabResult("GrabImage Error:" + QString(e.GetDescription()));
//        emit FrameReady(mtx);
        return -2;
    }
    catch(...)  {
        emit GrabResult("Not known Error");
        return -1;
    }
    return 0;
}
