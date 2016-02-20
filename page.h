/*
 * Copyright (c) 2012, 2016, Yutaka Tsutano
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

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
