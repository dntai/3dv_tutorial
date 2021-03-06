#include "opencv_all.hpp"

int main(void)
{
    cv::Mat K = (cv::Mat_<double>(3, 3) << 432.7390364738057, 0, 476.0614994349778, 0, 431.2395555913084, 288.7602152621297, 0, 0, 1);
    cv::Mat dist_coeff = (cv::Mat_<double>(5, 1) << -0.2852754904152874, 0.1016466459919075, -0.0004420196146339175, 0.0001149909868437517, -0.01803978785585194);
    cv::Size board_pattern(10, 7);
    double board_cellsize = 0.025;

    // Open an video
    cv::VideoCapture video;
    if (!video.open("data/chessboard.avi")) return -1;

    // Prepare a box for simple AR
    std::vector<cv::Point3d> box_lower, box_upper;
    box_lower.push_back(cv::Point3d(4 * board_cellsize, 2 * board_cellsize, 0));
    box_lower.push_back(cv::Point3d(5 * board_cellsize, 2 * board_cellsize, 0));
    box_lower.push_back(cv::Point3d(5 * board_cellsize, 4 * board_cellsize, 0));
    box_lower.push_back(cv::Point3d(4 * board_cellsize, 4 * board_cellsize, 0));
    box_upper.push_back(cv::Point3d(4 * board_cellsize, 2 * board_cellsize, -board_cellsize));
    box_upper.push_back(cv::Point3d(5 * board_cellsize, 2 * board_cellsize, -board_cellsize));
    box_upper.push_back(cv::Point3d(5 * board_cellsize, 4 * board_cellsize, -board_cellsize));
    box_upper.push_back(cv::Point3d(4 * board_cellsize, 4 * board_cellsize, -board_cellsize));

    // Run pose estimation
    std::vector<cv::Point3d> object_points;
    for (int r = 0; r < board_pattern.height; r++)
        for (int c = 0; c < board_pattern.width; c++)
            object_points.push_back(cv::Point3d(board_cellsize * c, board_cellsize * r, 0));
    while (true)
    {
        // Grab an image from the video
        cv::Mat image;
        video >> image;
        if (image.empty()) break;

        // Estimate camera pose
        std::vector<cv::Point2d> image_points;
        bool complete = cv::findChessboardCorners(image, board_pattern, image_points, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if (complete)
        {
            cv::Mat rvec, tvec;
            cv::solvePnP(object_points, image_points, K, dist_coeff, rvec, tvec);

            // Draw the box on the image
            cv::Mat line_lower, line_upper;
            cv::projectPoints(box_lower, rvec, tvec, K, dist_coeff, line_lower);
            cv::projectPoints(box_upper, rvec, tvec, K, dist_coeff, line_upper);
            line_lower.reshape(1).convertTo(line_lower, CV_32S); // Change 4 x 1 matrix (CV_64FC2) to 4 x 2 matrix (CV_32SC1)
            line_upper.reshape(1).convertTo(line_upper, CV_32S); // Because 'cv::polylines()' only accepts 'CV_32S' depth.
            cv::polylines(image, line_lower, true, cv::Scalar(255, 0, 0), 2);
            for (int i = 0; i < line_lower.rows; i++)
                cv::line(image, cv::Point(line_lower.row(i)), cv::Point(line_upper.row(i)), cv::Scalar(0, 255, 0), 2);
            cv::polylines(image, line_upper, true, cv::Scalar(0, 0, 255), 2);

            // Print camera position
            cv::Mat R;
            cv::Rodrigues(rvec, R);
            cv::Mat p = -R.t() * tvec;
            cv::String info = cv::format("XYZ: [%.3f, %.3f, %.3f]", cv::Point3d(p));
            cv::putText(image, info, cv::Point(5, 15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0));
        }

        // Show the image
        cv::imshow("3DV Tutorial: Pose Estimation (Chessboard)", image);
        int key = cv::waitKey(1);
        if (key == 27) break;                                   // 'ESC' key: Exit
        else if (key == 32)                                     // 'Space' key: Pause
        {
            key = cv::waitKey();
            if (key == 27) break;                               // 'ESC' key: Exit
        }
    }

    video.release();
    return 0;
}
