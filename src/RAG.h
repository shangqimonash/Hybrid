#ifndef RAG_H
#define RAG_H

#include <map>

#include "RAGEdge.h"
#include "RAGNode.h"
#include "image.h"

#define INF_EDGE RAGEdge(-10); //Must set up a new magic number to represent the unreachable

class eInvalNode {};

class RAG
{
private:
    std::map<int,RAGNode> nodes;
    std::map<int, std::map<int, RAGEdge> > adjMatrix;

// Constructor and Destructor
public:
    RAG() {};
    ~RAG() {};

// Getter and Setter
public:
    const RAGNode& get_node(int ID);
    RAGNode& set_node(int ID);

    const RAGEdge& get_edge(int srcID, int dstID);
    RAGEdge& set_edge(int srcID, int dstID);

// Some public methods and Overrides
public:
    void AddNode(int ID);
    void AddPixel(int ID, int offset, rgb pixel);
    RAGEdge& AddEdge(int srcID, int dstID);
    void MergeNode(int srcID, int dstID);
    void DelEdge(int srcID, int dstID);
    size_t Nodesize();
};

const RAGNode& RAG::get_node(int ID)
{
    if(nodes.find(ID) == nodes.end())
        throw eInvalNode();
    return nodes.at(ID);
}

RAGNode& RAG::set_node(int ID)
{
    if(nodes.find(ID) == nodes.end())
        throw eInvalNode();
    return nodes.at(ID);
}

const RAGEdge& RAG::get_edge(int srcID, int dstID)
{
    if(adjMatrix.find(srcID) == adjMatrix.end())
        return INF_EDGE;
    std::map<int, RAGEdge> temp = adjMatrix[srcID];
    if(temp.find(dstID) == temp.end())
        return INF_EDGE;

    return temp.at(dstID);
}

void RAG::AddNode(int ID)
{
    if(nodes.find(ID) == nodes.end())
        nodes[ID];
}

void RAG::AddPixel(int ID, int offset, rgb pixel)
{
    if(nodes.find(ID) == nodes.end())
        nodes[ID];
    nodes[ID].addPixel(offset, pixel);
}

RAGEdge& RAG::AddEdge(int srcID, int dstID)
{
    if(adjMatrix.find(srcID) == adjMatrix.end())
        adjMatrix[srcID];

    std::map<int, RAGEdge>& temp = adjMatrix.at(srcID);
    if(temp.find(dstID) == temp.end())
        temp[dstID];
    return temp[dstID];
}

void RAG::MergeNode(int srcID, int dstID)
{
    if(adjMatrix.find(srcID) == adjMatrix.end() || adjMatrix.find(dstID) == adjMatrix.end())
        throw eInvalNode();
    std::map<int, rgb> temp = nodes[dstID].get_pixel();
    while(!temp.empty())
    {
        AddPixel(srcID, temp.begin()->first, temp.begin()->second);
        temp.erase(temp.begin());
    }
    nodes.erase(dstID);
}

void RAG::DelEdge(int srcID, int dstID)
{
    if(adjMatrix.find(srcID) == adjMatrix.end())
        return;

    std::map<int, RAGEdge>& temp = adjMatrix.at(srcID);
    if(temp.find(dstID) == temp.end())
        return;
    temp[dstID] = INF_EDGE;
}

#endif // RAG_H
