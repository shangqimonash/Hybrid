// ******************************************************************************
// SEEDS Superpixels
// ******************************************************************************
// Author: Beat Kueng based on Michael Van den Bergh's code
// Contact: vamichae@vision.ee.ethz.ch
//
// This code implements the superpixel method described in:
// M. Van den Bergh, X. Boix, G. Roig, B. de Capitani and L. Van Gool,
// "SEEDS: Superpixels Extracted via Energy-Driven Sampling",
// ECCV 2012
//
// Copyright (c) 2012 Michael Van den Bergh (ETH Zurich). All rights reserved.
// ******************************************************************************

#include "seeds2.h"
#include "math.h"
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>

#include <cstring>

// Choose color space, LAB is better, HSV is faster (initialization)
//#define LAB_COLORSPACE
#define HSV_COLORSPACE

// Enables means post-processing
#define MEANS

// Enables 3x3 smoothing prior
#define PRIOR

// If enabled, each block-updating level will be iterated twice.
// Disable this to speed up the algorithm.
//#define DOUBLE_STEPS
#define REQ_CONF 0.1

#define MINIMUM_NR_SUBLABELS 1



/*
 这个函数直接在superpixel_test里面调用,调用iterate()之后，就开始drawContour了，所以这个是主要的函数核心
 */
//正式的superpixel生成函数，上面其他函数只是把那些变量给初始化和得到对应的histogram
void SEEDS::iterate()
{
    // block updates
    //seeds_current_level现在是top level下面的一层，start with level seeds_top_level-1, then go down
    while (seeds_current_level >= 0)
    {
#ifdef DOUBLE_STEPS
        update_blocks(seeds_current_level, REQ_CONF);
#endif
        //这一个就是主要算法，用遍历的方式，看看某个grid是分到左边的label好呢，还是右边好，是上面好呢，还是下面好，然后更新label,显示horizontal，看某个grid是分到左边的superpixel region好还是右边，接着检测vertical，看看是分到上面还是下面
        update_blocks(seeds_current_level);
        
        //更新nr_partition和current level grid的parent，这个parent指向top level对应的superpixel
        seeds_current_level = go_down_one_level();
    }
    
#ifdef MEANS
    /*
     求出每个label的lab的intensity value，也就是给每个label上什么颜色
     */
    compute_means();
    
    update_pixels_means();
    update_pixels_means();
    update_pixels_means();
    update_pixels_means();
#else
    /*
     如果没有开启MEANS的处理，那么就跟根据，update pixel其实操作和update block差不多，但是这里就是对pixel level进行处理的。因为之前block 更新到current level=0那层之后，就不会再往下走了
     
     */
    update_pixels();
    update_pixels();
    update_pixels();
    update_pixels();
#endif
}





SEEDS::SEEDS(int width, int height, int nr_channels, int nr_bins)
{
    this->width = width;
    this->height = height;
    this->nr_channels = nr_channels;
    this->nr_bins = nr_bins;
    
    //bin index (histogram) of each image pixel [y*width + x],也就是对于每个pixel，我都需要求出他的intensity在histogram里面属于哪个bin。image_bins的长度是图片的长度，内部存数的是histogram bin index
    image_bins = new UINT[width*height];
    
    /*
     image的l chanel的值
     
     */
    image_l = new float[width*height];
    image_a = new float[width*height];
    image_b = new float[width*height];
    
    /*
     这个是在找neighbor适合，是往前找还是往后找
     */
    forwardbackward = true;
    /*
     是histogram的bin的总数量一共三个颜色channel，每个chanel有5个bin所以（2，4，1）就代表
     在l中是bin 2，a中为bin 4，b中为bin1，241就是这个bin的5进制表达方式，如果转换为10进制
     就是2+4*nr_bin+1*nr_bin*nr_bin，就把一个3-D vector表示的histogram，转换到一个
     1-d的histogram
     */
    histogram_size = nr_bins*nr_bins*nr_bins;
    
    /*
     用来表示初始化是否完成
     */
    initialized = false;
}

SEEDS::~SEEDS()
{
    deinitialize();
    
    // from constructor
    delete[] image_bins;
    delete[] image_l;
    delete[] image_a;
    delete[] image_b;
}
void SEEDS::deinitialize()
{
    if(initialized)
    {
        initialized = false;
        for (int level=0; level<seeds_nr_levels; level++)
        {
            for (UINT label=0; label<nr_labels[level]; label++)
            {
                delete[] histogram[level][label];
            }
            delete[]  histogram[level];
            delete[] T[level];
            delete[] labels[level];
            delete[] parent[level];
            delete[] nr_partitions[level];
            
        }
        delete[] histogram;
        delete[] T;
        delete[] labels;
        delete[] parent;
        delete[] nr_partitions;
        delete[] nr_labels;
        delete[] nr_w;
        delete[] nr_h;
        
#ifdef LAB_COLORSPACE
        delete[] bin_cutoff1;
        delete[] bin_cutoff2;
        delete[] bin_cutoff3;
#endif
        
#ifdef MEANS
        delete[] L_channel;
        delete[] A_channel;
        delete[] B_channel;
#endif
    }
}


/*
 最开始传入的seeds_w，seeds_h是最小的block的width和height
 
 */
void SEEDS::initialize(int seeds_w, int seeds_h, int nr_levels)
{
    deinitialize();
    
    this->seeds_w = seeds_w;
    this->seeds_h = seeds_h;
    this->seeds_nr_levels = nr_levels;
    this->seeds_top_level = nr_levels - 1;
    
    // init labels
    //labels是一个2-d数组，他保存每个pixel在每个level时候的label的值,这里首先初始化了labels的第一维度，也就是有seeds_nr_levels个level分层，在这里是4层
    labels = new UINT*[seeds_nr_levels];
    
    //[level][label] = corresponding label of block with level+1，当前level的label对应上一层较大block的label
    //parent在level 0的长度，正好是这个level grid的总数量，里面存储的值则是level 0的某个grid对应上一层level 1中的哪个grid
    parent = new UINT*[seeds_nr_levels];
    
    //[level][label] how many partitions label has on level-1
    //这个还暂时不知道是什么,是一个2-d数组
    nr_partitions = new UINT*[seeds_nr_levels];
    
    //[level],是一个1d数组，保存某曾level中labels的数量，也就是某个level中grid region的数量
    nr_labels = new UINT[seeds_nr_levels];
    
    //1d数组，[level] number of seeds in x-direction
    nr_w = new int[seeds_nr_levels];
    nr_h = new int[seeds_nr_levels];
    
    int level = 0;
    int nr_seeds_w = floor(width/seeds_w);
    int nr_seeds_h = floor(height/seeds_h);
    int nr_seeds = nr_seeds_w*nr_seeds_h;
    
    //nr_labels就是这个level中，grid region的总数量
    nr_labels[level] = nr_seeds;
    //level 0 时候，每一行有多少个grid region
    nr_w[level] = nr_seeds_w;
    nr_h[level] = nr_seeds_h;
    
    //把label在level 0的时候初始化，他的长度正好是img 所有pixel一起的长度，
    labels[level] = new UINT[width*height];
    //parent在level 0的长度，正好是这个level grid的总数量，里面存储的值则是level 0的某个grid对应上一层level 1中的哪个grid
    parent[level] = new UINT[nr_seeds];
    
    //nr_partition[level0][]的长度也是这个level grid的总数量，里面存到东西还不知道
    nr_partitions[level] = new UINT[nr_seeds];
    
    for (level = 1; level < seeds_nr_levels; level++)
    {
        //nr_seeds_w是width上面的grid的数量，如果高了一个level，这个grid的数量就除以2
        nr_seeds_w /= 2; // always partitioned in 2x2 sub-blocks
        nr_seeds_h /= 2; // always partitioned in 2x2 sub-blocks
        
        nr_seeds = nr_seeds_w*nr_seeds_h;
        labels[level] = new UINT[width*height];
        parent[level] = new UINT[nr_seeds];
        nr_partitions[level] = new UINT[nr_seeds];
        nr_labels[level] = nr_seeds;
        nr_w[level] = nr_seeds_w;
        nr_h[level] = nr_seeds_h;
    }
    
    
#ifdef MEANS
    L_channel = new float[nr_labels[seeds_top_level]];
    A_channel = new float[nr_labels[seeds_top_level]];
    B_channel = new float[nr_labels[seeds_top_level]];
#endif
    
    
    // create histogram buffers
    
    
    //histogram[level][label][j < histogram_size],histogram是一个3d的数组，表示某个level上面某个grid(也就是label） region对应的某个bin的数量
    histogram = new int**[seeds_nr_levels]; // histograms are kept for each level and each label [level][label][bin]
    
    //[level][label] how many pixels with this label, 表示在某个level的某个grid上面，有多少个pixel
    T = new int*[seeds_nr_levels]; // block sizes are kept at each level [level][label]
    for (int level=0; level<seeds_nr_levels; level++)
    {
        //histogram的第2个维度，就是这个level上面有多少个grid，而这个有多少grid就存在nr_labels[level]这个变量上面
        histogram[level] = new int*[nr_labels[level]]; // histograms are kept for each level and each label [level][label][bin]
        //T保存的是某个level下面，某个grid的pixel的总数量
        T[level] = new int[nr_labels[level]]; // block sizes are kept at each level [level][label]
        for (UINT label=0; label<nr_labels[level]; label++)
        {
            //对histogram的第三个维度初始化，每个grid region有多少个bin
            histogram[level][label] = new int[histogram_size]; // histogram bins
        }
    }
    
#ifdef LAB_COLORSPACE
    /*
     histogram有nr_bins个bin，每个bin的cutoff value是多少保存在bin_cutoff1[]这个数组里
     
     */
    bin_cutoff1 = new float[nr_bins];
    bin_cutoff2 = new float[nr_bins];
    bin_cutoff3 = new float[nr_bins];
#endif
    
    initialized = true;
}

/*
 到目前update_image_ycbcr()为止，所有的histogram，T还有nr_partition才全部初始化完毕
 */
void SEEDS::update_image_ycbcr(UINT* image) {
    
    
    seeds_current_level = seeds_nr_levels - 2;
    
    //把label和parent跟更新好
    assign_labels();
    
    // adaptive histograms
    //得到LAB三个chanel的cutoff value的值，
#ifdef LAB_COLORSPACE
    lab_get_histogram_cutoff_values(img);
#endif
    
    // convert input image from YCbCr to LAB or HSV
    
    unsigned char r, g, b;
    float L, A, B;
    
    for(int x = 0; x < width; x++)
        for(int y = 0; y < height; y++)
        {
            int i = y * width + x;
            r = image[i] >> 16;
            g = (image[i] >> 8) & 0xff;
            b = (image[i]) & 0xff;
            
#ifdef LAB_COLORSPACE
            //RBG2LAB_special()返回的时0-124的值，也就是把(2,3,4)这样一个3D的bin，作为一个5进制的数字，转换为1D之后的效果。最后把这个值保存在image_bins[]这个数组中。
            //最大的5进制值为(4,4,4)=124
            image_bins[i] = RGB2LAB_special((int)r, (int)g, (int)b, &L, &A, &B);
            image_l[i] = L/100.0;
            image_a[i] = (A+128.0)/255.0;
            image_b[i] = (B+128.0)/255.0;
#endif
#ifdef HSV_COLORSPACE
            image_bins[i] = RGB2HSV((int)r, (int)g, (int)b, &L, &A, &B);
            image_l[i] = L;
            image_a[i] = A;
            image_b[i] = B;
#endif
        }
    
    //这个函数的作用就是把计算的到各个level的histogram，T，还有nr_partition【】
    compute_histograms();
}




// Assign Labels
// adds labeling to all the blocks at all levels and sets the correct parents
void SEEDS::assign_labels()
{
    /*
     level 0是保存的最低层，最小的哪个grid，最顶层的level则是最后输出的哪个superpixel
     
     */
    // level 0 (base level): seeds_w x seeds_h
    int level = 0;
    int nr_seeds_w = floor(width/seeds_w);
    int nr_seeds_h = floor(height/seeds_h);
    int step_w = seeds_w;
    int step_h = seeds_h;
    int nr_seeds = nr_seeds_w*nr_seeds_h;
    
    for (int i=0; i<nr_seeds; i++) nr_partitions[level][i] = 1; // base level: no further partitioning
    for (int y=0; y<height; y++) {
        
        //这一句的意思就是，如果当前y这个坐标，如果分到grid里面是多少
        int label_y = y/step_h;
        //如果这个label_y太大，就设置成h方向上seed的总数减1
        if (label_y >= nr_seeds_h) label_y = nr_seeds_h-1;
        const int y_width = y*width;
        for (int x=0; x<width; x++)
        {
            int label_x = x/step_w;
            if (label_x >= nr_seeds_w) label_x = nr_seeds_w-1;
            
            //这里就是把level=0时候，每个grid给标记上label
            labels[level][y_width+x] = label_y*nr_seeds_w + label_x;
            // parents will be set in the next loop
        }
    }
    
    // level 1, 2, 3 ... until top level
    for (int level = 1; level < seeds_nr_levels; level++)
    {
        //width宽度上的seed的数量，因为上了一层，所以要减半
        nr_seeds_w /= 2; // always partitioned in 2x2 sub-blocks
        nr_seeds_h /= 2; // always partitioned in 2x2 sub-blocks
        step_w *= 2;
        step_h *= 2;
        nr_seeds = nr_seeds_w*nr_seeds_h;
        //对于nr_partitions[level][i]的值，这里先设成0，再之后再更新
        for (int i=0; i<nr_seeds; i++) nr_partitions[level][i] = 0; // will be added in add_blocks
        for (int y=0; y<height; y++) {
            int label_y = y/step_h;
            if (label_y >= nr_seeds_h) label_y = nr_seeds_h-1;
            const int y_width = y*width;
            for (int x=0; x<width; x++)
            {
                int label_x = x/step_w;
                if (label_x >= nr_seeds_w) label_x = nr_seeds_w-1;
                labels[level][y_width+x] = label_y*nr_seeds_w + label_x;
                
                //这个pixel在level-1层对应的label的parent，就是这一层level中，label的值
                parent[level-1][labels[level-1][y_width+x]] = labels[level][y_width+x]; // set parent
            }
        }
    }
    
}

//得到LAB三个chanel的cutoff value的值，
#ifdef LAB_COLORSPACE
void SEEDS::lab_get_histogram_cutoff_values(const Image& image)
{
    // get image lists and histogram cutoff values
    vector<float> list_channel1;
    vector<float> list_channel2;
    vector<float> list_channel3;
    vector<float>::iterator it;
    int samp = 5;
    int ctr = 0;
    for (int x=0; x<width; x+=samp)
        for (int y=0; y<height; y+=samp)
        {
            int i = y*width +x;
            const Image::Pixel& pix = image.image[y][x];
            unsigned char r, g, b;
            ColorModelConversions::fromYCbCrToRGB(pix.y, pix.cb, pix.cr, r, g, b);
            float L = 0;
            float A = 0;
            float B = 0;
            //吧得到的r，g，b的值转换为LAB的值，并且保存在LAB这三个local variable中
            RGB2LAB((int)r, (int)g, (int)b, &L, &A, &B);
            list_channel1.push_back(L);
            list_channel2.push_back(A);
            list_channel3.push_back(B);
            ctr++;
        }
    
    /*
     对于list_chanel1[]里面存的都是L的值，但是这些intensity不一定都是从0开始，到intensity的max
     我希望我的histogram正好充满这写L intensity，所以如果我想要形成3个bins，那么第一个bin应该就
     包括n=list_chanel1.size()/3那么多元素，接下来我就把list_chanel1[]进行n_element sort，这样
     sort之后，第n个对应的list_chanel1[n]就是我得到的那个cutoff value，这样构建histogram会比较
     快
     */
    
    for (int i=1; i<nr_bins; i++)
    {
        //N，小于我第i个bin，应该包含多少个pixel，ctr就是总的pixel数量
        int N = (int) floor((float) (i*ctr)/ (float)nr_bins);
        nth_element(list_channel1.begin(), list_channel1.begin()+N, list_channel1.end());
        it = list_channel1.begin()+N;
        bin_cutoff1[i-1] = *it;
        nth_element(list_channel2.begin(), list_channel2.begin()+N, list_channel2.end());
        it = list_channel2.begin()+N;
        bin_cutoff2[i-1] = *it;
        nth_element(list_channel3.begin(), list_channel3.begin()+N, list_channel3.end());
        it = list_channel3.begin()+N;
        bin_cutoff3[i-1] = *it;
    }
    bin_cutoff1[nr_bins-1] = 300;
    bin_cutoff2[nr_bins-1] = 300;
    bin_cutoff3[nr_bins-1] = 300;
}

#endif

void SEEDS::compute_means()
{
    // clear counted LAB values
    for (UINT label=0; label<nr_labels[seeds_top_level]; label++)
    {
        L_channel[label] = 0;
        A_channel[label] = 0;
        B_channel[label] = 0;
    }
    
    // sweep image
    for (int i=0; i<width*height; i++)
    {
        int label = labels[seeds_top_level][i];
        // add LAB values to total
        L_channel[label] += image_l[i];
        A_channel[label] += image_a[i];
        B_channel[label] += image_b[i];
    }
}

/*
 每个pixel应该被分到哪个histogram的index已经保存在image_bins[]这个数组里了，接下来就是根据这个数组
 把每个grid对应的histogram来进行更新
 */
//until_level的值本来就是-1

//这个函数的作用就是把计算的到各个level的histogram，T，还有nr_partition【】
void SEEDS::compute_histograms(int until_level)
{
    
    //这里初始化就是until_level=-1,然后经过这一句最后等于seed_nr_levels,也就是level的层数。
    if (until_level == -1) until_level = seeds_nr_levels - 1;
    
    //until_level根据英文意思，正好就是到达某一层，这里是到达top那层，until_level最后还是等于seeds_nr_levels
    until_level++;
    
    /*
     似乎nr_labels[level]就是当前level下面，grid的数量，因为我要统计每个grid的histogram，
     而且每个level上面又都不一样，所以就存到nr_labels[]里面,用来index这个level下面这个grid
     
     histogram[level][label][bin]用来统计，在level下面，某个label的grid的histogram的
     某一bin的值是多少
     
     T[level][label]用来统计有多少个pixel在这个label的grid下面
     */
    
    // clear histograms，出事后histogram，分配内存
    for (int level=0; level<seeds_nr_levels; level++)
    {
        for (UINT label=0; label<nr_labels[level]; label++)
        {
            memset(histogram[level][label], 0, sizeof(int)*histogram_size);
        }
        memset(T[level], 0, sizeof(int)*nr_labels[level]);
    }
    
    // build histograms on the first level by adding the pixels to the blocks
    
    /*
     之前的nr_label[level]只是某个level一共有多少个label，而这里的label[level][pixelIndex]
     指的是当前pixelIndex下的pixel在某个level下面的label具体是多少
     
     */
    for (int x=0; x<width; x++)
        for (int y=0; y<height; y++)
        {
            int i = y*width +x;
            //labels[0][i]保存的是level为0的时候，某个pixel i属于哪个grid，
            
            //吧某个pixel添加到对应grid region的histogram的对应bin中，记录对应label的grid region的pixel数量T,更新level 0的histogram和T这俩个变量
            add_pixel(0, labels[0][i], x, y);
        }
    
    // build histograms on the upper levels by adding the histogram from the level below
    for (int level=1; level<until_level; level++)
    {
        for (UINT label=0; label<nr_labels[level-1]; label++)
            
        {
            //parent[level-1][label]对应的时，对于level-1,grid等于label这个grid region，它的parent的grid 的label是多少，所以这里的第二个参数，是在parent这一个level上grid得label数，
            //这里的label则是son这一个level上面grid的label的数
            
            ///这个函数的作用还是把histogram level=1开始的histogram的各个bin的值，由它的sublevel的那些grid region的histogram融合起来，出事后这些level>=1的histogram和T一级nr_partition[]这些变量
            add_block(level, parent[level-1][label], level-1, label);
        }
    }
}

// Merging Algorithm

//输入参数level是当前block update的那层level，从superpixel那层level下面那层level开始
//req_confidence初始化就是0

//这一个就是主要算法，用遍历的方式，看看某个grid是分到左边的label好呢，还是右边好，是上面好呢，还是下面好，然后更新label
void SEEDS::update_blocks(int level, float req_confidence)
{
    int labelA;
    int labelB;
    int sublabel;
    bool done;
    
    //step是当前level上面，width上面的seed的数量，也就是当前level grid在宽度上的数量
    int step = nr_w[level];
    
    // horizontal bidirectional block updating
    //他是按照一个扫描的方式，对于每个grid都尝试一下
    for (int y=1; y<nr_h[level]-1; y++)
        for (int x=1; x<nr_w[level]-2; x++)
        {
            // choose a label at the current level
            sublabel = y*step+x;
            // get the label at the top level (= superpixel label)，因为第一次执行的时候，这个level=seeds_current_level，也就是superpixel level下面那一层
            
            //labelA也就是当前grid(x,y)在superpixel那层的grid编号
            labelA = parent[level][y*step+x];
            // get the neighboring label at the top level (= superpixel label)
            //labelB，就是当前grid右边1个grid在superpixel那层的编号
            labelB = parent[level][y*step+x+1];
            
            //如果labelA，labelB正好是在一个superpixel中，其实就不需要下面这么多更新，如果两个不是，那么就看看是否需要进行merge
            if (labelA != labelB)
            {
                // get the surrounding labels at the top level, to check for splitting
                int a11 = parent[level][(y-1)*step+(x-1)];
                int a12 = parent[level][(y-1)*step+(x)];
                int a13 = parent[level][(y-1)*step+(x+1)];
                int a14 = parent[level][(y-1)*step+(x+2)];
                int a21 = parent[level][(y)*step+(x-1)];
                int a22 = parent[level][(y)*step+(x)];
                int a23 = parent[level][(y)*step+(x+1)];
                int a24 = parent[level][(y)*step+(x+2)];
                int a31 = parent[level][(y+1)*step+(x-1)];
                int a32 = parent[level][(y+1)*step+(x)];
                int a33 = parent[level][(y+1)*step+(x+1)];
                int a34 = parent[level][(y+1)*step+(x+2)];
                
                done = false;
                
                //还是不知道为什么要检查这个nr_partitions的数量，那个参数MINIUM=1，这个是为了防止你最后走到level=0的时候，这个最小精密度的grid，nr_partition就是=1，这个时候也就不需要下面者一些列的更新了
                if (nr_partitions[seeds_top_level][labelA] > MINIMUM_NR_SUBLABELS)
                {
                    // remove sublabel from A, get intersections with A and B
                    if (nr_partitions[seeds_top_level][labelA] <= 2) // == 2
                    {
                        // run algorithm as usual
                        
                        //首先把当前的grid region的histogram从top level的grid region的为labelA的那个region中，把sublabel这个小region给踢出去
                        delete_block(seeds_top_level, labelA, level, sublabel);
                        
                        //再把剔除后的parent level的grid region的histogram和这个剔除出去的grid region相交
                        //这个函数中参数，level正好比seed_top_level要小1
                        float intA = intersection(seeds_top_level, labelA, level, sublabel);
                        float intB = intersection(seeds_top_level, labelB, level, sublabel);
                        float confidence = fabs(intA - intB);
                        // add to label with highest intersection
                        if ((intB > intA) && (confidence > req_confidence))
                        {
                            add_block(seeds_top_level, labelB, level, sublabel);
                            done = true;
                        }
                        else
                        {
                            add_block(seeds_top_level, labelA, level, sublabel);
                        }
                    }
                    //如果当前的superpixel region的partition大于2，也就是会出现split的情况，这个时候就要检测是否需要check_split
                    else if (nr_partitions[seeds_top_level][labelA] > 2) // 3 or more partitions
                    {
                        //a11，a12是当前grid在上一层个parent level的label
                        //这里是要吧当前的grid分成labelB，也就是a23的label，这里check_split检查的内容是，a22和a22左边的label是否相等，如果不相等，则这个check就返回true，这个时候如果你把a22的label又变成labelB,也就是a23的label，就形成了a21,a22,a23三个label都不同，也就把labelA给split开来，这个时候就不能进行merge，就要避免这种情况
                        if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                        {
                            // run algorithm as usual
                            delete_block(seeds_top_level, labelA, level, sublabel);
                            float intA = intersection(seeds_top_level, labelA, level, sublabel);
                            float intB = intersection(seeds_top_level, labelB, level, sublabel);
                            float confidence = fabs(intA - intB);
                            // add to label with highest intersection
                            if ((intB > intA) && (confidence > req_confidence))
                            {
                                add_block(seeds_top_level, labelB, level, sublabel);
                                done = true;
                            }
                            else
                            {
                                add_block(seeds_top_level, labelA, level, sublabel);
                            }
                        }
                    }
                }
                
                //之前的时判断nr_partitions[seeds_top_level][labelA]是否大于MININUM，现在变成labelB
                // try opposite direction，这个时候就是尝试把x+1,y)这个grid来移动，移动到labelA中，而之前是吧x，y这个grid移动到labelB中，也就是对于boundary上面的grid，是把左边移动到右边，还是把右边的subgrid移动到左边，还是右边都是常识一下
                
                if ((!done) && (nr_partitions[seeds_top_level][labelB] > MINIMUM_NR_SUBLABELS))
                {
                    
                    //labelB = parent[level][y*step+x+1];也就是说，labelB其实就是sublabel的superpixel对应的label，
                    sublabel = y*step+x+1;
                    if (nr_partitions[seeds_top_level][labelB] <= 2) // == 2
                    {
                        // run algorithm as usual
                        delete_block(seeds_top_level, labelB, level, sublabel);
                        float intA = intersection(seeds_top_level, labelA, level, sublabel);
                        float intB = intersection(seeds_top_level, labelB, level, sublabel);
                        float confidence = fabs(intA - intB);
                        if ((intA > intB) && (confidence > req_confidence))
                        {
                            add_block(seeds_top_level, labelA, level, sublabel);
                            x++;
                        }
                        else
                        {
                            add_block(seeds_top_level, labelB, level, sublabel);
                        }
                    }
                    else if (nr_partitions[seeds_top_level][labelB] > 2)
                    {
                        if (!check_split(a12, a13, a14, a22, a23, a24, a32, a33, a34, true, false))
                        {
                            // run algorithm as usual
                            delete_block(seeds_top_level, labelB, level, sublabel);
                            float intA = intersection(seeds_top_level, labelA, level, sublabel);
                            float intB = intersection(seeds_top_level, labelB, level, sublabel);
                            float confidence = fabs(intA - intB);
                            if ((intA > intB) && (confidence > req_confidence))
                            {
                                add_block(seeds_top_level, labelA, level, sublabel);
                                x++;
                            }
                            else
                            {
                                add_block(seeds_top_level, labelB, level, sublabel);
                            }
                        }
                    }
                }
            }
        }
    
    //第一次传入这个函数的时候这个level就是top level下面的那一层
    //因为之前add_block()的操作，仅仅是更新了parent，但是对于grid本身的label还没有更新，这里就是利用之前的parent，把本身的label也更新了
    update_labels(level);
    
    // vertical bidirectional
    for (int x=1; x<nr_w[level]-1; x++)
        for (int y=1; y<nr_h[level]-2; y++)
        {
            // choose a label at the current level
            sublabel = y*step+x;
            // get the label at the top level (= superpixel label)
            labelA = parent[level][y*step+x];
            // get the neighboring label at the top level (= superpixel label)
            labelB = parent[level][(y+1)*step+x];
            
            if (labelA != labelB)
            {
                int a11 = parent[level][(y-1)*step+(x-1)];
                int a12 = parent[level][(y-1)*step+(x)];
                int a13 = parent[level][(y-1)*step+(x+1)];
                int a21 = parent[level][(y)*step+(x-1)];
                int a22 = parent[level][(y)*step+(x)];
                int a23 = parent[level][(y)*step+(x+1)];
                int a31 = parent[level][(y+1)*step+(x-1)];
                int a32 = parent[level][(y+1)*step+(x)];
                int a33 = parent[level][(y+1)*step+(x+1)];
                int a41 = parent[level][(y+2)*step+(x-1)];
                int a42 = parent[level][(y+2)*step+(x)];
                int a43 = parent[level][(y+2)*step+(x+1)];
                
                done = false;
                if (nr_partitions[seeds_top_level][labelA] > MINIMUM_NR_SUBLABELS)
                {
                    // remove sublabel from A, get intersections with A and B
                    if (nr_partitions[seeds_top_level][labelA] <= 2) // == 2
                    {
                        // run algorithm as usual
                        delete_block(seeds_top_level, labelA, level, sublabel);
                        float intA = intersection(seeds_top_level, labelA, level, sublabel);
                        float intB = intersection(seeds_top_level, labelB, level, sublabel);
                        float confidence = fabs(intA - intB);
                        // add to label with highest intersection
                        if ((intB > intA) && (confidence > req_confidence))
                        {
                            add_block(seeds_top_level, labelB, level, sublabel);
                            //y++;
                            done = true;
                        }
                        else
                        {
                            add_block(seeds_top_level, labelA, level, sublabel);
                        }
                    }
                    //还是不明白为什么这里要判断是否大于2
                    else if (nr_partitions[seeds_top_level][labelA] > 2) // 3 or more partitions
                    {
                        //a11，a12是当前grid在上一层个parent level的label
                        if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, false, true))
                        {
                            // run algorithm as usual
                            delete_block(seeds_top_level, labelA, level, sublabel);
                            float intA = intersection(seeds_top_level, labelA, level, sublabel);
                            float intB = intersection(seeds_top_level, labelB, level, sublabel);
                            float confidence = fabs(intA - intB);
                            // add to label with highest intersection
                            if ((intB > intA) && (confidence > req_confidence))
                            {
                                add_block(seeds_top_level, labelB, level, sublabel);
                                //y++;
                                done = true;
                            }
                            else
                            {
                                add_block(seeds_top_level, labelA, level, sublabel);
                            }
                        }
                    }
                }
                
                if ((!done) && (nr_partitions[seeds_top_level][labelB] > MINIMUM_NR_SUBLABELS))
                {
                    // try opposite direction
                    sublabel = (y+1)*step+x;
                    if (nr_partitions[seeds_top_level][labelB] <= 2) // == 2
                    {
                        // run algorithm as usual
                        delete_block(seeds_top_level, labelB, level, sublabel);
                        float intA = intersection(seeds_top_level, labelA, level, sublabel);
                        float intB = intersection(seeds_top_level, labelB, level, sublabel);
                        float confidence = fabs(intA - intB);
                        if ((intA > intB) && (confidence > req_confidence))
                        {
                            add_block(seeds_top_level, labelA, level, sublabel);
                            y++;
                        }
                        else
                        {
                            add_block(seeds_top_level, labelB, level, sublabel);
                        }
                    }
                    else if (nr_partitions[seeds_top_level][labelB] > 2)
                    {
                        if (!check_split(a21, a22, a23, a31, a32, a33, a41, a42, a43, false, false))
                        {
                            // run algorithm as usual
                            delete_block(seeds_top_level, labelB, level, sublabel);
                            float intA = intersection(seeds_top_level, labelA, level, sublabel);
                            float intB = intersection(seeds_top_level, labelB, level, sublabel);
                            float confidence = fabs(intA - intB);
                            if ((intA > intB) && (confidence > req_confidence))
                            {
                                add_block(seeds_top_level, labelA, level, sublabel);
                                y++;
                            }
                            else
                            {
                                add_block(seeds_top_level, labelB, level, sublabel);
                            }
                        }
                    }
                }
            }
        }
    update_labels(level);
}


//把nr_partition[]进行更新，同时根据上一层的parent的指向，求出当前level的grid对应top level的superpixel的label
int SEEDS::go_down_one_level()
{
    int old_level = seeds_current_level;
    int new_level = seeds_current_level - 1;
    
    if (new_level < 0) return -1;
    
    // go through labels of top level
    for (int x=0; x<nr_w[seeds_top_level]; x++)
        for (int y=0; y<nr_h[seeds_top_level]; y++)
        {
            // reset nr_partitions
            nr_partitions[seeds_top_level][y*nr_w[seeds_top_level]+x] = 0;
        }
    
    // go through labels of new_level
    for (int x=0; x<nr_w[new_level]; x++)
        for (int y=0; y<nr_h[new_level]; y++)
        {
            // assign parent = parent of old_label，换句话说，这个parent一直都是指向top_level的grid region的，底层的grid指向那个superpixel，是靠这个parent一层一层传递过去的
            int p = parent[old_level][parent[new_level][y*nr_w[new_level]+x]];
            parent[new_level][y*nr_w[new_level]+x] = p;
            // add nr_partitions[label] to parent'
            //当不断往下时候，partition就开始变得多了，其他level的nr_partition[]是在compute histogram哪里更新得到的，就是某一块grid是由多少块finest grid拼凑出来的
            nr_partitions[seeds_top_level][p] += nr_partitions[new_level][y*nr_w[new_level]+x];
        }
    
    return new_level;
}



// Border Updating Algorithm
void SEEDS::update_pixels()
{
    int labelA;
    int labelB;
    int priorA=0;
    int priorB=0;
    
    if (forwardbackward)
    {
        forwardbackward = false;
        for (int y=1; y<height-1; y++)
            for (int x=1; x<width-2; x++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a14 = labels[seeds_top_level][(y-1)*width+(x+2)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a24 = labels[seeds_top_level][(y)*width+(x+2)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a34 = labels[seeds_top_level][(y+1)*width+(x+2)];
                
                // horizontal bidirectional
                labelA = a22;
                labelB = a23;
                if (labelA != labelB)
                {
                    //labelA是a22，labelB是a23，也就是说，labelA是中心pixel的label，labelB是右边的那个pixel的label，如果进入这个循环，说明labelA和labelB不相等，也就是pixel和他右边的pixel不相等。
                    //接下来check的时候，就是check a22它左边的是否也不想等，如果也不相等，但是和这个pixel的上面以及下面的相等，这个时候把labelA分到labelB，就会将原来的labelA的那个region给割裂开来，就不对了，
                    
                    //只有当不会割裂开来的时候，我们才把labelA给分到labelB
                    if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                    {
#ifdef PRIOR
                        priorA = threebyfour(x,y,labelA);
                        priorB = threebyfour(x,y,labelB);
#endif
                        /*
                         看x，y的颜色，是在labelA中的histogram的probability大，还是在
                         labelB的histogram的probability大，那个prior现在都没有定义，所以
                         就不要管它
                         
                         如果在x，y的颜色在labelB的histogram的probability大，那么就把这个pixel更新成labelB
                         
                         其实这个算法判断pixel属于那个region很简单，就是看看我这个pixel的颜色对应的bin，在哪个region的histogram中的那个bin比较大。
                         
                         这个和之前block update不同的时，这里仅仅检查一个bin，而之前是intersection多个bin
                         */
                        if (probability(image_bins[y*width+x], labelA, labelB, priorA, priorB))
                        {
                            update(seeds_top_level, labelB, x, y);
                        }
                        //这里check的时候，a23是中心的那个pixel，也就是labelB的那个pixel，因为是backward，所以这里是a23和他右边的那些比较，比如a23和a24是不一个label的，但是a23和a13，以及a33都是labelB，这时候如果把a23分给labelA，就会把a13，a33者两个labelB的给割裂开来
                        else if (!check_split(a12, a13, a14, a22, a23, a24, a32, a33, a34, true, false))
                        {
                            if (probability(image_bins[y*width+x+1], labelB, labelA, priorB, priorA))
                            {
                                update(seeds_top_level, labelA, x+1, y);
                                x++;
                            }
                        }
                    }
                }
            }
        
        
        for (int x=1; x<width-1; x++)
            for (int y=1; y<height-2; y++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a41 = labels[seeds_top_level][(y+2)*width+(x-1)];
                int a42 = labels[seeds_top_level][(y+2)*width+(x)];
                int a43 = labels[seeds_top_level][(y+2)*width+(x+1)];
                
                // vertical bidirectional
                labelA = a22;
                labelB = a32;
                if (labelA != labelB)
                {
                    if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, false, true))
                    {
#ifdef PRIOR
                        priorA = fourbythree(x,y,labelA);
                        priorB =  fourbythree(x,y,labelB);
#endif
                        
                        if (probability(image_bins[y*width+x], labelA, labelB, priorA, priorB))
                        {
                            update(seeds_top_level, labelB, x, y);
                        }
                        else if (!check_split(a21, a22, a23, a31, a32, a33, a41, a42, a43, false, false))
                        {
                            if (probability(image_bins[(y+1)*width+x], labelB, labelA, priorB, priorA))
                            {
                                update(seeds_top_level, labelA, x, y+1);
                                y++;
                            }
                        }
                    }
                }
            }
    }
    
    //这里又变成当forward是false的时候，就进行下面这一系列的做法
    else
    {
        forwardbackward = true;
        for (int y=1; y<height-1; y++)
            for (int x=1; x<width-2; x++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a14 = labels[seeds_top_level][(y-1)*width+(x+2)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a24 = labels[seeds_top_level][(y)*width+(x+2)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a34 = labels[seeds_top_level][(y+1)*width+(x+2)];
                
                // horizontal bidirectional
                labelA = a22;
                labelB = a23;
                if (labelA != labelB)
                {
                    
                    //forward不同的区别就是，之前是
                    /*
                     !check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                     
                     也即是这里的中心变成了labelB，我要看看把labelB grid a23分给labelA行不行，而之前的forward=ture的时候，check的时我把labelA的a22分给labelB行不行
                     */
                    if (!check_split(a12, a13, a14, a22, a23, a24, a32, a33, a34, true, false))
                    {
#ifdef PRIOR
                        priorA = threebyfour(x,y,labelA);
                        priorB = threebyfour(x,y,labelB);
#endif
                        
                        if (probability(image_bins[y*width+x+1], labelB, labelA, priorB, priorA))
                        {
                            update(seeds_top_level, labelA, x+1, y);
                            x++;
                        }
                        else if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                        {
                            if (probability(image_bins[y*width+x], labelA, labelB, priorA, priorB))
                            {
                                update(seeds_top_level, labelB, x, y);
                            }
                        }
                    }
                }
            }
        
        
        for (int x=1; x<width-1; x++)
            for (int y=1; y<height-2; y++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a41 = labels[seeds_top_level][(y+2)*width+(x-1)];
                int a42 = labels[seeds_top_level][(y+2)*width+(x)];
                int a43 = labels[seeds_top_level][(y+2)*width+(x+1)];
                
                // vertical bidirectional
                labelA = a22;
                labelB = a32;
                if (labelA != labelB)
                {
                    if (!check_split(a21, a22, a23, a31, a32, a33, a41, a42, a43, false, false))
                    {
#ifdef PRIOR
                        priorA = fourbythree(x,y,labelA);
                        priorB =  fourbythree(x,y,labelB);
#endif
                        
                        if (probability(image_bins[(y+1)*width+x], labelB, labelA, priorB, priorA))
                        {
                            update(seeds_top_level, labelA, x, y+1);
                            y++;
                        }
                        else if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, false, true))
                        {
                            if (probability(image_bins[y*width+x], labelA, labelB, priorA, priorB))
                            {
                                update(seeds_top_level, labelB, x, y);
                            }
                        }
                    }
                }
            }
        
    }
    
    // update border pixels，对于image的上边界和下边界进行更新
    for (int x=0; x<width; x++)
    {
        labelA = labels[seeds_top_level][x];
        labelB = labels[seeds_top_level][width+x];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, x, 0);
        }
        labelA = labels[seeds_top_level][(height-1)*width + x];
        labelB = labels[seeds_top_level][(height-2)*width + x];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, x, height-1);
        }
    }
    for (int y=0; y<height; y++)
    {
        labelA = labels[seeds_top_level][y*width];
        labelB = labels[seeds_top_level][y*width+1];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, 0, y);
        }
        labelA = labels[seeds_top_level][y*width + width - 1];
        labelB = labels[seeds_top_level][y*width + width - 2];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, width-1, y);
        }
    }
}


// Border Updating Algorithm
void SEEDS::update_pixels_means()
{
    int labelA;
    int labelB;
    int priorA=0;
    int priorB=0;
    
    if (forwardbackward)
    {
        forwardbackward = false;
        for (int y=1; y<height-1; y++)
            for (int x=1; x<width-2; x++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a14 = labels[seeds_top_level][(y-1)*width+(x+2)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a24 = labels[seeds_top_level][(y)*width+(x+2)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a34 = labels[seeds_top_level][(y+1)*width+(x+2)];
                
                // horizontal bidirectional
                labelA = a22;
                labelB = a23;
                if (labelA != labelB)
                {
                    if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                    {
#ifdef PRIOR
                        /*
                         这个是输一个3*4的window中，处于x，y的这个pixel周围neighbor的pixel和
                         函数输入的label是否相等,返回的时x，y周围3*4 window里面和labelA相等
                         的pixel数量
                         
                         priorA：x，y这个pixel周围3*4的window中labelA的pixel数量
                         */
                        priorA = threebyfour(x,y,labelA);
                        priorB = threebyfour(x,y,labelB);
#endif
                        /*
                         如果我x，y这个pixel和labelA的差别比和labelB的pixel的平均差别大，
                         就把x，y分配给labelB
                         
                         */
                        if (probability_means(image_l[y*width+x], image_a[y*width+x], image_b[y*width+x], labelA, labelB, priorA, priorB))
                        {
                            update(seeds_top_level, labelB, x, y);
                        }
                        else if (!check_split(a12, a13, a14, a22, a23, a24, a32, a33, a34, true, false))
                        {
                            if (probability_means(image_l[y*width+x+1], image_a[y*width+x+1], image_b[y*width+x+1], labelB, labelA, priorB, priorA))
                            {
                                update(seeds_top_level, labelA, x+1, y);
                                x++;
                            }
                        }
                    }
                }
            }
        
        
        for (int x=1; x<width-1; x++)
            for (int y=1; y<height-2; y++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a41 = labels[seeds_top_level][(y+2)*width+(x-1)];
                int a42 = labels[seeds_top_level][(y+2)*width+(x)];
                int a43 = labels[seeds_top_level][(y+2)*width+(x+1)];
                
                // vertical bidirectional
                labelA = a22;
                labelB = a32;
                if (labelA != labelB)
                {
                    if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, false, true))
                    {
#ifdef PRIOR
                        priorA = fourbythree(x,y,labelA);
                        priorB =  fourbythree(x,y,labelB);
#endif
                        
                        if (probability_means(image_l[y*width+x], image_a[y*width+x], image_b[y*width+x], labelA, labelB, priorA, priorB))
                        {
                            update(seeds_top_level, labelB, x, y);
                        }
                        else if (!check_split(a21, a22, a23, a31, a32, a33, a41, a42, a43, false, false))
                        {
                            if (probability_means(image_l[(y+1)*width+x], image_a[(y+1)*width+x], image_b[(y+1)*width+x], labelB, labelA, priorB, priorA))
                            {
                                update(seeds_top_level, labelA, x, y+1);
                                y++;
                            }
                        }
                    }
                }
            }
    }
    else
    {
        forwardbackward = true;
        for (int y=1; y<height-1; y++)
            for (int x=1; x<width-2; x++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a14 = labels[seeds_top_level][(y-1)*width+(x+2)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a24 = labels[seeds_top_level][(y)*width+(x+2)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a34 = labels[seeds_top_level][(y+1)*width+(x+2)];
                
                // horizontal bidirectional
                labelA = a22;
                labelB = a23;
                if (labelA != labelB)
                {
                    if (!check_split(a12, a13, a14, a22, a23, a24, a32, a33, a34, true, false))
                    {
#ifdef PRIOR
                        priorA = threebyfour(x,y,labelA);
                        priorB = threebyfour(x,y,labelB);
#endif
                        
                        if (probability_means(image_l[y*width+x+1], image_a[y*width+x+1], image_b[y*width+x+1], labelB, labelA, priorB, priorA))
                        {
                            update(seeds_top_level, labelA, x+1, y);
                            x++;
                        }
                        else if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, true, true))
                        {
                            if (probability_means(image_l[y*width+x], image_a[y*width+x], image_b[y*width+x], labelA, labelB, priorA, priorB))
                            {
                                update(seeds_top_level, labelB, x, y);
                            }
                        }
                    }
                }
            }
        
        
        for (int x=1; x<width-1; x++)
            for (int y=1; y<height-2; y++)
            {
                int a11 = labels[seeds_top_level][(y-1)*width+(x-1)];
                int a12 = labels[seeds_top_level][(y-1)*width+(x)];
                int a13 = labels[seeds_top_level][(y-1)*width+(x+1)];
                int a21 = labels[seeds_top_level][(y)*width+(x-1)];
                int a22 = labels[seeds_top_level][(y)*width+(x)];
                int a23 = labels[seeds_top_level][(y)*width+(x+1)];
                int a31 = labels[seeds_top_level][(y+1)*width+(x-1)];
                int a32 = labels[seeds_top_level][(y+1)*width+(x)];
                int a33 = labels[seeds_top_level][(y+1)*width+(x+1)];
                int a41 = labels[seeds_top_level][(y+2)*width+(x-1)];
                int a42 = labels[seeds_top_level][(y+2)*width+(x)];
                int a43 = labels[seeds_top_level][(y+2)*width+(x+1)];
                
                // vertical bidirectional
                labelA = a22;
                labelB = a32;
                if (labelA != labelB)
                {
                    if (!check_split(a21, a22, a23, a31, a32, a33, a41, a42, a43, false, false))
                    {
#ifdef PRIOR
                        priorA = fourbythree(x,y,labelA);
                        priorB =  fourbythree(x,y,labelB);
#endif
                        
                        if (probability_means(image_l[(y+1)*width+x], image_a[(y+1)*width+x], image_b[(y+1)*width+x], labelB, labelA, priorB, priorA))
                        {
                            update(seeds_top_level, labelA, x, y+1);
                            y++;
                        }
                        else if (!check_split(a11, a12, a13, a21, a22, a23, a31, a32, a33, false, true))
                        {
                            if (probability_means(image_l[y*width+x], image_a[y*width+x], image_b[y*width+x], labelA, labelB, priorA, priorB))
                            {
                                update(seeds_top_level, labelB, x, y);
                            }
                        }
                    }
                }
            }
        
    }
    
    // update border pixels
    
    /*
     这里是对上边界和下边界的pixel进行更新，labels[seeds_top_level][x];就是image最上面那条边的
     pixel的值
     
     
     
     */
    
    //这里是image对上下两条边界进行更新，保证最上面那条边的上的pixel和他们下面的pixel相等
    
    for (int x=0; x<width; x++)
    {
        labelA = labels[seeds_top_level][x];
        labelB = labels[seeds_top_level][width+x];
        if (labelA != labelB)
        {
            /*
             把这个x，0的pixel更新成labelB，也就是x,1的label
             */
            update(seeds_top_level, labelB, x, 0);
        }
        labelA = labels[seeds_top_level][(height-1)*width + x];
        labelB = labels[seeds_top_level][(height-2)*width + x];
        
        /*
         把x，height-1，这个pixel的label更新成，x，height-2的那个label
         
         */
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, x, height-1);
        }
    }
    //这里是对左右两天边界更新，
    for (int y=0; y<height; y++)
    {
        labelA = labels[seeds_top_level][y*width];
        labelB = labels[seeds_top_level][y*width+1];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, 0, y);
        }
        labelA = labels[seeds_top_level][y*width + width - 1];
        labelB = labels[seeds_top_level][y*width + width - 2];
        if (labelA != labelB)
        {
            update(seeds_top_level, labelB, width-1, y);
        }
    }
}



void SEEDS::update(int level, int label_new, int x, int y)
{
    int label_old = labels[level][y*width+x];
    delete_pixel_m(level, label_old, x, y);
    add_pixel_m(level, label_new, x, y);
    labels[level][y*width+x] = label_new;
}
//吧某个pixel添加到对应grid region的histogram的对应bin中，记录对应label的grid region的pixel数量T
void SEEDS::add_pixel(int level, int label, int x, int y)
{
    histogram[level][label][image_bins[y*width+x]]++;
    T[level][label]++;
}

void SEEDS::add_pixel_m(int level, int label, int x, int y)
{
    histogram[level][label][image_bins[y*width+x]]++;
    T[level][label]++;
    
#ifdef MEANS
    L_channel[label] += image_l[y*width + x];
    A_channel[label] += image_a[y*width + x];
    B_channel[label] += image_b[y*width + x];
#endif
}

void SEEDS::delete_pixel(int level, int label, int x, int y)
{
    histogram[level][label][image_bins[y*width+x]]--;
    T[level][label]--;
}

void SEEDS::delete_pixel_m(int level, int label, int x, int y)
{
    histogram[level][label][image_bins[y*width+x]]--;
    T[level][label]--;
    
#ifdef MEANS
    L_channel[label] -= image_l[y*width + x];
    A_channel[label] -= image_a[y*width + x];
    B_channel[label] -= image_b[y*width + x];
#endif
}


void SEEDS::add_block(int level, int label, int sublevel, int sublabel)
{
    //几把，这一句是费的，没有任何意义，
    //不是的，是有意义的，在移动block的时候，我移动一个grid region之后，就需要把他的parent给进行更新
    parent[sublevel][sublabel] = label;
    
    for (int n=0; n<histogram_size; n++)
    {
        histogram[level][label][n] += histogram[sublevel][sublabel][n];
    }
    T[level][label] += T[sublevel][sublabel];
    
    //在compute histogram中这里把nr_partitions[][]给进行了更新也就是某个level的某个label的grid上面被分了几次，换句话就是说，这个grid region是吸收了几个sublevel得到的
    
    //在merge函数中这个level 参数一致都是top level
    nr_partitions[level][label]++;
}


//首先把当前的grid region的histogram从top level的grid region的为labelA的那个region中，把sublabel这个小region给踢出去
void SEEDS::delete_block(int level, int label, int sublevel, int sublabel)
{
    //这里把parent置为-1，是因为我们还不能缺点要不呀把sublevel中的sublabel移动到level中的别的label中
    parent[sublevel][sublabel] = -1;
    
    for (int n=0; n<histogram_size; n++)
    {
        histogram[level][label][n] -= histogram[sublevel][sublabel][n];
    }
    T[level][label] -= T[sublevel][sublabel];
    
    nr_partitions[level][label]--;
}



void SEEDS::update_labels(int level)
{
    for (int i=0; i<width*height; i++)
    {
        labels[seeds_top_level][i] = parent[level][labels[level][i]];
    }
}




bool SEEDS::probability(int color, int label1, int label2, int prior1, int prior2)
{
    float P_label1 = (float)histogram[seeds_top_level][label1][color] / (float)T[seeds_top_level][label1];
    float P_label2 = (float)histogram[seeds_top_level][label2][color] / (float)T[seeds_top_level][label2];
    
#ifdef PRIOR
    P_label1 *= (float) prior1;
    P_label2 *= (float) prior2;
#endif
    
    return (P_label2 > P_label1);
}


bool SEEDS::probability_means(float L, float a, float b, int label1, int label2, int prior1, int prior2)
{
    /*
     L chanel上面，label1的颜色的值，除以label1的pixel数量，所以L1就是label1的颜色的平均值
     
     */
    float L1 = L_channel[label1] / T[seeds_top_level][label1];
    float a1 = A_channel[label1] / T[seeds_top_level][label1];
    float b1 = B_channel[label1] / T[seeds_top_level][label1];
    float L2 = L_channel[label2] / T[seeds_top_level][label2];
    float a2 = A_channel[label2] / T[seeds_top_level][label2];
    float b2 = B_channel[label2] / T[seeds_top_level][label2];
    
    /*
     L,a,b是image在x，y这个左边上面的lab intensity value，
     
     P_label1就是这个x，y的pixel的lab的值和属于label1的pixel的平均值之间的不哈别
     
     */
    float P_label1 = (L-L1)*(L-L1) + (a-a1)*(a-a1) + (b-b1)*(b-b1);
    float P_label2 = (L-L2)*(L-L2) + (a-a2)*(a-a2) + (b-b2)*(b-b2);
    
#ifdef PRIOR
    //priorA：x，y这个pixel周围3*4的window中labelA的pixel数量
    /*
     用P_label1就是这个x，y的pixel的lab的值和属于label1的pixel的平均值之间的差别，除以
     x，y这个pixel周围属于label1的pixel数量
     
     */
    P_label1 /= prior1;
    P_label2 /= prior2;
#endif
    
    return (P_label1 > P_label2);
}



void SEEDS::LAB2RGB(float L, float a, float b, int* R, int* G, int* B)
{
    float T1 = 0.008856;
    float T2 = 0.206893;
    
    float fY = pow((L + 16.0) / 116.0, 3);
    bool YT = fY > T1;
    fY = (!YT) * (L / 903.3) + YT * fY;
    float Y = fY;
    
    // Alter fY slightly for further calculations
    fY = YT * pow(fY,1.0/3.0) + (!YT) * (7.787 * fY + 16.0/116.0);
    
    float fX = a / 500.0 + fY;
    bool XT = fX > T2;
    float X = (XT * pow(fX, 3) + (!XT) * ((fX - 16.0/116.0) / 7.787));
    
    float fZ = fY - b / 200.0;
    bool ZT = fZ > T2;
    float Z = (ZT * pow(fZ, 3) + (!ZT) * ((fZ - 16.0/116.0) / 7.787));
    
    X = X * 0.950456 * 255.0;
    Y = Y * 255.0;
    Z = Z * 1.088754 * 255.0;
    
    *R = (int) (3.240479*X - 1.537150*Y - 0.498535*Z);
    *G = (int) (-0.969256*X + 1.875992*Y + 0.041556*Z);
    *B = (int) (0.055648*X - 0.204043*Y + 1.057311*Z);
}


int SEEDS::RGB2LAB(const int& r, const int& g, const int& b, float* lval, float* aval, float* bval)
{
    float xVal = 0.412453 * r + 0.357580 * g + 0.180423 * b;
    float yVal = 0.212671 * r + 0.715160 * g + 0.072169 * b;
    float zVal = 0.019334 * r + 0.119193 * g + 0.950227 * b;
    
    xVal /= (255.0 * 0.950456);
    yVal /=  255.0;
    zVal /= (255.0 * 1.088754);
    
    float fY, fZ, fX;
    float lVal, aVal, bVal;
    float T = 0.008856;
    
    bool XT = (xVal > T);
    bool YT = (yVal > T);
    bool ZT = (zVal > T);
    
    fX = XT * pow(xVal,1.0/3.0) + (!XT) * (7.787 * xVal + 16.0/116.0);
    
    // Compute L
    float Y3 = pow(yVal,1.0/3.0);
    fY = YT*Y3 + (!YT)*(7.787*yVal + 16.0/116.0);
    lVal  = YT * (116 * Y3 - 16.0) + (!YT)*(903.3*yVal);
    
    fZ = ZT*pow(zVal,1.0/3.0) + (!ZT)*(7.787*zVal + 16.0/116.0);
    
    // Compute a and b
    aVal = 500 * (fX - fY);
    bVal = 200 * (fY - fZ);
    
    *lval = lVal;
    *aval = aVal;
    *bval = bVal;
    
    return 1; //LAB2bin(lVal, aVal, bVal);
}



int SEEDS::RGB2HSV(const int& r, const int& g, const int& b, float* hval, float* sval, float* vval)
{
    float r_ = r / 256.0;
    float g_ = g / 256.0;
    float b_ = b / 256.0;
    
    float min_rgb = min(r_, min(g_, b_));
    float max_rgb = max(r_, max(g_, b_));
    float V = max_rgb;
    float H = 0.0;
    float S = 0.0;
    
    float delta = max_rgb - min_rgb;
    
    if ((delta > 0.0) && (max_rgb > 0.0))
    {
        S = delta / max_rgb;
        if (max_rgb == r_)
            H = (g_ - b_) / delta;
        else if (max_rgb == g_)
            H = 2 + (b_ - r_) / delta;
        else
            H = 4 + (r_ - g_) / delta;
    }
    
    H /= 6;
    
    /*float V = max(r_,max(g_,b_));
     float S = 0.0;
     if (V != 0.0) S = (V - min(r_, min(g_,b_)))/V;
     float H = 0.0;
     if (S != 0.0)
     {
     if (V == r_)
     H = ((g_ - b_)/6.0)/S;
     else if (V == g_)
     H = 1.0/2.0 + ((b_-r_)/6.0)/S;
     else
     H = 2.0/3.0 + ((r_-g_)/6.0)/S;
     }*/
    
    if (H<0.0) H += 1.0;
    
    *hval = H;
    *sval = S;
    *vval = V;
    
    int hbin = int(H * nr_bins);
    int sbin = int(S * nr_bins);
    int vbin = int(V * nr_bins);
    if(sbin == nr_bins) --sbin; //S can be equal to 1.0
    
    //printf("%d %d %d -- %f %f %f -- bins %d %d %d\n", r, g, b, H, S, V, hbin, sbin, vbin);
    
    return hbin + nr_bins*(sbin + nr_bins*vbin);
}

int SEEDS::LAB2bin(float l, float a, float b)
{
    // binning
    int bin_l = floor(l/100.1*nr_bins);
    
    int bin_a = floor(a/100*(nr_bins-2) + (nr_bins-2)/2 + 1);
    if (bin_a < 0) bin_a = 0;
    if (bin_a >= nr_bins) bin_a = nr_bins-1;
    
    int bin_b = floor(b/100*(nr_bins-2) + (nr_bins-2)/2 + 1);
    if (bin_b < 0) bin_b = 0;
    if (bin_b >= nr_bins) bin_b = nr_bins-1;
    
    // encoding
    return bin_l + nr_bins*bin_a + nr_bins*nr_bins*bin_b;
}



int SEEDS::RGB2LAB_special(int r, int g, int b, float* lval, float* aval, float* bval)
{
    float xVal = 0.412453 * r + 0.357580 * g + 0.180423 * b;
    float yVal = 0.212671 * r + 0.715160 * g + 0.072169 * b;
    float zVal = 0.019334 * r + 0.119193 * g + 0.950227 * b;
    
    xVal /= (255.0 * 0.950456);
    yVal /=  255.0;
    zVal /= (255.0 * 1.088754);
    
    float fY, fZ, fX;
    float lVal, aVal, bVal;
    float T = 0.008856;
    
    bool XT = (xVal > T);
    bool YT = (yVal > T);
    bool ZT = (zVal > T);
    
    fX = XT * pow(xVal,1.0/3.0) + (!XT) * (7.787 * xVal + 16/116);
    
    // Compute L
    float Y3 = pow(yVal,1.0/3.0);
    fY = YT*Y3 + (!YT)*(7.787*yVal + 16/116);
    lVal  = YT * (116 * Y3 - 16.0) + (!YT)*(903.3*yVal);
    
    fZ = ZT*pow(zVal,1.0/3.0) + (!ZT)*(7.787*zVal + 16/116);
    
    // Compute a and b
    aVal = 500 * (fX - fY);
    bVal = 200 * (fY - fZ);
    
    *lval = lVal;
    *aval = aVal;
    *bval = bVal;
    
    int bin1 = 0;
    int bin2 = 0;
    int bin3 = 0;
    
    /*
     也就是说，这里的histogram其实是一个agreviate的histogram？
     不是的！
     如果我lVal应该处于第3个bin，那么这里首先bin1=0，得到bin1的cutoff value，之后如果这个lVal还是比较大，就变成了bin1=1，进入第二次循环，之后得到bin1=3
     */
    while (lVal > bin_cutoff1[bin1]) {
        bin1++;
    }
    while (aVal > bin_cutoff2[bin2]) {
        bin2++;
    }
    while (bVal > bin_cutoff3[bin3]) {
        bin3++;
    }
    
    //最后返回的时一个0-124的值
    return bin1 + nr_bins*bin2 + nr_bins*nr_bins*bin3;
}


int SEEDS::RGB2LAB_special(int r, int g, int b, int* bin_l, int* bin_a, int* bin_b)
{
    float xVal = 0.412453 * r + 0.357580 * g + 0.180423 * b;
    float yVal = 0.212671 * r + 0.715160 * g + 0.072169 * b;
    float zVal = 0.019334 * r + 0.119193 * g + 0.950227 * b;
    
    xVal /= (255.0 * 0.950456);
    yVal /=  255.0;
    zVal /= (255.0 * 1.088754);
    
    float fY, fZ, fX;
    float lVal, aVal, bVal;
    float T = 0.008856;
    
    bool XT = (xVal > T);
    bool YT = (yVal > T);
    bool ZT = (zVal > T);
    
    fX = XT * pow(xVal,1.0/3.0) + (!XT) * (7.787 * xVal + 16/116);
    
    // Compute L
    float Y3 = pow(yVal,1.0/3.0);
    fY = YT*Y3 + (!YT)*(7.787*yVal + 16/116);
    lVal  = YT * (116 * Y3 - 16.0) + (!YT)*(903.3*yVal);
    
    fZ = ZT*pow(zVal,1.0/3.0) + (!ZT)*(7.787*zVal + 16/116);
    
    // Compute a and b
    aVal = 500 * (fX - fY);
    
    bVal = 200 * (fY - fZ);
    
    //*lval = lVal;
    //*aval = aVal;
    //*bval = bVal;
    
    int bin1 = 0;
    int bin2 = 0;
    int bin3 = 0;
    
    while (lVal > bin_cutoff1[bin1]) {
        bin1++;
    }
    while (aVal > bin_cutoff2[bin2]) {
        bin2++;
    }
    while (bVal > bin_cutoff3[bin3]) {
        bin3++;
    }
    
    *bin_l = bin1;
    *bin_a = bin2;
    *bin_b = bin3;
    
    return bin1 + nr_bins*bin2 + nr_bins*nr_bins*bin3;
}

int SEEDS::threebythree(int x, int y, UINT label)
{
    int count = 0;
    const UINT* ptr_label = labels[seeds_top_level];
    const int y_width = y*width;
    if (ptr_label[y_width-width+x-1]==label) count++;
    if (ptr_label[y_width-width+x]==label) count++;
    if (ptr_label[y_width-width+x+1]==label) count++;
    
    if (ptr_label[y_width+x-1]==label) count++;
    if (ptr_label[y_width+x+1]==label) count++;
    
    if (ptr_label[y_width+width+x-1]==label) count++;
    if (ptr_label[y_width+width+x]==label) count++;
    if (ptr_label[y_width+width+x+1]==label) count++;
    
    return count;
}

/*
 这个是输一个3*4的window中，处于x，y的这个pixel周围neighbor的pixel和
 函数输入的label是否相等
 
 */

int SEEDS::threebyfour(int x, int y, UINT label)
{
    int count = 0;
    const UINT* ptr_label = labels[seeds_top_level];
    const int y_width = y*width;
    if (ptr_label[y_width-width+x-1]==label) count++;
    if (ptr_label[y_width-width+x]==label) count++;
    if (ptr_label[y_width-width+x+1]==label) count++;
    if (ptr_label[y_width-width+x+2]==label) count++;
    
    if (ptr_label[y_width+x-1]==label) count++;
    if (ptr_label[y_width+x+2]==label) count++;
    
    if (ptr_label[y_width+width+x-1]==label) count++;
    if (ptr_label[y_width+width+x]==label) count++;
    if (ptr_label[y_width+width+x+1]==label) count++;
    if (ptr_label[y_width+width+x+2]==label) count++;
    
    return count;
}

int SEEDS::fourbythree(int x, int y, UINT label)
{
    int count = 0;
    const UINT* ptr_label = labels[seeds_top_level];
    const int y_width = y*width;
    if (ptr_label[y_width-width+x-1]==label) count++;
    if (ptr_label[y_width-width+x]==label) count++;
    if (ptr_label[y_width-width+x+1]==label) count++;
    
    if (ptr_label[y_width+x-1]==label) count++;
    if (ptr_label[y_width+x+2]==label) count++;
    
    if (ptr_label[y_width+width+x-1]==label) count++;
    if (ptr_label[y_width+width+x+2]==label) count++;
    
    if (ptr_label[y_width+width*2+x-1]==label) count++;
    if (ptr_label[y_width+width*2+x]==label) count++;
    if (ptr_label[y_width+width*2+x+1]==label) count++;
    
    return count;
}


float SEEDS::intersection(int level1, int label1, int level2, int label2)
{
    //intersection of 2 histograms: take the smaller value in each bin
    //and return the sum，是[0,1]哪个frequency的sum
    int sum1 = 0, sum2=0;
    int* h1 = histogram[level1][label1];
    int* h2 = histogram[level2][label2];
    const int count1 = T[level1][label1];
    const int count2 = T[level2][label2];
    
    for (int n=0; n<histogram_size; n++)
    {
        //哦，因为histogram里面存的是number，要除以grid region number才是histogram的frequency，这里只是把哪个触发左右相乘了一下
        if(*h1 * count2 < *h2 * count1) sum1+=*h1;
        else sum2+=*h2;
        ++h1;
        ++h2;
    }
    
    return ((float)sum1)/(float)count1 + ((float)sum2)/(float)count2;
}



//labelA是a22，labelB是a23，也就是说，labelA是中心pixel的label，labelB是右边的那个pixel的label，如果进入这个循环，说明labelA和labelB不相等，也就是pixel和他右边的pixel不相等。
//接下来check的时候，就是check a22它左边的是否也不想等，如果也不相等，但是和这个pixel的上面以及下面的相等，这个时候把labelA分到labelB，就会将原来的labelA的那个region给割裂开来，就不对了，

//只有当不会割裂开来的时候，我们才把labelA给分到labelB


bool SEEDS::check_split(int a11, int a12, int a13, int a21, int a22, int a23, int a31, int a32, int a33, bool horizontal, bool forward)
{
    if (horizontal)
    {
        /*
         forward的意思就是检查3*3中心的pixel的label，和左边，也就是x轴的前面，forward的方向
         左上，左或者左下，如果任何一个是不同的，就返回true
         */
        if (forward)
        {
            if ((a22 != a21) && (a22 == a12) && (a22 == a32)) return true;
            if ((a22 != a11) && (a22 == a12) && (a22 == a21)) return true;
            if ((a22 != a31) && (a22 == a32) && (a22 == a21)) return true;
        }
        
        else /*backward*/
        {
            if ((a22 != a23) && (a22 == a12) && (a22 == a32)) return true;
            if ((a22 != a13) && (a22 == a12) && (a22 == a23)) return true;
            if ((a22 != a33) && (a22 == a32) && (a22 == a23)) return true;
        }
    }
    else /*vertical*/
    {
        if (forward)
        {
            if ((a22 != a12) && (a22 == a21) && (a22 == a23)) return true;
            if ((a22 != a11) && (a22 == a21) && (a22 == a12)) return true;
            if ((a22 != a13) && (a22 == a23) && (a22 == a12)) return true;
        }
        else /*backward*/
        {
            if ((a22 != a32) && (a22 == a21) && (a22 == a23)) return true;
            if ((a22 != a31) && (a22 == a21) && (a22 == a32)) return true;
            if ((a22 != a33) && (a22 == a23) && (a22 == a32)) return true;
        }
    }
    
    return false;
}


int SEEDS::count_superpixels()
{
    int* count_labels = new int[nr_labels[seeds_top_level]];
    for (UINT i=0; i<nr_labels[seeds_top_level]; i++)
        count_labels[i] = 0;
    for (int i=0; i<width*height; i++)
        count_labels[labels[seeds_top_level][i]] = 1;
    int count = 0;
    for (UINT i=0; i<nr_labels[seeds_top_level]; i++)
        count += count_labels[i];
    
    delete count_labels;
    return count;
}


void SEEDS::SaveLabels_Text(string filename)
{
    
    ofstream outfile;
    outfile.open(filename.c_str());
    int i = 0;
    for( int h = 0; h < height; h++ )
    {
        for( int w = 0; w < width -1; w++ )
        {
            outfile << labels[seeds_top_level][i] << " ";
            i++;
        }
        outfile << labels[seeds_top_level][i] << endl;
        i++;
    }
    outfile.close();
}


void SEEDS::compute_mean_map()
{
    means = new UINT[width*height];
    
    for (int i=0; i<width*height; i++)
    {
        int label = labels[seeds_top_level][i];
        float L = 100.0 * ((float) L_channel[label]) / T[seeds_top_level][label];
        float a = 255.0 * ((float) A_channel[label]) / T[seeds_top_level][label] - 128.0;
        float b = 255.0 * ((float) B_channel[label]) / T[seeds_top_level][label] - 128.0;
        int R, G, B;
        LAB2RGB(L, a, b, &R, &G, &B);
        means[i] = B | (G << 8) | (R << 16);
    }
}












