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


universe* buildFinalLabel(RAG & g,std::vector<int> fhLabel, int fhNum, int binNum,int outputSPnum,double beta){
    std::vector<HeapEdge> edgeArray;
    dd_dist diffusionDist;
    std::map<int, std::map<int, RAGEdge> > adjList=g.allEdges();
    
    std::map<int, std::map<int,RAGEdge>>::iterator i;
    
    //convert the adjacent list to edge vector
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
            double * h1=srcNode.getHist();
            double * h2=dstNode.getHist();
            double regionTerm=diffusionDist.dd3D(h1,h2,binNum,binNum,binNum);
            double weight=regionTerm+beta*boundaryTerm;
            HeapEdge heapEdge(src,dst,weight)
            edgeArray.push_back(heapEdge);
            
        }
    }
    
    std::make_heap(edgeArray.begin(),edgeArray.end());
    
    // make the disjoint set which is the ouput
    universe *u = new universe(fhNum);
    
    int spNum=fhNum;
    while (spNum<outputSPnum) {
        int src_org=edgeArray[0].src;
        int dst_org=edgeArray[0].dst;
        int src=u->find(src_org);
        int dst=u->find(dst_org);
        if (src==dst) {
            std::pop_heap (edgeArray.begin(),edgeArray.end());
            edgeArray.pop_back();
        }
        else{
            double * h1=g.get_node(src).getHist();
            double * h2=g.get_node(src).getHist();
            double regionTerm=diffusionDist.dd3D(h1,h2,binNum,binNum,binNum);
            double boundaryTerm=g.get_edge(src,dst).weight;
            double currWeight=regionTerm+beta*boundaryTerm;
            if (currWeight!=edgeArray[0].weight) {
                edgeArray[0].weight=currWeight;
                std::make_heap(edgeArray.begin(),edgeArray.end());
            }
            else{
                g.MergeNode(src,dst);
                u.join(src,dst);
                std::pop_heap (edgeArray.begin(),edgeArray.end());
                edgeArray.pop_back();

            }
        }
    }
    return u;
}


#endif
