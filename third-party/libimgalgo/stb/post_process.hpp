

#ifndef __POST_PROCESS_H__
#define __POST_PROCESS_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

#define  MI_ALG_DETECT      //小米算法


typedef struct ObjectBox {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int classid;
    ObjectBox(){
        x1 = 0.0f;
        y1 = 0.0f;
        x2 = 0.0f;
        y2 = 0.0f;
        score = 0.0f;
        classid = -1;
    }
} ObjectBox;

static void generateBBox_YOLOV5(float *p, vector<ObjectBox> &candidate_boxes, int img_w, int img_h, vector<float> &anchor,
			 vector<float> &strides, float person_threshold, int onechannel, int box_num, int classes);
static void nms(std::vector<ObjectBox> &input, std::vector<ObjectBox> &output, int type, float nms_threshold);    
static void manyclass_nms(std::vector<ObjectBox> &input, std::vector<ObjectBox> &output,
		   int classnums, float nms_threshold);
static void postprocess(float *pv, std::vector<ObjectBox>& face_list, int img_w, int img_h, int iType, std::vector<float> &threshold);

static void generateBBox_YOLOV5(float *p, vector<ObjectBox> &candidate_boxes, int img_w, int img_h, vector<float> &anchor,
			 vector<float> &strides, float person_threshold, int onechannel, int box_num, int classes){
    for (int s = 0; s < 3; s++){
	int height_ = img_h/strides[s];
	int width_ = img_w/strides[s];
	for(int h = 0 ; h < height_; h++) {
	    for(int w = 0; w < width_; w++){
		for(int n = 0; n < box_num; n++){
		    float xptr = p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel];
		    float yptr = p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel+1];
		    xptr = 1.f / (1.f + exp(-xptr));
		    yptr = 1.f / (1.f + exp(-yptr));
		    float wptr = p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel+2];
		    float hptr = p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel+3];
		    wptr = 1.f / (1.f + exp(-wptr));
		    hptr = 1.f / (1.f + exp(-hptr));
		    float box_score_ptr = p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel+4];
		    box_score_ptr = 1.f / (1.f + exp(-box_score_ptr));

		    float anchor_w = anchor[s*box_num*2+n * 2] / img_w;
		    float anchor_h = anchor[s*box_num*2+n * 2 + 1] / img_h;

		    float box_w = pow(wptr*2.f, 2) * anchor_w;
		    float box_h = pow(hptr*2.f, 2) * anchor_h;

		    float xmin = (xptr*2.f - 0.5f + w) / width_ - box_w * 0.5f;
		    float ymin = (yptr*2.f - 0.5f + h) / height_ - box_h * 0.5f;
		    float xmax = xmin + box_w;
		    float ymax = ymin + box_h;

		    int class_index = 0;
		    float class_score = 0.f;
		    for (int c =0;c <classes; c++){
                float score =  p[h*(width_*box_num*onechannel)+w*(box_num*onechannel)+n*onechannel+5+c];
                //softmax
                score = 1.f / (1.f + exp(-score));
                if (score>class_score){
                    class_index = c;
                    class_score = score;
                }
		    }

		    float prob = box_score_ptr * class_score;
		    if (prob >= person_threshold){
		        ObjectBox object;
			object.x1 = xmin*img_w;
			object.y1 = ymin*img_h;
			object.x2 = xmax*img_w;
			object.y2 = ymax*img_h;
			object.score = prob;
			object.classid = class_index;
			candidate_boxes.push_back(object);
		    }
	        }
	    }
    }
	p += height_*width_*box_num*onechannel;
    }
}

static void nms(std::vector<ObjectBox> &input, std::vector<ObjectBox> &output, int type, float nms_threshold) {
    std::sort(input.begin(), input.end(), [](const ObjectBox &a, const ObjectBox &b) {return a.score > b.score; });
    int box_num = input.size();
    // std::cout << "box_num: " << box_num << std::endl;
    std::vector<int> merged(box_num, 0);

    for (int i = 0; i < box_num; i++) {
        if (merged[i]){
            continue;
        }
        std::vector<ObjectBox> buf;
     
        buf.push_back(input[i]);
        merged[i] = 1;

        float h0 = input[i].y2 - input[i].y1 + 1;
        float w0 = input[i].x2 - input[i].x1 + 1;

        float area0 = h0 * w0;
        
        for (int j = i + 1; j < box_num; j++) {
            if (merged[j]) {
                continue;
            }
            float inner_x0 = input[i].x1 > input[j].x1 ? input[i].x1 : input[j].x1;
            float inner_y0 = input[i].y1 > input[j].y1 ? input[i].y1 : input[j].y1;

            float inner_x1 = input[i].x2 < input[j].x2 ? input[i].x2 : input[j].x2;
            float inner_y1 = input[i].y2 < input[j].y2 ? input[i].y2 : input[j].y2;

            float inner_h = inner_y1 - inner_y0 + 1;
            float inner_w = inner_x1 - inner_x0 + 1;

            // std::cout << "----------------------------------" << i << "  "<< j <<std::endl; 
            // std::cout << "input[i]: " << input[i].x1 << " " << input[i].y1 << " " << input[i].x2 << " " << input[i].y2 << std::endl;
            // std::cout << "input[j]: " << input[j].x1 << " " << input[j].y1 << " " << input[j].x2 << " " << input[j].y2 << std::endl;
            // std::cout << "inner_x0: " << inner_x0 << " inner_y0: " << inner_y0 << " inner_x1: " << inner_x1 << " inner_y1: " << inner_y1 << std::endl;
            // std::cout << "inner_h: " << inner_h << " inner_w: " << inner_w << std::endl; 

            if (inner_h <= 0 || inner_w <= 0)
                continue;

            float inner_area = inner_h * inner_w;
            float h1 = input[j].y2 - input[j].y1 + 1;
            float w1 = input[j].x2 - input[j].x1 + 1;
            float area1 = h1 * w1;
            float score;

            score = inner_area / (area0 + area1 - inner_area);
            // std::cout << "score: " << score << " nms_threshold: " << nms_threshold << std::endl;
            if (score > nms_threshold) {
                merged[j] = 1;
                buf.push_back(input[j]);
            }
        }

        switch (type) {
        case 0: {
            output.push_back(buf[0]);
            break;
        }
        case 1: {
            float total = 0;
            for (int i = 0; i < (int)buf.size(); i++) {
                total += exp(buf[i].score);
            }
            ObjectBox rects;
            memset(&rects, 0, sizeof(rects));
            float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
            for (int i = 0; i < (int)buf.size(); i++) {
                float rate = exp(buf[i].score) / total;
                x0 += buf[i].x1 * rate;
                y0 += buf[i].y1 * rate;
                x1 += buf[i].x2 * rate;
                y1 += buf[i].y2 * rate;
                rects.score += buf[i].score * rate;
            }
            rects.x1 = x0;
            rects.y1 = y0;
            rects.x2 = x1;
            rects.y2 = y1;
            rects.classid =buf[0].classid;
            output.push_back(rects);
            break;
        }
        default: {
            printf("wrong type of nms.");
            exit(-1);
        }
        }
    }
}

static void manyclass_nms(std::vector<ObjectBox> &input, std::vector<ObjectBox> &output, int classnums, float nms_threshold) {
    int box_num = input.size();
    std::vector<int> merged(box_num, 0);
    std::vector<ObjectBox> classbuf;
  
    for (int clsid = 0; clsid < classnums; clsid++) {
        classbuf.clear();
        for (int i = 0; i < box_num; i++) {
            if (merged[i]){
	        continue;
            }         
            if(clsid!=input[i].classid){
	        continue;
            }
            classbuf.push_back(input[i]);
            merged[i] = 1;
        }
        nms(classbuf, output, 0, nms_threshold);
    }
}

static void postprocess(float *pv, std::vector<ObjectBox>& face_list, int img_w, int img_h, int iType, std::vector<float> &threshold)
{
    int classes = 6;
    int box_num = 3;
    float nms_threshold = 0.4;

    vector<float> anchor = {4,5,8,10,13,16,23,29,43,55,73,105,146,217,231,300,335,433};
    vector<float> strides = {8.0, 16.0, 32.0};
    float person_threshold = 0.1;

    if(iType == 0)
        classes = 6;//OD
    else if(iType == 1)
        classes = 1;//PD

    threshold = {0.1,0.1,0.1,0.1,0.1,0.1};

    int onechannel = 5 + classes;
    std::vector<ObjectBox> candidate_boxes;
    generateBBox_YOLOV5(pv, candidate_boxes, img_w, img_h, anchor, strides, person_threshold, onechannel, box_num, classes);
    // manyclass_nms(candidate_boxes, face_list, classes, nms_threshold);
    nms(candidate_boxes, face_list, 0, nms_threshold);
}
#endif /* __POST_PROCESS_H__ */