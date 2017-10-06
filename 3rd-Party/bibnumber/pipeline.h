#ifndef PIPELINE_H
#define PIPELINE_H

#include "opencv2/imgproc/imgproc.hpp"
#include "textdetection.h"
#include "textrecognition.h"

namespace pipeline
{
	class Pipeline {
	public:
		Pipeline(void);
		~Pipeline(void);
		void setDirectory(std::string dir);
		int processImage(cv::Mat& img, std::string svmModel, int darkOnLight, std::vector<int>& bibNumbers);
	private:
		textdetection::TextDetector textDetector;
		textrecognition::TextRecognizer textRecognizer;
		std::string directory;
	};

}

#endif /* #ifndef PIPELINE_H */

