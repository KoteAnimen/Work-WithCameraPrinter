#include "barcodeprocessing.h"

#include <QDateTime>
#include <QDir>

#include <exception>
#include <stdio.h>

#include <targetver.h>
#include "stdafx.h"
#include "atlbase.h"

using namespace  Bytescout_BarCodeReader;

BarCodeProcessing::BarCodeProcessing()
{
    lastGoodFrame=0;
    frameToSave = 0;
    currentweek = QDate::currentDate().weekNumber();

    if(!QDir("C:/cadr/CurrentWeek").exists())
        QDir().mkdir("C:/cadr/CurrentWeek");

    // Create the interface pointer.
    pIReader=IReaderPtr(__uuidof(Reader));

    BSTR regname = ::SysAllocString(L"1_DEV_BYTESCOUT_BARCODE_READER_SDK_OEM_DEVELOPER_LICENSE_ACTIVATION_1_SRV_VCPU16_UNLIM_PAGES_MNT_UNTIL_AUGUST_15_2021_TARSIS_ULTRA@MAIL.RU");
    pIReader->put_RegistrationName(regname);
    SysFreeString(regname);
    BSTR regkey = ::SysAllocString(L"34C3-D156-5598-A3DB-0AE8-5875-A6A");
    pIReader->put_RegistrationKey(regkey);
    SysFreeString(regkey);

    _BarcodeTypeSelectorPtr pBarcodeTypesToFind;
    pIReader->get_BarcodeTypesToFind(&pBarcodeTypesToFind);
    pBarcodeTypesToFind->put_DataMatrix(VARIANT_TRUE);

    pIReader->put_ColorConversionMode(ColorConversionMode_ImageBlocks);

    _ImagePreprocessingFiltersCollection* pIImageFilters;
    pIReader->get_ImagePreprocessingFilters(&pIImageFilters);
}

int BarCodeProcessing::getLastGoodFrame() const
{
    return lastGoodFrame;
}

void BarCodeProcessing::setLastGoodFrame(int value)
{
    lastGoodFrame = value;
}
int BarCodeProcessing::getFrameToSave() const
{
    return frameToSave;
}

void BarCodeProcessing::setFrameToSave(int value)
{
    frameToSave = value;
}
QString BarCodeProcessing::getFolderName() const
{
    return FolderName;
}

void BarCodeProcessing::setFolderName(const QString &value)
{
    FolderName = value;
    if(!QDir("C:/cadr/CurrentWeek/"+FolderName).exists())
        QDir().mkdir("C:/cadr/CurrentWeek/"+FolderName);
}
bool BarCodeProcessing::getQrFound() const
{
    return qrFound;
}

void BarCodeProcessing::setQrFound(bool value)
{
    qrFound = value;
}

int BarCodeProcessing::getMainThresh() const
{
    return MainThresh;
}

void BarCodeProcessing::setMainThresh(int value)
{
    MainThresh = value;
}

void BarCodeProcessing::writemat()
{
    QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy");

    if(!QDir("C:/cadr/CurrentWeek/"+FolderName+"/"+date).exists())
        QDir().mkdir("C:/cadr/CurrentWeek/"+FolderName+"/"+date);

    std::string date_of_incident = QDateTime::currentDateTime().toString("dd_MM hh_mm_ss").toStdString();

    QDir().mkdir("C:/cadr/CurrentWeek/"+FolderName+"/"+date+"/"+QDateTime::currentDateTime().toString("dd_MM hh_mm_ss"));

    for (int i=0;i<mv.size();i++)
    {
        cv::Mat wrmat = mv[i];
        // cv::resize(wrmat,wrmat,cv::Size(640,480));
        cv::String img_file_name = "C:/cadr/CurrentWeek/"+FolderName.toStdString()+"/"+date.toStdString()+"/"+date_of_incident+"/"+QString::number(i).toStdString()+".jpg";
        cv::imwrite(img_file_name,wrmat);
    }
}

void BarCodeProcessing::UpdateDB()
{
    QDir dir("C:/cadr/PreviousWeek");
    dir.removeRecursively();

    QDir dir1("C:/cadr");
    dir1.rename("CurrentWeek","PreviousWeek");

    if(!QDir("C:/cadr/CurrentWeek").exists())
    {
        QDir().mkdir("C:/cadr/CurrentWeek");
        QDir().mkdir("C:/cadr/CurrentWeek/cam1");
        QDir().mkdir("C:/cadr/CurrentWeek/cam2");
    }

    currentweek = QDate::currentDate().weekNumber();
}

bool compareContourAreas (std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 )
{
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i < j );
}

void BarCodeProcessing::ProcessFrame(cv::Mat frame)
{
    lastGoodFrame++;
    if(lastGoodFrame%2==0)
    {
        cv::Mat to_write = frame.clone();
        mv.push_back(to_write);
    }

    cv::Mat draw = frame.clone();

    QString workFolder = "C:/"+FolderName;
    cv::imwrite(workFolder.toStdString()+"/1.png",frame);

    cvtColor(draw,draw,cv::COLOR_GRAY2BGR);
    QString filename = workFolder + "1.png";

    HRESULT hr = CoInitialize(NULL);

    WCHAR file[MAX_PATH];

    if(FolderName=="cam1")
        ::GetFullPathName(L"C:/cam1/1.png", MAX_PATH, file, NULL);

    else
        ::GetFullPathName(L"C:/cam2/1.png", MAX_PATH, file, NULL);

    // Read barcode from file
    hr = pIReader->ReadFromFile(_bstr_t(file));
    // Get full path of sample barcode image file

    // Get found barcode count
    long count;
    pIReader->get_FoundCount(&count);

    // Get found barcode information
    for (int i = 0; i < count; i++)
    {
        SymbologyType type;
        hr = pIReader->GetFoundBarcodeType(i, &type);

        float confidence;
        hr = pIReader->GetFoundBarcodeConfidence(i, &confidence);

        if(confidence>0)
            correctCode = 1;
        else
            correctCode = 2;


        BSTR bstrValue;
        hr = pIReader->GetFoundBarcodeValue(i, &bstrValue);

        long t;
        hr = pIReader->GetFoundBarcodeTop(i,&t);

        long l;
        hr = pIReader->GetFoundBarcodeLeft(i,&l);

        long w;
        hr = pIReader->GetFoundBarcodeWidth(i,&w);

        long h;
        hr = pIReader->GetFoundBarcodeHeight(i,&h);

        QString res = QString::fromWCharArray(bstrValue);
        cv::Rect rec = cv::Rect(l,t,w,h);
        cv::rectangle(draw,rec,cv::Scalar(255,255,0),3);

        cv::circle(draw,cv::Point(l,t),5,cv::Scalar(255,0,0),4);
        cv::putText(draw,res.toStdString().c_str(),cv::Point(200,200),3,cv::FONT_HERSHEY_SIMPLEX,cv::Scalar(255,0,255),3);
        if(res!=LastRes)
        {
            emit CodeQr(res);
            LastRes=res;
        }
    }
    // Uninitialize COM.
    CoUninitialize();
    emit ThreshMat(draw);
    emit ProcessingRequest(true);
    //emit ElapsedTime(frameToSave);
}
