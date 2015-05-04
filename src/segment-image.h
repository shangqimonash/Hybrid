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

#ifndef SEGMENT_IMAGE
#define SEGMENT_IMAGE

#include <cstdlib>
#include <stdlib.h>
#include <set>
#include <map>
#if defined (__linux__)
#define random() random()
#elif defined(_WIN32)
#define random() (rand()%256)
#endif

#include "RAG.h"
#include "buildFinal.h"
#include "filter.h"
#include "image.h"
#include "misc.h"
#include "segment-graph.h"

// random color
rgb random_rgb()
{
    rgb c;
    double r;

    c.r = (uchar)random();
    c.g = (uchar)random();
    c.b = (uchar)random();

    return c;
}

// dissimilarity measure between pixels
static inline float diff(image<float> *r, image<float> *g, image<float> *b,
                         int x1, int y1, int x2, int y2)
{
    return sqrt(square(imRef(r, x1, y1)-imRef(r, x2, y2)) +
                square(imRef(g, x1, y1)-imRef(g, x2, y2)) +
                square(imRef(b, x1, y1)-imRef(b, x2, y2)));
}

/*
Description: transfer the universe into a map and a one-dimensional vector of label
Input:  image<rgb> *im
        universe *univ
Output: map<int, vector<int> > &mapNode: represent each super-pixel with an vector
        vector<int> &vecLabel: an vector with each pixel's label inside
*/
void universeToVector(image<rgb> *im, universe *univ,
                      map<int, vector<int> > &mapNode,
                      vector<int> &vecLabel)

{
    int width = im->width();
    int height = im->height();

    map<int, vector<int> >::iterator iter;
    map<int, int> rootSet;
    int numLabel = 0;

    /*find all the roots*/
    for (int i = 0; i < width * height; i++)
    {
        /*get current node's root*/
        int curRoot = univ->find(i);
        iter = mapNode.find(curRoot);

        /*current root hasn't been found*/
        if (iter == mapNode.end())
        {
            vector<int> child;
            if (curRoot != i)
            {
                child.push_back(i);
            }
            mapNode.insert(pair<int, vector<int> >(curRoot, child));

            rootSet.insert(pair<int, int>(curRoot, numLabel));
            vecLabel.push_back(numLabel);
            numLabel++;
        }
        /*has been found before*/
        else
        {
            if (curRoot != i)
            {
                mapNode[curRoot].push_back(i);
            }

            vecLabel.push_back(rootSet[curRoot]);
        }
    }
    /**/

}

void ragToVector(image<rgb> *im, RAG rag,
                 map<int, int> &vecLabel)
{
    int width = im->width();
    int height = im->height();

    std::map<int, RAGNode> nodes = rag.getNodes();
    std::cout<< nodes.size() << endl;
    for(std::map<int, RAGNode>::iterator it = nodes.begin();
        it != nodes.end(); it++)
    {
        std::map<int, rgb> pixel = it->second.get_pixel();
        for(std::map<int, rgb>::const_iterator pix = pixel.begin();
            pix != pixel.end(); pix++)
        {
            vecLabel[pix->first];
            vecLabel[pix->first] = it->first;
        }
    }
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
 * num_ccs: number of connected components in the segmentation.
 
 * 返回map<int, int> finalLabel: pixelIndex 转换为 labelIndex
 */
//image<rgb> *segment_image(image<rgb> *im, float sigma, float c, int min_size,
//                         int *num_ccs)
std::map<int, int>  segment_image(image<rgb> *im, float sigma, float c, int min_size,
                                 int *num_ccs)

{
    time_t begin,end;
    begin = clock();
    int width = im->width();
    int height = im->height();

    image<float> *r = new image<float>(width, height);
    image<float> *g = new image<float>(width, height);
    image<float> *b = new image<float>(width, height);

    // smooth each color channel
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            imRef(r, x, y) = imRef(im, x, y).r;
            imRef(g, x, y) = imRef(im, x, y).g;
            imRef(b, x, y) = imRef(im, x, y).b;
        }
    }
    image<float> *smooth_r = smooth(r, sigma);
    image<float> *smooth_g = smooth(g, sigma);
    image<float> *smooth_b = smooth(b, sigma);
    delete r;
    delete g;
    delete b;

    // build graph
    edge *edges = new edge[width*height*4];
    int num = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x < width-1) && (im->labels[y * width + x] == im->labels[y * width + x + 1]))
            {
                edges[num].a = y * width + x;
                edges[num].b = y * width + (x+1);
                edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
                num++;
            }

            if ((y < height-1) && (im->labels[y * width + x] == im->labels[(y+1) * width + x]))
            {
                edges[num].a = y * width + x;
                edges[num].b = (y+1) * width + x;
                edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
                num++;
            }

            if ((x < width-1) && (y < height-1) && (im->labels[y * width + x] == im->labels[(y+1) * width + (x+1)]))
            {
                edges[num].a = y * width + x;
                edges[num].b = (y+1) * width + (x+1);
                edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
                num++;
            }

            if ((x < width-1) && (y > 0) && (im->labels[y * width + x] == im->labels[(y-1) * width + (x+1)]))
            {
                edges[num].a = y * width + x;
                edges[num].b = (y-1) * width + (x+1);
                edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
                num++;
            }
        }
    }
    delete smooth_r;
    delete smooth_g;
    delete smooth_b;

    // segment
    universe *u = segment_graph(width*height, num, edges, c);

    // post process small components
    for (int i = 0; i < num; i++)
    {
        int a = u->find(edges[i].a);
        int b = u->find(edges[i].b);
        if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
            u->join(a, b);
    }
    delete [] edges;
    *num_ccs = u->num_sets();
    end = clock();

    std::map<int ,vector<int> > mapNode;
    std::vector<int> vecLabel;
    // Construct RAG
    universeToVector(im, u, mapNode, vecLabel);



    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << elapsed_secs << endl;

    std::map<int, vector<int> > finalNode;
    std::map<int, int> finalLabel;
    begin = clock();

    RAG rag(vecLabel, im);

    universe *final_u = buildFinalLabel(rag, vecLabel, *num_ccs, 5, 100, 1);
    end = clock();
    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << elapsed_secs << endl;
    ragToVector(im, rag, finalLabel);

    
    for(int j = 0; j < height; j++)
    {
        for(int i = 0; i < width; i++)
        {
            cout << finalLabel[j*width + i] << " ";
        }
        cout << endl;
    }

    image<rgb> *output = new image<rgb>(width, height);

    // pick random colors for each component
    rgb *colors = new rgb[width*height];
    for (int i = 0; i < width*height; i++)
        colors[i] = random_rgb();

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int comp = u->find(y * width + x);
            imRef(output, x, y) = colors[comp];
        }
    }

    delete [] colors;
    delete u;
    delete final_u;

    return finalLabel;
}

#endif
