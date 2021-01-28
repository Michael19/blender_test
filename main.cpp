#include <iostream>
#include "opencv2/stitching/detail/blenders.hpp"


#if CV_MAJOR_VERSION >= 3
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#else
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

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

std::vector<cv::Rect> generate_rect_without_overlap(){

    std::vector<int> pnts = {0, 512, 1024};
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

std::vector<cv::Rect> generate_rect_with_overlap(std::vector<cv::Rect>& to_write)
{

    std::vector<int> pnts = {0, 512, 1024};
    std::vector<cv::Rect> rois;

    for (int i = 0; i < pnts.size()-1; ++i) {
        for (int j = 0; j < pnts.size()-1; ++j) {

            cv::Rect r(cv::Point(pnts.at(i), pnts.at(j)),
                       cv::Point(pnts.at(i+1) + 256, pnts.at(j+1) + 256));

            cv::Rect to_w (0,0,512,512);

            int x = 0, y = 0;

            if(r.br().x > 1024 || r.br().y > 1024)
            {
                if(r.br().x > 1024)
                {
                    x = r.tl().x - 256;
                    to_w.x += 256;
                }

                if(r.br().y > 1024)
                {
                    y = r.tl().y - 256;
                    to_w.y += 256;
                }

                r = cv::Rect(cv::Point(x,y), cv::Point(pnts.at(i+1), pnts.at(j+1)));

            }
            rois.push_back(r);
            to_write.push_back(to_w);

        }
    }

    return rois;

}

int main()
{
    std::string directory_to_save = "/home/lunto/Documents/1/";
    std::string path = "/home/lunto/00.69879.jpg";
    cv::Mat img_pano = cv::imread(path);

//    cv::resize(img_pano, img_pano,cv::Size(1024,1024));
    img_pano(cv::Rect(2000,1500,1024,1024)).copyTo(img_pano);
    cv::imwrite(directory_to_save + "pano.png", img_pano);

    cv::Mat msk = cv::Mat::ones(img_pano.rows, img_pano.cols, CV_8UC1) * 255;
    std::string img1_path = directory_to_save + "3.png", img2_path = directory_to_save + "2.png";

    cv::detail::MultiBandBlender blender ;

    // with overlap
    {
        cv::Mat out = cv::Mat::zeros(img_pano.rows, img_pano.cols, img_pano.type());
         std::vector<cv::Rect> rects_to_write ;
        std::vector<cv::Rect> rects_with = generate_rect_with_overlap(rects_to_write);
        std::vector<cv::Rect> rects_without = generate_rect_without_overlap();


        for (int i = 0; i < rects_with.size(); ++i) {

            cv::Rect r = rects_with.at(i);
            cv::Rect rr(0,0,r.width,r.height);
            blender = cv::detail::MultiBandBlender(false,6);
            blender.prepare(rr);

            std::vector<cv::Rect> rects = generate();
            for (const cv::Rect& rt : rects)
            {
                if((rects_without.at(i) & rt).area() >= 1)
                {
                    cv::Rect read = r & rt;
                    cv::Point tl(read.x - r.x, read.y - r.y);

//                    float r = 1. / float(std::rand() % 10);

                    blender.feed(img_pano(read)  ,msk(read), tl);
                }
            }

            cv::Mat im, ms;
            blender.blend(im,ms);
            im(rects_to_write.at(i)).copyTo(out(rects_without.at(i)));

        }
        cv::imwrite(img1_path, out);



    }

//    {
//        blender = cv::detail::MultiBandBlender(false, 9);
//        cv::Rect rr(0,0,512,512);
//        blender.prepare(rr);

//        std::vector<cv::Rect> rects = generate();


//        for (const cv::Rect& r : rects)
//        {
//            if((r & rr).area() >= 1){
//                cv::Rect read = r & rr;
//                blender.feed(img_pano(read),msk(read), read.tl());
//            }
//        }

//        cv::Mat im, ms;
//        blender.blend(im,ms);
//        cv::imwrite(img2_path, im);
//    }

    cv::Mat img1, img2;
    img1 = cv::imread(img1_path);
    img2 = cv::imread(directory_to_save + "pano.png");

    cv::imwrite(directory_to_save + "diff.png", cv::abs(img1-img2) + 100);

    return 0;
}
