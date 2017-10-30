#ifndef PIPELINE_H
#define PIPELINE_H

#include "opencv2/imgproc/imgproc.hpp"
#include "textdetection.h"
#include "textrecognition.h"

namespace pipeline
{
	class Pipeline {
	public:
		int processImage(cv::Mat& img, std::string svmModel, std::vector<int>& bibNumbers);
		int processImage(cv::Mat& img, std::string svmModel, std::vector<int>& bibNumbers, DetectText::TextDetectionParams params);
	private:
		textrecognition::TextRecognizer textRecognizer;
	};

}

#endif /* #ifndef PIPELINE_H */

