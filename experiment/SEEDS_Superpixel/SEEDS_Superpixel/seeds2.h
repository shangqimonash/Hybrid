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

#if !defined(_SEEDS_H_INCLUDED_)
#define _SEEDS_H_INCLUDED_

#include <string>

using namespace std;

typedef unsigned int UINT;


class SEEDS  
{
public:
	SEEDS(int width, int height, int nr_channels, int nr_bins);
	~SEEDS();

	// initialize. this needs to be called only once
	//the number of superpixels is:
	// (image_width/seeds_w/(2^(nr_levels-1))) * (image_height/seeds_h/(2^(nr_levels-1)))
	void initialize(int seeds_w, int seeds_h, int nr_levels);
	
	//set a new image in YCbCr format
	//image must have the same size as in the constructor was given
	void update_image_ycbcr(UINT* image);
	
	// go through iterations
	void iterate();

	UINT* get_labels() { return labels[seeds_top_level]; }
	// output labels
    
     //labels是一个2-d数组，他保存每个pixel在每个level时候的label的值
	UINT** labels;	 //[level][y * width + x]

	// evaluation
	int count_superpixels();
	void SaveLabels_Text(string filename);

	// mean colors
	void compute_mean_map();
	UINT* means;

private:
	void deinitialize();
	
	bool initialized;

	// seeds	
	int seeds_w;//最底层，最密集的grid的seed的宽度，这里为3个pixel
	int seeds_h;//最底层，最密集的grid的seed的高度，这里为4个pixel
	int seeds_nr_levels;//level的数量，这里为4，第四层就是输出的superpixel的level
	int seeds_top_level; // == seeds_nr_levels-1 (const)
    
    //current level是top level下面的那一层，也就是输出的superpixel的grid除以2之后的grid
	int seeds_current_level; //start with level seeds_top_level-1, then go down

    
   	// image processing
    /*
     输入图片image的lab三个chanel的值，这个array的size等于width*height, 里面的值是lab一个归一化之后[0,1]的值
     */
	float* image_l; //input image,取值范围是[0,1]归一化的
	float* image_a;
	float* image_b;
	UINT* image_bins; //bin index (histogram) of each image pixel [y*width + x],也就是对于每个pixel，我都需要求出他的intensity在histogram里面属于哪个bin。image_bins的长度是图片的长度，内部存数的是histogram bin index，是[0,125]这样第一个范围，如果一个pixel在lab上属于（4，2，3）这样的情况那么对于的index就是4+2*5+3*5*5
    
    
    /*
     histogram有nr_bins个bin，每个bin的cutoff value是多少保存在bin_cutoff1[]这个数组里
     
     */
	float* bin_cutoff1; //for each bin: define upper limit of L value used to calc bin index from color value
	float* bin_cutoff2; //for each bin: define upper limit of a value
	float* bin_cutoff3; //for each bin: define upper limit of b value

	// keep one labeling for each level，nr_labels就是这个level中，grid region的总数量
	UINT* nr_labels; //[level]
    
    //[level][label] = corresponding label of block with level+1，当前level的label对应上一层较大block的label
    //parent在level 0的长度，正好是这个level grid的总数量，里面存储的值则是level 0的某个grid对应上一层level 1中的哪个grid
	UINT** parent; //[level][label] = corresponding label of block with level+1
    
    //比如在最底层，这里就是1，也就是不会再细分了，如果是再倒数第二层，这里就是4，也就是当前这个grid会在被最底层分成4个
    //在这里把nr_partitions[][]给进行了更新也就是某个level的某个label的grid上面被分了几次，换句话就是说，这个grid region是吸收了几个sublevel得到的
	UINT** nr_partitions; //[level][label] how many partitions label has on level-1
    int** T; //[level][label] how many pixels with this label, 表示在某个level的某个grid上面，有多少个pixel

	int go_down_one_level();

	// initialization
	void assign_labels();
	void compute_histograms(int until_level = -1);
	void compute_means();
	//void lab_get_histogram_cutoff_values(const Image& image);
	
	// color conversion and histograms
	int RGB2HSV(const int& r, const int& g, const int& b, float* hval, float* sval, float* vval);
	int RGB2LAB(const int& r, const int& g, const int& b, float* lval, float* aval, float* bval);
	int LAB2bin(float l, float a, float b);
	int RGB2LAB_special(int r, int g, int b, float* lval, float* aval, float* bval);
	int RGB2LAB_special(int r, int g, int b, int* bin_l, int* bin_a, int* bin_b);
	void LAB2RGB(float L, float a, float b, int* R, int* G, int* B);

	int histogram_size; //= nr_bins ^ 3 (3 channels)
    
    
    //histogram[level][label][j < histogram_size],histogram是一个3d的数组，表示某个level上面某个grid(也就是label） region对应的某个bin的数量
	int*** histogram; //[level][label][j < histogram_size]
	

  void update(int level, int label_new, int x, int y);
	void add_pixel(int level, int label, int x, int y);
	void add_pixel_m(int level, int label, int x, int y);
	void delete_pixel(int level, int label, int x, int y);
	void delete_pixel_m(int level, int label, int x, int y);
	void add_block(int level, int label, int sublevel, int sublabel);
	void delete_block(int level, int label, int sublevel, int sublabel);
	void update_labels(int level);


	// probability computation
	bool probability(int color, int label1, int label2, int prior1, int prior2);
	bool probability_means(float L, float a, float b, int label1, int label2, int prior1, int prior2);

	int threebythree(int x, int y, UINT label);
	int threebyfour(int x, int y, UINT label);
	int fourbythree(int x, int y, UINT label);


	// block updating
	void update_blocks(int level, float req_confidence = 0.0);
	float intersection(int level1, int label1, int level2, int label2);

	// border updating
	void update_pixels();
	void update_pixels_means();
	bool forwardbackward;
	int threebythree_upperbound;
	int threebythree_lowerbound;

	inline bool check_split(int a11, int a12, int a13, int a21, int a22, 
			int a23, int a31, int a32, int a33, bool horizontal, bool forward);

    //level 0 时候，每一行有多少个grid region
	int* nr_w; //[level] number of seeds in x-direction
	int* nr_h;

	float* L_channel;//image 在L chanel上面，对于某个label的颜色值
	float* A_channel;
	float* B_channel;
	int width, height, nr_channels, nr_bins;
};




#endif // !defined(_SEEDS_H_INCLUDED_)
