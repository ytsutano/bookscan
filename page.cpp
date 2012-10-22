#include <iostream>
#include <map>

#include "page.h"
#include "marker.h"

BookImage::BookImage(const IplImage *src_img) : src_img(cvCloneImage(src_img))
{
    // Create grayscale image.
    IplImage *gray_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_8U, 1);
    cvCvtColor(src_img, gray_img, CV_BGR2GRAY);

    // Threshold.
    IplImage *bw_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_8U, 1);
    cvAdaptiveThreshold(gray_img, bw_img, 128,
            CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 11+20, 8);

    // Find contours.
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq *contour;
    cvFindContours(bw_img, storage, &contour, sizeof(CvContour),
            CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));

    // Examine each contour that was found.
    for(; contour != 0; contour = contour->h_next) {
        CvSeq *poly = cvApproxPoly(contour, sizeof(CvContour), NULL,
                CV_POLY_APPROX_DP, 6.0);
        // Make sure that contour is quadrilateral and convex.
        if (poly->total != 4 || !cvCheckContourConvexity(poly)) {
            continue;
        }

        CvPoint2D32f points[4];
        int marker_id = analyze_marker(gray_img, poly, points);
        if (marker_id != -1) {
            src_markers[marker_id] = points[0];
        }
    }

    // Clean up.
    cvReleaseMemStorage(&storage);
    cvReleaseImage(&gray_img);
    cvReleaseImage(&bw_img);
}

BookImage::~BookImage()
{
    // Clean up.
    cvReleaseImage(&src_img);
}

IplImage *BookImage::create_page_image(
        const std::map<int, CvPoint2D32f> &dst_markers,
        CvSize dst_size)
{
    // Make sure more than 4 makers are provided.
    if (dst_markers.size() < 4) {
        return NULL;
    }

    // Create matrices for perspective transform.
    CvMat *src_points = cvCreateMat(dst_markers.size(), 2, CV_64FC1);
    CvMat *dst_points = cvCreateMat(dst_markers.size(), 2, CV_64FC1);

    typedef std::map<int, CvPoint2D32f>::const_iterator MMCIT;
    int row = 0;
    for (MMCIT dit = dst_markers.begin(); dit != dst_markers.end(); ++dit) {
        // Find a source marker with the specified ID.
        MMCIT sit = src_markers.find(dit->first);

        // Make sure the marker specified exists.
        if (sit == src_markers.end()) {
            // Couldn't find the marker: clean up and return NULL.
            cvReleaseMat(&src_points);
            cvReleaseMat(&dst_points);
            std::cout << "Couldn't find " << dit->first << "\n";
            return NULL;
        }

        std::cout << dit->first << " ";

        // Update the matrice.
        cvmSet(src_points, row, 0, sit->second.x);
        cvmSet(src_points, row, 1, sit->second.y);
        cvmSet(dst_points, row, 0, dit->second.x);
        cvmSet(dst_points, row, 1, dit->second.y);

        row++;
    }
    std::cout << "\n";

    // Create destination image.
    IplImage *dst_image = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);

    // Compute homography matrix.
    CvMat *h = cvCreateMat(3, 3, CV_64FC1);
    cvFindHomography(src_points, dst_points, h);

    // Transform perspective.
    cvWarpPerspective(src_img, dst_image, h);

    // Clean up.
    cvReleaseMat(&src_points);
    cvReleaseMat(&dst_points);
    cvReleaseMat(&h);

    return dst_image;
}

IplImage *BookImage::create_page_image(
        const std::map<int, CvPoint2D32f> &dst_markers,
        const LayoutInfo &layout)
{
    // Get the destination image size in pixel.
    double page_width_px = (layout.page_right - layout.page_left) * layout.dpi;
    double page_height_px = (layout.page_bottom - layout.page_top) * layout.dpi;
    CvSize pageSize = cvSize(
            static_cast<int>(page_width_px + 0.5),
            static_cast<int>(page_height_px + 0.5));

    std::printf("%4.2f %4.2f\n", page_width_px, page_height_px);

    // Convert marker positions to pixel representation.
    std::map<int, CvPoint2D32f> dst_markers_px;
    typedef std::map<int, CvPoint2D32f>::const_iterator MMCIT;
    for (MMCIT dit = dst_markers.begin(); dit != dst_markers.end(); ++dit) {
        dst_markers_px[dit->first] = cvPoint2D32f(
                (dit->second.x - layout.page_left) * layout.dpi,
                (dit->second.y - layout.page_top) * layout.dpi);
    }

    // Return page image.
    return create_page_image(dst_markers_px, pageSize);
}
