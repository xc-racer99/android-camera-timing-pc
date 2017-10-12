#include <QFile>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "pipeline.h"

#include "ocrpipeline.h"

OcrPipeline::OcrPipeline(QString fileName, QString directoryName, bool fromBack, QObject *parent) : QObject(parent)
{
    file = fileName;
    directory = directoryName;
    fromBehind = fromBack;
}

void OcrPipeline::process() {
    // Run the OCR
    cv::Mat image = cv::imread(file.toLatin1().constData(), 1);
    int bibNumber = 0;
    if (image.empty()) {
        qDebug("ERROR:Failed to open image file %s", file.toLatin1().constData());
    } else {
        pipeline::Pipeline pipeline;
        pipeline.setDirectory((directory + "../../tempImages/").toLatin1().constData());
        std::vector<int> bibNumbers;
        QString svmModel = "";
        QFile svmFile(directory + "../../svm.xml");
        if(svmFile.exists())
            svmModel = directory + "../../svm.xml";
        pipeline.processImage(image, svmModel.toLatin1().constData(), /*darkOnLight*/ 1, bibNumbers);
        if(!bibNumbers.empty()) {
            if(fromBehind)
                bibNumber = bibNumbers.back();
            else
                bibNumber = bibNumbers.front();
        }
    }

    emit newImage(file, bibNumber);
}

