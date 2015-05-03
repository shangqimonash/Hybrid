//
//  buildFinal.h
//  Hybrid-Master
//
//  Created by Yuan on 28/4/15.
//  Copyright (c) 2015 Yuan. All rights reserved.
//

#ifndef Hybrid_Master_buildFinal_h
#define Hybrid_Master_buildFinal_h
#include "dd_head.h"
#include "RAG.h"
#include "disjoint-set.h"
#include "heapEdge.h"
#include <vector>
#include <algorithm>

/*
 input:
    g: the RAG class store the second order superpxiel
    fhLabel: the label matrix of 2nd order superpixel
    fhNum: the number of 2nd order superpixel
    binNum: the bin number of each RGB chanel histogram
    outputSPnum: the used-defined superpxiel number
    beta: the weight parameter combining the boundary term and region term

 output:
    universe: the disjoint set. we need to call another function to convert it into vector
 */

universe* buildFinalLabel(RAG & g,std::vector<int> &fhLabel, int fhNum, int binNum,int outputSPnum,double beta)
{

    std::vector<HeapEdge> edgeArray;
    dd_dist diffusionDist;

    //get all edges in RAG to construct heap
    std::map<int, std::map<int, RAGEdge> > adjList;
    g.get_all_edges(adjList);


    //convert the adjacent list in RAG class to edge vector
    //edgeWeight=boundaryTerm*regionTerm
    std::map<int, std::map<int,RAGEdge> >::iterator i;
    for (i = adjList.begin(); i != adjList.end(); i++)
    {
        std::map<int, RAGEdge>::iterator j;
        for (j = (*i).second.begin(); j != (*i).second.end(); j++)
        {
            int src=(*i).first;
            int dst=(*j).first;
            float boundaryTerm=(*j).second.weight;
            RAGNode srcNode = g.get_node((*i).first);
            RAGNode dstNode = g.get_node((*j).first);
            srcNode.calculateHist(binNum);
            dstNode.calculateHist(binNum);
            double * h1=srcNode.getHist();
            double * h2=dstNode.getHist();

            // call the new region term calculated by histogram diffusion distance
            double regionTerm=diffusionDist.dd3D(h1,h2,binNum,binNum,binNum);
            //double weight=regionTerm+beta*boundaryTerm;
            double weight=regionTerm*boundaryTerm;
            HeapEdge heapEdge(src,dst,weight);
            edgeArray.push_back(heapEdge);

        }
    }

    // change the edge vector to heap
    std::make_heap(edgeArray.begin(),edgeArray.end());

    // make the disjoint set to record the merge result, which is also the ouput of this function
    universe *u = new universe(fhNum);

    //the superpixel number before merging
    int spNum=fhNum;

    //merge the node until the superpixel number is same the user-defined one
    while (spNum>outputSPnum)
    {

        int src_org=edgeArray[0].src;
        int dst_org=edgeArray[0].dst;
        // during merging, the root node can be changed. find the root node
        int src=u->find(src_org);
        int dst=u->find(dst_org);

        // two node already merged
        if (src==dst)
        {
            std::pop_heap (edgeArray.begin(),edgeArray.end());
            edgeArray.pop_back();
        }
        else
        {
            //calculate the new weight
            g.get_node(src).calculateHist(binNum);
            g.get_node(dst).calculateHist(binNum);
            double * h1=g.get_node(src).getHist();
            double * h2=g.get_node(dst).getHist();
            // call the new region term calculated by histogram diffusion distance
            double regionTerm=diffusionDist.dd3D(h1,h2,binNum,binNum,binNum);
            double boundaryTerm=g.get_edge(src,dst).weight;
            //this is another form of the weight, the beta parameter need tuing
            //double currWeight=regionTerm+beta*boundaryTerm;
            double currWeight=regionTerm*boundaryTerm;

            // the merging affect the edge weight in RAG，heap need to be updated
            if (currWeight!=edgeArray[0].weight)
            {
                edgeArray[0].weight=currWeight;
                std::make_heap(edgeArray.begin(),edgeArray.end());
            }
            // top edge is not affect by the merging process done before, we can safely merge this two node
            else
            {
                u->join(src,dst);
                if(u->find(src) == src)
                    g.MergeNode(src,dst);
                else
                    g.MergeNode(dst, src);
                spNum--;
                std::pop_heap (edgeArray.begin(),edgeArray.end());
                edgeArray.pop_back();
            }
        }
    }
    return u;
}


#endif
