#include "cameraconnection.h"
#include "QTextStream"

using namespace Basler_UsbCameraParams;
using namespace Basler_UsbStreamParams;

struct MyContext
{
};

CameraConnection::CameraConnection(QObject *parent, int iop) :
    QObject(parent)
{
    isAbort = false;
    offsetY = 212;
    try {
        if ( 0 == pTl->EnumerateDevices( devices ) ) {
            cerr << "No camera present!" << endl;
            emit NoCameraPresent();
            return;
        }

        // Create a camera object
        Camera = new CBaslerUsbCamera( pTl->CreateDevice(devices[iop] ) );

    }
    catch( GenICam::GenericException &e )
    {
        cerr << "An exception occurred!" << endl << e.GetDescription() << endl;
        emit CameraNotOpen();
        return;
    }

    Camera->Open();

    Camera->LineSelector.SetValue(LineSelector_Line4);
    Camera->LineMode.SetValue(LineMode_Output);
    Camera->LineSource.SetValue(LineSource_ExposureActive);
    Camera->LineInverter.SetValue(LineInv);

    //        Camera->LineDebouncerTime.SetValue(5);

    Camera->TriggerSelector.SetValue( TriggerSelector_FrameStart );
    Camera->TriggerMode.SetValue( TriggerMode_Off );
    Camera->AcquisitionMode.SetValue( AcquisitionMode_SingleFrame );
    Camera->AcquisitionFrameRateEnable.SetValue(true);
    Camera->AcquisitionFrameRate.SetValue(50);
    //Camera->ExposureTime.SetValue(5000);
    // Camera->PgiMode.SetValue(PgiMode_Off);
    //   Camera->SharpnessEnhancement.SetValue(1.5);
    //        Camera->NoiseReduction.SetValue(0.5);


    // emit CameraReady();         // теперь можем установить режим по прерыванию
    Camera->PixelFormat.SetValue( PixelFormat_Mono8 );

    Camera->WidthMax.GetValue();
    Camera->HeightMax.GetValue();
    Camera->OffsetX.SetValue(0);
    // Camera->OffsetY.SetValue(212);

    //Camera->LineDebouncerTime.SetValue(5);
    Camera->TriggerSource.SetValue(TriggerSource_Software);


    StreamGrabber = new CBaslerUsbCamera::StreamGrabber_t( Camera->GetStreamGrabber(0) );
    StreamGrabber->Open();

    // Parameterize the stream grabber
    const int bufferSize = (int) Camera->PayloadSize();
    const int numBuffers = 1;
    StreamGrabber->MaxBufferSize = bufferSize;
    StreamGrabber->MaxNumBuffer = numBuffers;
    StreamGrabber->PrepareGrab();
    unsigned char* ppBuffers[numBuffers];
    MyContext context[numBuffers];
    StreamBufferHandle handles[numBuffers];
    for ( int i = 0; i < numBuffers; ++i )
    {
        ppBuffers[i] = new unsigned char[bufferSize];
        handles[i] = StreamGrabber->RegisterBuffer( ppBuffers[i], bufferSize);
        StreamGrabber->QueueBuffer( handles[i], &context[i] );
    }

    int metatype_id = qRegisterMetaType<cv::Mat>("cv::Mat");
}

void CameraConnection::SetAscuitionMode()
{
    // Continuous mode, no external trigger used
    Camera->TriggerSelector.SetValue( TriggerSelector_FrameStart );
    Camera->TriggerMode.SetValue( TriggerMode_On );
    Camera->AcquisitionMode.SetValue( AcquisitionMode_SingleFrame);
}



void CameraConnection::Grab()
{


        isAbort = false;
        cv::Mat* mtx;


        Camera->AcquisitionStart.Execute();
        if ( StreamGrabber->GetWaitObject().Wait( 1000 )) {
            if ( ! StreamGrabber->RetrieveResult( Result ) ) {
                cerr << "Failed to retrieve an item from the output queue" << endl;
            }
            if ( Result.Succeeded() ) {
                mtx = new cv::Mat(Result.GetSizeY(),Result.GetSizeX(), CV_8UC1,(uchar*) Result.Buffer());
                emit FrameReady(*mtx);
                if (flag) {
                    emit FrameReady1(*mtx);
                }
                //imshow("mtx", mtx);
            }
            else
            {
                cerr << "Grab failed: " << Result.GetErrorDescription() << endl;

            }
            StreamGrabber->QueueBuffer( Result.Handle(), Result.Context() );
        }
        else {
            cerr << "timeout occurred when waiting for a grabbed image" << endl;

        }
}
//Sleep(10);

//            Camera->AcquisitionStop.Execute();
//            StreamGrabber->CancelGrab();
//            while ( StreamGrabber->GetWaitObject().Wait(0) ) {
//                StreamGrabber->RetrieveResult( Result );
//                if ( Result.Status() == Canceled )
//                    cout << "Got canceled buffer" << endl;
//            }
//            StreamGrabber->FinishGrab();
//            StreamGrabber->Close();
//            Camera->Close();
//            delete Camera;
//            TlFactory.ReleaseTl( pTl );
//            emit StopThread();
// }
// }

//}

//void CameraConnection::Grab()
//{
//    isAbort = false;
//    cv::Mat* mtx;
//    PylonAutoInitTerm autoInitTerm;
//    CTlFactory& TlFactory  = CTlFactory::GetInstance();
//     ITransportLayer *pTl = TlFactory.CreateTl( Pylon::CBaslerUsbCamera::DeviceClass() );
//    DeviceInfoList_t devices;

//    int metatype_id = qRegisterMetaType<cv::Mat>("cv::Mat");
////    CDeviceInfo di;
//   // di.SetSerialNumber(serialNum.toStdString().c_str());
//    try {
//        if ( 0 == pTl->EnumerateDevices( devices ) ) {
//          cerr << "No camera present!" << endl;
//            emit NoCameraPresent();
//           return;
//        }

//        // Create a camera object
//        Camera = new CBaslerUsbCamera( pTl->CreateDevice(devices[0] ) );

//    }
//    catch( GenICam::GenericException &e )
//    {
//        cerr << "An exception occurred!" << endl << e.GetDescription() << endl;
//        emit CameraNotOpen();
//        return;
//    }
//    // Open the camera object

//  // if (pTl->IsDeviceAccessible(di))
//   // {
//        Camera->Open();
////    }

////    else
////    {
////        delete Camera;
////        TlFactory.ReleaseTl( pTl );
////        emit CameraNotOpen();
////        emit StopThread();
////        cerr << "No camera present!" << endl;
////        return;
////    }

//    //==============проверка серийника камеры
////    QString SN = Camera->DeviceSerialNumber.GetValue().c_str();
////    if(SN=="1")
////    {
////        emit serialNumber("another cam");
////    }

////    else
////    {
////        emit serialNumber("right cam");
//       Camera->LineSelector.SetValue(LineSelector_Line4);
//        Camera->LineMode.SetValue(LineMode_Output);
//        Camera->LineSource.SetValue(LineSource_ExposureActive);
//        Camera->LineInverter.SetValue(LineInv);

// //        Camera->LineDebouncerTime.SetValue(5);

//         Camera->TriggerSelector.SetValue( TriggerSelector_FrameStart );
//         Camera->TriggerMode.SetValue( TriggerMode_Off );
//        Camera->AcquisitionMode.SetValue( AcquisitionMode_SingleFrame );
//        Camera->AcquisitionFrameRateEnable.SetValue(true);
//        Camera->AcquisitionFrameRate.SetValue(50);
//        Camera->ExposureTime.SetValue(400);
//       // Camera->PgiMode.SetValue(PgiMode_Off);
//    //   Camera->SharpnessEnhancement.SetValue(1.5);
////        Camera->NoiseReduction.SetValue(0.5);


//      // emit CameraReady();         // теперь можем установить режим по прерыванию
//        Camera->PixelFormat.SetValue( PixelFormat_Mono8 );

//        Camera->Width.SetValue(1600);
//        Camera->Height.SetValue(700);
//        Camera->OffsetX.SetValue(0);
//        Camera->OffsetY.SetValue(offsetY);

//                //Camera->LineDebouncerTime.SetValue(5);
//        Camera->TriggerSource.SetValue(TriggerSource_Software);
////        TriggerSourceEnums e = Camera->TriggerSource.GetValue();
////        Camera->TriggerSelector.SetValue( TriggerSelector_FrameStart );
////        Camera->TriggerMode.SetValue( TriggerMode_On );


//}

void CameraConnection::StopGrab()
{
    isAbort = true;
}

void CameraConnection::setReqestFalse()
{
    request = false;
}

void CameraConnection::setReqestTrue()
{
    request = true;
}

bool CameraConnection::isOpenCamera()
{
    if (Camera->IsOpen())
        return true;
    else
        return false;
}
int CameraConnection::getExpos() const
{
    return expos;
}

void CameraConnection::setExpos(int value)
{
    expos = value;
    Camera->ExposureTime.SetValue(expos);
}
QString CameraConnection::getSerialNum() const
{
    return serialNum;
}

void CameraConnection::setSerialNum(const QString &value)
{
    serialNum = value;
}
bool CameraConnection::getLineInv() const
{
    return LineInv;
}

void CameraConnection::setLineInv(bool value)
{
    LineInv = value;
}

void CameraConnection::setOffsetY(int value)
{
    offsetY = value;
    Camera->OffsetY.SetValue(offsetY);
}
