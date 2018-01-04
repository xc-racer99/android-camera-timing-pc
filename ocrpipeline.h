#ifndef OCRPIPELINE_H
#define OCRPIPELINE_H

#include <QObject>
#include <QQueue>
#include <QString>

#ifndef NO_OCR
#include "3rd-Party/bibnumber/pipeline.h"
#include "3rd-Party/bibnumber/textdetection.h"
#else
namespace DetectText {
struct TextDetectionParams {
    bool darkOnLight;
    int maxStrokeLength;
    int minCharacterheight;
    float maxImgWidthToTextRatio;
    float maxAngle;
    int topBorder;
    int bottomBorder;
    unsigned int minChainLen;
    int modelVerifLenCrit;
    int modelVerifMinHeight;
    bool useOriginalChainCode;
};
}
#endif

class OcrPipeline : public QObject
{
    Q_OBJECT
public:
    explicit OcrPipeline(QString imagesDir, bool fromBack, QObject *parent = 0);

    DetectText::TextDetectionParams getParams();
private:
    QString directory;
    QString svmModel;
    QQueue<QString> fileNames;

    bool fromBehind;
    bool running;

#ifndef NO_OCR
    pipeline::Pipeline pipeline;
#endif

    DetectText::TextDetectionParams params;
signals:
    void finished();
    void newImage(QString filename, int bibNumber);
public slots:
    void addImage(QString filename);
    void process();
    void setFromBehind(bool fromBack);
    void setParams(DetectText::TextDetectionParams parameters);
    void stopThread();
};

#endif // OCRPIPELINE_H
