#ifndef TEXTREC_H
#define TEXTREC_H

#include <tesseract/baseapi.h>

#include "opencv2/imgproc/imgproc.hpp"

#include "textdetection.h"

namespace textrecognition
{
	class TextRecognizer {
	public:
		TextRecognizer(void);
		~TextRecognizer(void);
		void setOutputDirectory(std::string dir);
		int recognize (cv::Mat input,
	   	               const struct DetectText::TextDetectionParams params,
	   	               std::string svmModel,
		               std::vector<DetectText::Chain> &chains,
			           std::vector<DetectText::SWTPointPair2d > &compBB,
			           std::vector<DetectText::SWTPointPair2i > &chainBB,
			           std::vector<std::string> &text);
	private:
		tesseract::TessBaseAPI tess;
		int dsid; /* digit sequence id */
		int bsid; /* bib sequence id */
		std::string directory; /* dir for temp images */
	};

}

#endif /* #ifndef TEXTREC_H */

