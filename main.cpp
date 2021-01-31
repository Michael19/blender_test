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

std::vector<int> generate_points(int size, int min_s){
    std::vector<int> out;

    for (int i = 0; i <= size; i+=min_s) {

        if(i == 0){
            out.push_back(i);
        }
        else{
            if(i >= size){
                i=size;
            }

            out.push_back(i);
        }
    }

    return out;
}

std::vector<cv::Rect> generate(int w, int h, int min_s){

    std::vector<int> pnts_w = generate_points(w, min_s);
    std::vector<int> pnts_h = generate_points(h, min_s);
    std::vector<cv::Rect> rois;

    for (int i = 0; i < pnts_w.size()-1; ++i) {
        for (int j = 0; j < pnts_h.size()-1; ++j) {

            cv::Rect r(cv::Point(pnts_w.at(i), pnts_h.at(j)),
                       cv::Point(pnts_w.at(i+1), pnts_h.at(j+1)));
            rois.push_back(r);

        }
    }

    return rois;

}

std::vector<cv::Rect> generate_rect_with_overlap(int w, int h, int min_s, int overlay, std::vector<cv::Rect>& to_write)
{

    std::vector<int> pnts_w = generate_points(w, min_s);
    std::vector<int> pnts_h = generate_points(h, min_s);

    std::vector<cv::Rect> rois;

    for (int i = 0; i < pnts_w.size()-1; ++i) {
        for (int j = 0; j < pnts_h.size()-1; ++j) {


            int left = pnts_w.at(i) - overlay < 0 ? 0 :  pnts_w.at(i) - overlay;
            int right = pnts_w.at(i+1) + overlay >= w ? w :  pnts_w.at(i+1) + overlay;

            int top = pnts_h.at(j) - overlay < 0 ? 0 :  pnts_h.at(j) - overlay;
            int bottom = pnts_h.at(j+1) + overlay >= h ? h :  pnts_h.at(j+1) + overlay;

            int leftw = pnts_w.at(i) - overlay < 0 ? 0 :  overlay;
            int rightw = leftw + min_s;// pnts_w.at(i+1) + overlay >= w ? w :  pnts_w.at(i+1) + overlay;

            int topw = pnts_h.at(j) - overlay < 0 ? 0 :  overlay;
            int bottomw = topw + min_s;// pnts_h.at(j+1) + overlay >= h ? h :  pnts_h.at(j+1) + overlay;

            cv::Rect r(cv::Point(left, top),cv::Point(right, bottom));
            cv::Rect to_w(cv::Point(leftw, topw),cv::Point(rightw, bottomw));

//            cv::Rect to_w (0,0,min_s,min_s);

//            if(r.br().x > w || r.br().y > h)
//            {
//                if(r.br().x > w)
//                {
//                    to_w.x += overlay;
//                }

//                if(r.br().y > h)
//                {
//                    to_w.y += overlay;
//                }

//            }
            rois.push_back(r);
            to_write.push_back(to_w);

        }
    }

    return rois;

}

int main()
{
    std::string directory_to_save = "/home/mike/Downloads/sosnovka/3/";
    std::string path = "/home/mike/Downloads/sosnovka/Panorama_park_1.jpg";
    cv::Mat img_pano = cv::imread(path);

    //    cv::resize(img_pano, img_pano,cv::Size(1024,1024));
//    img_pano(cv::Rect(2000,1500,1024,1024)).copyTo(img_pano);
    cv::imwrite(directory_to_save + "pano.png", img_pano);

    cv::Mat msk = cv::Mat::ones(img_pano.rows, img_pano.cols, CV_8UC1) * 255;
    std::string img1_path = directory_to_save + "3.png", img2_path = directory_to_save + "2.png";

    int size_tile = 1024;
    int overlaps = 256;
    int size_image = 400;



    // with overlap
    {
        cv::Mat out = cv::Mat::zeros(img_pano.rows, img_pano.cols, img_pano.type());
        std::vector<cv::Rect> rects_to_write ;
        std::vector<cv::Rect> rects_with = generate_rect_with_overlap(img_pano.cols, img_pano.rows,
                                                                      size_tile, overlaps,rects_to_write);
        std::vector<cv::Rect> rects_without = generate(img_pano.cols, img_pano.rows,
                                                       size_tile);

        std::vector<cv::Rect> rects = generate(img_pano.cols, img_pano.rows,
                                               size_image);

#pragma omp parallel for
        for (int i = 0; i < rects_with.size(); ++i) {

            cv::Rect r = rects_with.at(i);
            cv::Rect rr(0,0,r.width,r.height);
            cv::detail::MultiBandBlender blender  = cv::detail::MultiBandBlender(true,7);
            blender.prepare(rr);


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
