#include <iostream>
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;

std::vector<cv::Rect> generate(){

    std::vector<int> pnts = {0, 350, 700, 1024};
    std::vector<cv::Rect> rois;

    for (int i = 0; i < pnts.size()-1; ++i) {
        for (int j = 0; j < pnts.size()-1; ++j) {

            cv::Rect r(cv::Point(pnts.at(i), pnts.at(j)),
                       cv::Point(pnts.at(i+1), pnts.at(j+1)));
            rois.push_back(r);

        }
    }

    return rois;

}

int main()
{
    std::string path = "/home/mike/Pictures/1.png";
    cv::Mat img_pano = cv::imread(path);
    cv::resize(img_pano, img_pano,cv::Size(1024,1024));

    cv::Mat msk = cv::Mat::ones(img_pano.rows, img_pano.cols, CV_8UC1) * 255;
    std::string img1_path = "/home/mike/Pictures/3.png", img2_path = "/home/mike/Pictures/2.png";

    cv::detail::MultiBandBlender blender ;
    {
        blender = cv::detail::MultiBandBlender(false, 9);
        cv::Rect rr(0,0,700,700);
        blender.prepare(rr);

        std::vector<cv::Rect> rects = generate();


        for (const cv::Rect& r : rects)
        {
            if((r & rr).area() >= 1){
                cv::Rect read = r & rr;
                blender.feed(img_pano(read),msk(read), read.tl());
            }
        }

        cv::Mat im, ms;
        blender.blend(im,ms);
        cv::imwrite(img1_path, im(cv::Rect(0,0,512,512)));
    }

    {
        blender = cv::detail::MultiBandBlender(false, 9);
        cv::Rect rr(0,0,512,512);
        blender.prepare(rr);

        std::vector<cv::Rect> rects = generate();


        for (const cv::Rect& r : rects)
        {
            if((r & rr).area() >= 1){
                cv::Rect read = r & rr;
                blender.feed(img_pano(read),msk(read), read.tl());
            }
        }

        cv::Mat im, ms;
        blender.blend(im,ms);
        cv::imwrite(img2_path, im);
    }

    cv::Mat img1, img2;
    img1 = cv::imread(img1_path);
    img2 = cv::imread(img2_path);

    cv::imwrite("/home/mike/Pictures/diff.png", cv::abs(img1-img2) + 150);

    return 0;
}
