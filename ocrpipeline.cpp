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

    pipeline.setDirectory(QString(directory + "../../tempImages").toLatin1().constData());

    svmModel = "";
    QFile svmFile(directory + "../../svm.xml");
    if(svmFile.exists())
        svmModel = directory + "../../svm.xml";
}

void OcrPipeline::addImage(QString filename) {
    fileNames.enqueue(filename);
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
                pipeline.processImage(image, svmModel.toLatin1().constData(), /*darkOnLight*/ 1, bibNumbers);
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
    }
    emit finished();
}

void OcrPipeline::setFromBehind(bool fromBack) {
    fromBehind = fromBack;
}

void OcrPipeline::stopThread() {
    running = false;
}
