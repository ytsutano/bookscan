#include "marker.h"

int decode_marker(CvMat *mark_mat, marker_rotation_t &rotation)
{
    // Make sure that the outermost cells are black.
    for (int i = 0; i < 6; i++) {
        if (cvmGet(mark_mat, i, 0) > 0.5
                    || cvmGet(mark_mat, i, 5) > 0.5
                    || cvmGet(mark_mat, 0, i) > 0.5
                    || cvmGet(mark_mat, 5, i) > 0.5) {
            return -1;
        }
    }

    // Make sure that the next cells of the outermost cells are white.
    if (cvmGet(mark_mat, 2, 1) < 0.5
                || cvmGet(mark_mat, 3, 1) < 0.5
                || cvmGet(mark_mat, 1, 2) < 0.5
                || cvmGet(mark_mat, 1, 3) < 0.5
                || cvmGet(mark_mat, 2, 4) < 0.5
                || cvmGet(mark_mat, 3, 4) < 0.5
                || cvmGet(mark_mat, 4, 2) < 0.5
                || cvmGet(mark_mat, 4, 3) < 0.5) {
        return -1;
    }

    // Make sure that the number of corner marker is exactly one. Also
    // detect the orientation.
    int numberOfCornerMarkers = 0;
    if (cvmGet(mark_mat, 1, 1) < 0.5) {
        numberOfCornerMarkers++;
        rotation = MARKER_ROT_0_DEG;
    }
    if (cvmGet(mark_mat, 1, 4) < 0.5) {
        numberOfCornerMarkers++;
        rotation = MARKER_ROT_90_DEG;
    }
    if (cvmGet(mark_mat, 4, 4) < 0.5) {
        numberOfCornerMarkers++;
        rotation = MARKER_ROT_180_DEG;
    }
    if (cvmGet(mark_mat, 4, 1) < 0.5) {
        numberOfCornerMarkers++;
        rotation = MARKER_ROT_270_DEG;
    }
    if (numberOfCornerMarkers != 1) {
        return -1;
    }

    int id = 0;
    switch (rotation) {
    case MARKER_ROT_0_DEG:
        id = ((cvmGet(mark_mat, 2, 2) < 0.5) << 3)
                | ((cvmGet(mark_mat, 2, 3) < 0.5) << 2)
                | ((cvmGet(mark_mat, 3, 2) < 0.5) << 1)
                | ((cvmGet(mark_mat, 3, 3) < 0.5) << 0);
        break;
    case MARKER_ROT_90_DEG:
        id = ((cvmGet(mark_mat, 2, 2) < 0.5) << 1)
                | ((cvmGet(mark_mat, 2, 3) < 0.5) << 3)
                | ((cvmGet(mark_mat, 3, 2) < 0.5) << 0)
                | ((cvmGet(mark_mat, 3, 3) < 0.5) << 2);
        break;
    case MARKER_ROT_180_DEG:
        id = ((cvmGet(mark_mat, 2, 2) < 0.5) << 0)
                | ((cvmGet(mark_mat, 2, 3) < 0.5) << 1)
                | ((cvmGet(mark_mat, 3, 2) < 0.5) << 2)
                | ((cvmGet(mark_mat, 3, 3) < 0.5) << 3);
        break;
    case MARKER_ROT_270_DEG:
        id = ((cvmGet(mark_mat, 2, 2) < 0.5) << 2)
                | ((cvmGet(mark_mat, 2, 3) < 0.5) << 0)
                | ((cvmGet(mark_mat, 3, 2) < 0.5) << 3)
                | ((cvmGet(mark_mat, 3, 3) < 0.5) << 1);
        break;
    }

    // Now determine the id using a table.
    static const unsigned char id_table[] = {
        8, 2, 4, 15, 6, 13, 11, 1,
        0, 10, 12, 7, 14, 5, 3, 9
    };
    id = id_table[id];

    return id;
}

int analyze_marker(const IplImage *src_img, CvSeq *poly, CvPoint2D32f *points)
{
    // Make sure the shape is square and convex.
    if (poly->total != 4
            || !cvCheckContourConvexity(poly)
            || cvContourArea(poly) < 360.0) {
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        points[i] = cvPointTo32f(*CV_GET_SEQ_ELEM(CvPoint, poly, i));
    }

    // Refine to sub-pixel accuracy.
    cvFindCornerSubPix(src_img, points, 4, cvSize(3, 3), cvSize(-1, -1),
             cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

    double a[] = {
        points[0].x, points[0].y,
        points[1].x, points[1].y,
        points[2].x, points[2].y,
        points[3].x, points[3].y
    };
    CvMat src_points = cvMat(poly->total, 2, CV_64FC1, a);

    static const int MARK_WIDTH = 6*3;
    static const int MARK_HEIGHT = 6*3;
    static const int MARK_TEMP_WIDTH = MARK_WIDTH * 10;
    static const int MARK_TEMP_HEIGHT = MARK_HEIGHT * 10;
    IplImage *mark_temp_img = cvCreateImage(
            cvSize(MARK_TEMP_WIDTH, MARK_TEMP_HEIGHT), IPL_DEPTH_8U, 1);
    double b[] = {
        0, 0,
        0, MARK_TEMP_HEIGHT,
        MARK_TEMP_WIDTH, MARK_TEMP_HEIGHT,
        MARK_TEMP_WIDTH, 0,
    };
    CvMat mark_points = cvMat(poly->total, 2, CV_64FC1, b);

    // Compute homography matrix.
    CvMat *h = cvCreateMat(3, 3, CV_64FC1);
    cvFindHomography(&src_points, &mark_points, h);

    // Transform perspective.
    cvWarpPerspective(src_img, mark_temp_img, h);

    IplImage *mark_img = cvCreateImage(
            cvSize(MARK_WIDTH, MARK_HEIGHT), IPL_DEPTH_8U, 1);
    cvResize(mark_temp_img, mark_img, CV_INTER_AREA);
    int threshold = cvAvg(mark_img).val[0];
    cvThreshold(mark_img, mark_img, threshold, 255, CV_THRESH_BINARY);
    for (int i = 0; i < MARK_WIDTH; i++) {
        for (int j = 0; j < MARK_HEIGHT; j++) {
            if ((i % 3) != 1 || (j % 3) != 1) {
                cvSetReal2D(mark_img, i, j, threshold);
            }
        }
    }

    // Create marker matrix.
    CvMat *mark_mat = cvCreateMat(6, 6, CV_64FC1);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            cvmSet(mark_mat, i, j, (cvGetReal2D(mark_img, i*3+1, j*3+1) > 128));
        }
    }

    // Decode the marker ID.
    marker_rotation_t rotation;
    int marker_id = decode_marker(mark_mat, rotation);

    // Based on rotation, correct the points array so that it starts from
    // the corner with the rotation dot.
    CvPoint2D32f temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = points[i];
    }
    for (int i = 0; i < 4; i++) {
        points[i] = temp[(i + rotation) % 4];
    }

    // Show decoded marker image for debugging.
    /*
    if (marker_id != -1) {
        cvResize(mark_img, mark_temp_img, CV_INTER_AREA);
        cvShowImage("Window", mark_temp_img);
    }
    //*/

    // Clean up.
    cvReleaseMat(&h);
    cvReleaseMat(&mark_mat);
    cvReleaseImage(&mark_img);
    cvReleaseImage(&mark_temp_img);

    return marker_id;
}
