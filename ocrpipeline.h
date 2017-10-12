#ifndef OCRPIPELINE_H
#define OCRPIPELINE_H

#include <QObject>
#include <QString>

class OcrPipeline : public QObject
{
    Q_OBJECT
public:
    explicit OcrPipeline(QString fileName, QString directoryName, bool fromBack, QObject *parent = 0);
private:
    QString file;
    QString directory;
    bool fromBehind;
signals:
    void newImage(QString filename, int bibNumber);
public slots:
    void process();
};

#endif // OCRPIPELINE_H
