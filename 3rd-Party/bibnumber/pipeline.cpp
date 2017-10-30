#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <boost/algorithm/string/trim.hpp>

#include "pipeline.h"
#include "textdetection.h"

#include "stdio.h"

namespace pipeline {

static void vectorAtoi(std::vector<int>&numbers, std::vector<std::string>&text)
{
	for (std::vector<std::string>::iterator it = text.begin(); it != text.end();
					it++) {
		boost::algorithm::trim(*it);
		numbers.push_back(atoi(it->c_str()));
	}
}

int Pipeline::processImage(
		cv::Mat& img,
		std::string svmModel,
		std::vector<int>& bibNumbers) {
	std::vector<std::string> text;
	struct DetectText::TextDetectionParams params = {
						true, /* darkOnLight */
						15, /* maxStrokeLength */
						11, /* minCharacterHeight */
						100, /* maxImgWidthToTextRatio */
						45, /* maxAngle */
						10, /* topBorder: discard top 10% */
						5,  /* bottomBorder: discard bottom 5% */
						3, /* min chain len */
						0, /* verify with SVM model up to this chain len */
						0, /* height needs to be this large to verify with model */
						false, /* use gheinrich's chain code */
				};

	if (!svmModel.empty())
	{
		/* lower min chain len */
		params.minChainLen = 2;
		/* verify with SVM model up to this chain len */
		params.modelVerifLenCrit = 2;
		/* height needs to be this large to verify with model */
		params.modelVerifMinHeight = 15;
	}

	std::vector<DetectText::Chain> chains;
	std::vector<DetectText::SWTPointPair2d > compBB;
	std::vector<DetectText::SWTPointPair2i > chainBB;
	DetectText::textDetection(img, params, chains, compBB, chainBB);
	textRecognizer.recognize(img, params, svmModel, chains, compBB, chainBB, text);
	vectorAtoi(bibNumbers, text);

	return 0;
}

} /* namespace pipeline */

