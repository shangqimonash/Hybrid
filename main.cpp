#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "src/SLIC.h"
#include "src/image.h"
#include "src/misc.h"
#include "src/pnmfile.h"
#include "src/segment-image.h"

using namespace std;

int main()
{
    time_t begin,end;
    string fileName = "Users/yuan/inputSuperpixel/fishMan";
    printf("loading input image.\n");
    //image<rgb> *input = loadPPM(fileName.c_str());

    ///上面转string我不知道用不了，直接hard code进来
    image<rgb> *input = loadPPM("fishMan.ppm");

    input->labels = new int[input->width() * input->height()];
    int numLabels(0);
    SLIC slic;
    begin = clock();
    slic.DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(input->getRGBData(), input->width(), input->height(),input->labels,numLabels, 400,10);
    end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << elapsed_secs << endl;
    //cout << numLabels << endl;
    /*for(int j = 0; j < input->height(); j++)
    {
        for(int i = 0; i < input->width(); i++)
            cout<<input->labels[j * input->width() + i]<< " ";
        cout<<endl;
    }*/


    printf("processing\n");
    int num_ccs;
    image<rgb> *seg = segment_image(input, 0.7, 5, 100, &num_ccs);
    savePPM(seg, "output1.ppm");

    printf("got %d components\n", num_ccs);
    printf("done! uff...thats hard work.\n");
}
