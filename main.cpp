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

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <iostream>
#include <vector>
#include <string>

#include "marker.h"
#include "page.h"

void process_image(IplImage *src_img,
        std::map<int, CvPoint2D32f> &left_dst_markers,
        LayoutInfo left_layout,
        std::map<int, CvPoint2D32f> &right_dst_markers,
        LayoutInfo right_layout)
{
    BookImage book_image(src_img);

    {
        IplImage *dst_img
                = book_image.create_page_image(left_dst_markers, left_layout);
        if (dst_img != NULL) {
            cvShowImage("Left", dst_img);
            cvReleaseImage(&dst_img);
        }
    }

    {
        IplImage *dst_img
                = book_image.create_page_image(right_dst_markers, right_layout);
        if (dst_img != NULL) {
            cvShowImage("Right", dst_img);
            cvReleaseImage(&dst_img);
        }
    }
}

int main(int argc, char **argv)
{
    // Configure left page.
    std::map<int, CvPoint2D32f> left_dst_markers;
    left_dst_markers[0] = cvPoint2D32f(0.00, 0.00);
    left_dst_markers[1] = cvPoint2D32f(6.00, 0.00);
    left_dst_markers[2] = cvPoint2D32f(6.00, 9.50);
    left_dst_markers[3] = cvPoint2D32f(0.00, 9.50);
    LayoutInfo left_layout;
    left_layout.page_left = 0.50;
    left_layout.page_top = 0.25;
    left_layout.page_right = 6.30;
    left_layout.page_bottom = 9.20;
    left_layout.dpi = 600.0;

    // Configure right page.
    std::map<int, CvPoint2D32f> right_dst_markers;
    right_dst_markers[4] = cvPoint2D32f(0.00, 0.00);
    right_dst_markers[5] = cvPoint2D32f(6.00, 0.00);
    right_dst_markers[6] = cvPoint2D32f(6.00, 9.50);
    right_dst_markers[7] = cvPoint2D32f(0.00, 9.50);
    LayoutInfo right_layout;
    right_layout.page_left = -0.30;
    right_layout.page_top = 0.25;
    right_layout.page_right = 5.50;
    right_layout.page_bottom = 9.20;
    right_layout.dpi = 600.0;

    // Process if an input image is supplied; otherwise, open a webcam for
    // debugging.
    if (argc > 3) {
        IplImage *src_img = cvLoadImage(argv[1]);
        if (src_img == NULL) {
            std::cerr << "Failed to load the source image specified.\n";
            return 1;
        }

        BookImage book_img(src_img);

        IplImage *left_img
                = book_img.create_page_image(left_dst_markers, left_layout);
        if (left_img != NULL) {
            cvSaveImage(argv[2], left_img);
            cvReleaseImage(&left_img);
        }

        IplImage *right_img
                = book_img.create_page_image(right_dst_markers, right_layout);
        if (right_img != NULL) {
            cvSaveImage(argv[3], right_img);
            cvReleaseImage(&right_img);
        }

        cvReleaseImage(&src_img);
    } else {
        // Create windows.
        cvNamedWindow("Source", 0);
        cvResizeWindow("Source", 480, 640);

        left_layout.dpi = 100;
        right_layout.dpi = 100;

        // Open webcam.
        CvCapture* capture = cvCreateCameraCapture(0);
        if (!capture) {
            std::cerr << "Failed to load the camera device.\n";
            return 1;
        }
        const double scale = 1.0;
        cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1600 * scale);
        cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 1200 * scale);

        while (cvWaitKey(10) < 0) {
            IplImage *src_img = cvQueryFrame(capture);
            cvShowImage("Source", src_img);
            process_image(src_img,
                    left_dst_markers, left_layout,
                    right_dst_markers, right_layout);
        }
    }

    return 0;
}
