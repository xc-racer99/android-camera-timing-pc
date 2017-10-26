#ifndef OCRPIPELINE_H
#define OCRPIPELINE_H

#include <QObject>
#include <QQueue>
#include <QString>

#include "3rd-Party/bibnumber/pipeline.h"

class OcrPipeline : public QObject
{
    Q_OBJECT
public:
    explicit OcrPipeline(QString imagesDir, bool fromBack, QObject *parent = 0);
private:
    QString directory;
    QString svmModel;
    QQueue<QString> fileNames;

    bool fromBehind;
    bool running;

    pipeline::Pipeline pipeline;
signals:
    void finished();
    void newImage(QString filename, int bibNumber);
public slots:
    void addImage(QString filename);
    void process();
    void setFromBehind(bool fromBack);
    void stopThread();
};

#endif // OCRPIPELINE_H
