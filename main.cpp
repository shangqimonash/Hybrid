#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "src/SLIC.h"
#include "src/image.h"
#include "src/misc.h"
#include "src/pnmfile.h"
#include "src/segment-image.h"
#include "src/RAG.h"
#include "src/RAGEdge.h"
#include "src/RAGNode.h"

using namespace std;

int main()
{
    string fileName = "fishMan.ppm";
    printf("loading input image.\n");
    image<rgb> *input = loadPPM(fileName.c_str());

    input->labels = new int[input->width() * input->height()];
    int numLabels(0);
    SLIC slic;

    slic.PerformSLICO_ForGivenK(input->getRGBData(), input->width(), input->height(),input->labels,numLabels, 100,10);

    /*for(int j = 0; j < input->height(); j++)
    {
        for(int i = 0; i < input->width(); i++)
            cout<<input->labels[j * input->width() + i]<< " ";
        cout<<endl;
    }*/


    printf("processing\n");
    int num_ccs;
    image<rgb> *seg = segment_image(input, 0.7, 5, 100, &num_ccs);
    savePPM(seg, "output.ppm");

    printf("got %d components\n", num_ccs);
    printf("done! uff...thats hard work.\n");
}
