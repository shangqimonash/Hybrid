#ifndef RAG_H
#define RAG_H

#include <map>

#include "RAGEdge.h"
#include "RAGNode.h"
#include "image.h"

class RAG
{
private:
    std::map<int,RAGNode> nodes;
    std::map<int, std::map<int, RAGEdge> > adjMatrix;

// Constructor and Destructor
public:
    RAG(){};
    ~RAG(){};

// Getter and Setter
public:
    const RAGNode& get_node(int ID);
    RAGNode& set_node(int ID);

    const RAGEdge& get_edge(int srcID, int dstID);
    RAGEdge& set_edge(int srcID, int dstID);

// Some public methods and Overrides
public:
    void AddNode(int ID, rgb pixel);
    void AddEdge(int srcID, int dstID);
    void MergeNode(int srcID, int dstID);
    void DelEdge(int srcID, int dstID);
};


void RAG::AddNode(int ID, rgb pixel)
{
    if(nodes.find(ID) == nodes.end())
        nodes[ID];
    nodes[ID].addPixel(pixel);
}

void RAG::AddEdge(int srcID, int dstID)
{

}

void RAG::MergeNode(int srcID, int dstID)
{

}

void RAG::DelEdge(int srcID, int dstID)
{

}

#endif // RAG_H
