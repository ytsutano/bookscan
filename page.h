#ifndef _PAGE_H
#define _PAGE_H

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <map>

struct LayoutInfo
{
    double page_left;
    double page_top;
    double page_right;
    double page_bottom;
    double dpi;
};

class BookImage
{
private:
    IplImage *src_img;
    std::map<int, CvPoint2D32f> src_markers;

public:
    BookImage(const IplImage *src_img);
    ~BookImage();
    IplImage *create_page_image(const std::map<int, CvPoint2D32f> &dst_markers,
            CvSize dst_size);
    IplImage *create_page_image(
            const std::map<int, CvPoint2D32f> &dst_markers,
            const LayoutInfo &layout);
};

#endif
