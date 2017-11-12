/**
 * Copyright (C) 2017  Jonathan Bakker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <QCoreApplication>
#include <QFile>
#include <QThread>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "ocrpipeline.h"

OcrPipeline::OcrPipeline(QString imagesDir, bool fromBack, QObject *parent) : QObject(parent)
{
    directory = imagesDir;
    fromBehind = fromBack;
    running = true;

    params = { true, /* darkOnLight */
               15, /* maxStrokeLength */
               11, /* minCharacterHeight */
               100, /* maxImgWidthToTextRatio */
               45, /* maxAngle */
               10, /* topBorder: discard top 10% */
               5,  /* bottomBorder: discard bottom 5% */
               3, /* min chain len */
               0, /* verify with SVM model up to this chain len */
               0, /* height needs to be this large to verify with model */
               false /* use new chaining code */
    };

    pipeline.setDirectory(QString(directory + "../../tempImages").toLatin1().constData());

    svmModel = "";
    QFile svmFile(directory + "../../svm.xml");
    if(svmFile.exists()) {
        svmModel = directory + "../../svm.xml";
        params.minChainLen = 2;
        params.modelVerifLenCrit = 2;
        params.modelVerifMinHeight = 15;
    }
}

void OcrPipeline::addImage(QString filename) {
    fileNames.enqueue(filename);
}

DetectText::TextDetectionParams OcrPipeline::getParams() {
    return params;
}

void OcrPipeline::process() {
    while(running) {
        if(!fileNames.empty()) {
            // Run the OCR
            QString file = fileNames.dequeue();
            cv::Mat image = cv::imread(file.toLatin1().constData(), 1);
            int bibNumber = 0;
            if (image.empty()) {
                qDebug("ERROR:Failed to open image file %s", file.toLatin1().constData());
            } else {
                std::vector<int> bibNumbers;
                pipeline.processImage(image, svmModel.toLatin1().constData(), bibNumbers, params);
                if(!bibNumbers.empty()) {
                    if(fromBehind)
                        bibNumber = bibNumbers.back();
                    else
                        bibNumber = bibNumbers.front();
                }
            }
            emit newImage(file, bibNumber);
        } else {
            // Nothing in queue, wait 1s and check again
            QThread::sleep(1);
        }
        QCoreApplication::processEvents();
    }
    emit finished();
}

void OcrPipeline::setFromBehind(bool fromBack) {
    fromBehind = fromBack;
}

void OcrPipeline::setParams(DetectText::TextDetectionParams parameters) {
    params = parameters;
}

void OcrPipeline::stopThread() {
    running = false;
}
