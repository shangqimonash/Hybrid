//
//  heapEdge.h
//  Hybrid-Master
//
//  Created by Yuan on 28/4/15.
//  Copyright (c) 2015 Yuan. All rights reserved.
//

#ifndef Hybrid_Master_heapEdge_h
#define Hybrid_Master_heapEdge_h
/*
 the edge class only used in heap during merging process
 */

class HeapEdge
{
public:
    int src;
    int dst;
    double weight;
    HeapEdge(){
        src=0;
        dst=0;
        weight=0
    }
    HeapEdge(int src,int dst,double weight){
        this->src=src;
        this->dst=dst;
        this->weight=weight
    }
    bool operator >(HeapEdge & e){
       return this->weight<e.weight;
    }
    bool operator <(HeapEdge & e){
        return this->weight>e.weight;
    }
};

#endif
