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


/**
 the update of rank and size is not correct. we should update the size of root node, but in this function, there is not find procedure in the join function.
 
 probably the in the main program, the author doen't even use the size()function
 
 */


#ifndef DISJOINT_SET
#define DISJOINT_SET

// disjoint-set forests using union-by-rank and path compression (sort of).

typedef struct {
    int rank;//the height of the set tree. We only record the rank of root node
    int p;// point to the index of the parent node
    int size;//the size of each particular set
} uni_elt;

class universe {
public:
    universe(int elements);
    ~universe();
    int find(int x);
    void join(int x, int y);
    int size(int x) const { return elts[x].size; }
    int num_sets() const { return num; }
    
private:
    uni_elt *elts;
    int num;//the number of different set
};

universe::universe(int elements) {
    elts = new uni_elt[elements];
    num = elements;
    for (int i = 0; i < elements; i++) {
        elts[i].rank = 0;
        elts[i].size = 1;
        elts[i].p = i;
    }
}

universe::~universe() {
    delete [] elts;
}

//return the root of set tree
int universe::find(int x) {
    int y = x;
    //find the root element of set in which the element x reside
    while (y != elts[y].p)
        y = elts[y].p;
    //point the elment which is found to the root of his set
    elts[x].p = y;
    return y;
}

/**
 the update of rank and size is not correct. we should update the size of root node, but in this function, there is not find procedure in the join function.
 
 probably the in the main program, the author doen't even use the size()function
 
 In the program, the author first use find() function to find the root and then call this join function. In this way, the 2 input index is already the root of set tree, thus the update of variable size will be reasonable.
 
 
 every new node is directly attached to the root node. In general the height(rank) is always 1 (init with 0), only when two set tree have the same rank, the rank of the tree can be increase by 1. Otherwise the rank remains the same.
 
 */
void universe::join(int x, int y) {
    if (elts[x].rank > elts[y].rank) {
        elts[y].p = x;
        elts[x].size += elts[y].size;
    } else {
        elts[x].p = y;
        elts[y].size += elts[x].size;
        if (elts[x].rank == elts[y].rank)
            elts[y].rank++;
    }
    num--;
}

#endif
