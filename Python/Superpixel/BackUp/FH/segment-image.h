

#ifndef SEGMENT_IMAGE
#define SEGMENT_IMAGE

/*
 Copyright (C) 2006 Pedro Felzenszwalb
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */



#include <cstdlib>
#include "image.h"
#include "misc.h"
#include "filter.h"
#include "segment-graph.h"
#include <iostream>



// random color
rgb random_rgb(){
    rgb c;
    double r;
    
    c.r = (uchar)random();
    c.g = (uchar)random();
    c.b = (uchar)random();
    
    return c;
}

// threshold function
#define THRESHOLD(size, c) (c/size)

// dissimilarity measure between pixels
static inline float diff(image<float> *r, image<float> *g, image<float> *b,
                         int x1, int y1, int x2, int y2) {
    return sqrt(square(imRef(r, x1, y1)-imRef(r, x2, y2)) +
                square(imRef(g, x1, y1)-imRef(g, x2, y2)) +
                square(imRef(b, x1, y1)-imRef(b, x2, y2)));
}



/*
 * Segment an image
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation. this variable is initially set by zero and will return the result value to the main program
 
 输入
 SLIC那样的一个unit的image array ,通过过内部的bit shift来获得r，b，g 的值
 
 slicLabelMask,是slic输出的一个1-labelmask
 
 
 输出
 输入参数穿进去的时一个1-d labels[]的array，用来保存输出的图片上面的labelMask
 所以这里声明的时候，一定要int *&,指针的引用，这样才能保证内部修改array可以影响到外面
 这个function是void类型，其实就是对label这个数组进行处理，这个labels[]的定义在调用segment_image()
 函数外面就声明了
 
 
 */
void segment_image(const unsigned int*im, const int width, const int height, int*& outLabels,int *& slicLabelMask,int slicSegNum,float sigma, float c, int min_size,
                            int *num_ccs) {
    
    
    
    image<float> *r = new image<float>(width, height);
    image<float> *g = new image<float>(width, height);
    image<float> *b = new image<float>(width, height);
    
    
    int sz=width*height;
    
    //下面是对cython 输入的处理
    
    
    // smooth each color channel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            
            int index=y*width+x;
            float rValue=(im[index] >> 16) & 0xFF;
            float gValue=(im[index] >> 8 ) & 0xFF;
            float bValue=(im[index]      ) & 0xFF;
            
            imRef(r, x, y) = rValue;
            imRef(g, x, y) = gValue;
            imRef(b, x, y) = bValue;
        }
    }
    
    
    
    image<float> *smooth_r = smooth(r, sigma);
    image<float> *smooth_g = smooth(g, sigma);
    image<float> *smooth_b = smooth(b, sigma);
    delete r;
    delete g;
    delete b;
    
    /*
     初始化set和threshold[]
     */
    
    outLabels=new int[width*height];
    universe * u=new universe(sz);
    float * graph_threshold=new float[sz];
    for (int i=0; i<sz; i++) {
        graph_threshold[i]=THRESHOLD(1, c);
    }
    
    
    /*
     我的edgeList建立的函数，之后还要把后面的seg_graph加进来
     
     */
    
    for (int currLabel=0; currLabel<slicSegNum;currLabel++) {
        
        std::vector<edge> edgeList;
        
        for (int y=0; y<height; y++) {
            for(int x=0;x<width;x++){
                int index=y*width+x;
                //如果不是当前label，就直接跳出来
                if (slicLabelMask[index]==currLabel) {


                //右边edge
                if (x<width-1) {
                    int rightPixIndex=y*width+(x+1);
                    if (slicLabelMask[rightPixIndex]==currLabel) {
                        edge newEdge;
                        newEdge.a=index;
                        newEdge.b=rightPixIndex;
                        newEdge.w=diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
                        edgeList.push_back(newEdge);
                    }
                }
                
                //下面edge
                if (y<height-1) {
                    int dowEdgeIndex=(y+1)*width+x;
                    if (slicLabelMask[dowEdgeIndex]==currLabel) {
                        edge addEdge;
                        addEdge.a=index;
                        addEdge.b=dowEdgeIndex;
                        addEdge.w=diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
                        edgeList.push_back(addEdge);
                    }
                }
                
                //右下edge
                if ((x<width-1)&&(y<height-1)) {
                    int desEdgeIndex=(y+1)*width+(x+1);
                    if (slicLabelMask[desEdgeIndex]==currLabel) {
                        edge addEdge;
                        addEdge.a=index;
                        addEdge.b=desEdgeIndex;
                        addEdge.w=diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
                        edgeList.push_back(addEdge);
                        
                    }
                }
                
                //右上
                if ((x<width-1)&&(y>0)) {
                    int desEdgeIndex=(y-1)*width+(x+1);
                    if (slicLabelMask[desEdgeIndex]==currLabel) {
                        edge addEdge;
                        addEdge.a=index;
                        addEdge.b=desEdgeIndex;
                        addEdge.w=diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
                        edgeList.push_back(addEdge);
                    }
                }
            }
        }
        }
        
        region_segment_graph(edgeList, u,graph_threshold,c);
        
        //处理所有小于min_size的部分
        
        for (int i = 0; i < edgeList.size(); i++) {
            int a = u->find(edgeList[i].a);
            int b = u->find(edgeList[i].b);
            if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
                u->join(a, b);
        }
        
    }
    
    
    
    
    
    
    
    *num_ccs = u->num_sets();
    
    int segNum=u->num_sets();
    
    /*
     输出outLabels[]是一个1-d vector，从0-segNum来编号，但是这里segRegion的编号不是从
     pixel[0][0]开始就为0的，而是第一个找到的rootNode对应的那个region label为0
     */
    
    int label=0;
    for (int i=0; i<width*height; i++) {
        int rootNode=u->find(i);
        if (rootNode==i) {
            outLabels[i]=label;
            label++;
        }
        else{
            outLabels[i]=-1;
        }
    }
    
    for (int i=0; i<width*height; i++) {
        int rootNode=u->find(i);
        outLabels[i]=outLabels[rootNode];
    }
    
    
    image<rgb> *output = new image<rgb>(width, height);
    
    
    // pick random colors for each component
    rgb *colors = new rgb[width*height];
    for (int i = 0; i < width*height; i++)
        colors[i] = random_rgb();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int comp = u->find(y * width + x);
            //set the color to be the same as the root node
            imRef(output, x, y) = colors[comp];
        }
    }
    
    delete [] colors;
    delete u;
    
    
    //return output;
    
    
}




#endif
