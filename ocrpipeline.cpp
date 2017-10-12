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

