#ifndef _MARKER_H
#define _MARKER_H

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc_c.h>

typedef enum {
    MARKER_ROT_0_DEG,
    MARKER_ROT_270_DEG,
    MARKER_ROT_180_DEG,
    MARKER_ROT_90_DEG,
} marker_rotation_t;

int analyze_marker(const IplImage *src_img, CvSeq *poly, CvPoint2D32f *points);

#endif
