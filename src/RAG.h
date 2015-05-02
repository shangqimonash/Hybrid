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
    /*one label maps to one RAGNode*/
    std::map<int, RAGNode> nodeMap;
    /*adjacent list*/
    std::map<int, std::map<int, RAGEdge> > adjList;

// Constructor and Destructor
public:
    RAG(vector<int> &vecLabel, image<rgb> *im);
    ~RAG() {};

// Getter and Setter
public:
    RAGNode& get_node(int ID);
    RAGNode& set_node(int ID);

    const RAGEdge& get_edge(int srcID, int dstID);
    RAGEdge& set_edge(int srcID, int dstID);
    void get_all_edges(std::map<int, std::map<int, RAGEdge> >& res);

// Some public methods and Overrides
public:
//    void AddNode(int ID);
//    void AddPixel(int ID, rgb pixel);
    void AddEdge(int srcID, int dstID);
    void MergeNode(int srcID, int dstID);
    void DelEdge(int srcID, int dstID);
    size_t Nodesize();
    void CalEdgeWeight();
    void CalEdgeWeight(int srcID, int dstID);
    float rgbDistance(rgb a, rgb b);
};

RAG::RAG(vector<int> &vecLabel, image<rgb> *im)
{
    int height = im->height();
    int width = im->width();
    rgb *rgbImage = im->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int curLabel = vecLabel[y * width + x];
            /*consider node*/
            if (nodeMap.find(curLabel) == nodeMap.end())
            {
                /*add a node, position: y * width + x, rgb: im?*/
                //RAGNode newNode(y * width + x, rgbImage[y * width + x]);
                nodeMap[curLabel];
            }
            /*insert this pixel into an existing node*/
            nodeMap[curLabel].addPixel(y * width + x, rgbImage[y * width + x]);

            /*consider edge*/
            if ((x < width-1) && (vecLabel[y * width + x] != vecLabel[y * width + x + 1]))
            {
                //AddEdge should ensure edge added won't be duplicate(both direction)
                AddEdge(curLabel, vecLabel[y * width + x + 1]);
                AddEdge(vecLabel[y * width + x + 1], curLabel);

                RAGEdge &tmpEdge = (adjList[curLabel][vecLabel[y * width + x + 1]]);
                tmpEdge.rgbDistSum += rgbDistance(rgbImage[y * width + x], rgbImage[y * width + x + 1]);
                tmpEdge.rgbDistNum ++;
            }
            else
            {
                RAGNode &tmpNode = nodeMap[curLabel];
                tmpNode.rgbDistSum = rgbDistance(rgbImage[y * width + x], rgbImage[y * width + x + 1]);
                tmpNode.rgbDistNum ++;
            }

            if ((y < height-1) && (vecLabel[y * width + x] != vecLabel[(y+1) * width + x]))
            {
                AddEdge(curLabel, vecLabel[(y+1) * width + x]);
                AddEdge(vecLabel[(y+1) * width + x], curLabel);

                RAGEdge &tmpEdge = (adjList[curLabel][vecLabel[(y+1) * width + x]]);
                tmpEdge.rgbDistSum += rgbDistance(rgbImage[y * width + x], rgbImage[(y+1) * width + x]);
                tmpEdge.rgbDistNum ++;
            }
            else
            {
                RAGNode &tmpNode = nodeMap[curLabel];
                tmpNode.rgbDistSum = rgbDistance(rgbImage[y * width + x], rgbImage[(y+1) * width + x]);
                tmpNode.rgbDistNum ++;
            }

            if ((x < width-1) && (y < height-1) && (vecLabel[y * width + x] != vecLabel[(y+1) * width + (x+1)]))
            {
                AddEdge(curLabel, vecLabel[(y+1) * width + (x+1)]);
                AddEdge(vecLabel[(y+1) * width + (x+1)], curLabel);

                RAGEdge &tmpEdge = (adjList[curLabel][vecLabel[(y+1) * width + (x+1)]]);
                tmpEdge.rgbDistSum += rgbDistance(rgbImage[y * width + x], rgbImage[(y+1) * width + (x+1)]);
                tmpEdge.rgbDistNum ++;
            }
            else
            {
                RAGNode &tmpNode = nodeMap[curLabel];
                tmpNode.rgbDistSum = rgbDistance(rgbImage[y * width + x], rgbImage[(y+1) * width + (x+1)]);
                tmpNode.rgbDistNum ++;
            }

            if ((x < width-1) && (y > 0) && (vecLabel[y * width + x] != vecLabel[(y-1) * width + (x+1)]))
            {
                AddEdge(curLabel, vecLabel[(y-1) * width + (x+1)]);
                AddEdge(vecLabel[(y-1) * width + (x+1)], curLabel);

                RAGEdge &tmpEdge = (adjList[curLabel][vecLabel[(y-1) * width + (x+1)]]);
                tmpEdge.rgbDistSum += rgbDistance(rgbImage[y * width + x], rgbImage[(y-1) * width + (x+1)]);
                tmpEdge.rgbDistNum ++;
            }
            else
            {
                RAGNode &tmpNode = nodeMap[curLabel];
                tmpNode.rgbDistSum = rgbDistance(rgbImage[y * width + x], rgbImage[(y-1) * width + (x+1)]);
                tmpNode.rgbDistNum ++;
            }
        }
    }
}

RAGNode& RAG::get_node(int ID)
{
    if(nodeMap.find(ID) == nodeMap.end())
        throw eInvalNode();
    return nodeMap.at(ID);
}

RAGNode& RAG::set_node(int ID)
{
    if(nodeMap.find(ID) == nodeMap.end())
        throw eInvalNode();
    return nodeMap.at(ID);
}

const RAGEdge& RAG::get_edge(int srcID, int dstID)
{
    if(adjList.find(srcID) == adjList.end())
        return INF_EDGE;
    std::map<int, RAGEdge> temp = adjList[srcID];
    if(temp.find(dstID) == temp.end())
        return INF_EDGE;

    return temp.at(dstID);
}

void RAG::get_all_edges(std::map<int, std::map<int, RAGEdge> >& res)
{
    for(std::map<int, std::map<int, RAGEdge> >::const_iterator it_s = adjList.begin();
            it_s != adjList.end(); it_s++)
    {
        for(std::map<int, RAGEdge>::const_iterator it_d = it_s->second.begin();
            it_d != it_s->second.end(); it_d++)
        {
            if(res.find(it_d->first) != res.end())
            {
                if(res[it_d->first].find(it_s->first) != res[it_d->first].end())
                    continue;
            }
            res[it_s->first][it_d->first] = adjList[it_s->first][it_d->first];
        }
    }
}

//void RAG::AddNode(int ID)
//{
//    if(nodeMap.find(ID) == nodeMap.end())
//        nodeMap[ID];
//}
//
//void RAG::AddPixel(int ID, rgb pixel)
//{
//    if(nodeMap.find(ID) == nodeMap.end())
//        nodeMap[ID];
//    nodeMap[ID].addPixel(pixel);
//}

///无向图，src，dst两个map上面都要添加

void RAG::AddEdge(int srcID, int dstID)
{
    if(adjList.find(srcID) == adjList.end())
    {
        adjList[srcID];
        adjList[srcID][dstID].rgbDistSum = 0;
        adjList[srcID][dstID].rgbDistNum = 0;
    }

    std::map<int, RAGEdge>& temp = adjList.at(srcID);
    if(temp.find(dstID) == temp.end())
    {
        temp[dstID];
        temp[dstID].IsCaled = false;
    }
    return;
}

///edge方面，没有考虑公共edge的变化，没有更定adjList；node方面没有考虑histogram的相加变化
void RAG::MergeNode(int srcID, int dstID)
{
    if(adjList.find(srcID) == adjList.end() || adjList.find(dstID) == adjList.end())
        throw eInvalNode();

    RAGNode &srcNode = nodeMap[srcID];
    RAGNode &dstNode = nodeMap[dstID];

    /*Update node*/
    std::map<int, rgb> &nodePixel = dstNode.get_pixel();
    for (std::map<int, rgb>::iterator i = nodePixel.begin(); i != nodePixel.end(); i++)
    {
        nodeMap[srcID].addPixel((*i).first, (*i).second);
        nodePixel.erase(i);
    }
    srcNode.rgbDistSum += dstNode.rgbDistSum;
    srcNode.rgbDistNum += dstNode.rgbDistNum;


    /*Find share neighbor node*/
    /*update the edge between share neighbor and merged nodes*/
    /*delete the edge no longer used(connected to share neighbor)*/
    std::map<int, RAGEdge>& srcNeighbor = adjList.at(srcID);
    std::map<int, RAGEdge>& dstNeighbor = adjList.at(dstID);
    std::map<int, RAGEdge> shareNeighbor;
    for(std::map<int, RAGEdge>::iterator itSrcNeighbor = srcNeighbor.begin();
        itSrcNeighbor != srcNeighbor.end(); itSrcNeighbor++)
    {
        /*edge between neighbor and src*/
        RAGEdge &srcShareNeighborEdge = (*itSrcNeighbor).second;

        for(std::map<int, RAGEdge>::iterator itDstNeighbor = dstNeighbor.begin();
            itDstNeighbor != dstNeighbor.end(); itDstNeighbor++)
        {
            /*edge between neighbor and dst*/
            RAGEdge &dstShareNeighborEdge = (*itDstNeighbor).second;

            /*delete the edge between dst and its neighbors and add edge between these neighbors and src*/
            adjList[dstID].erase(itDstNeighbor->first);
            adjList[itDstNeighbor->first].erase(dstID);

            /*add edge to src*/
            AddEdge(srcID, itDstNeighbor->first);
            AddEdge(itDstNeighbor->first, srcID);

            /*find a share neighbor*/
            if(itSrcNeighbor->first == itDstNeighbor->first)
            {
//                shareNeighbor[itSrcNeighbor->first] = srcNeighbor[itSrcNeighbor->first] + dstNeighbor[itDstNeighbor->first];
                /*update the edge between the share neighbor and src*/
                srcShareNeighborEdge.rgbDistSum += dstShareNeighborEdge.rgbDistSum;
                srcShareNeighborEdge.rgbDistNum += dstShareNeighborEdge.rgbDistNum;
                /*delete the edge between the share neighbor and dst*/
//                dstNeighbor[itDstNeighbor->first].erase(dstID);
//                adjList.at(itDstNeighbor->first).erase(dstID);
            }

            /*calculate the weight of edge again*/
            CalEdgeWeight(srcID, itDstNeighbor->first);
        }
        /*calculate the weight of edge again*/
        CalEdgeWeight(srcID, itSrcNeighbor->first);
    }

    /*update edge only connected to src*/
    /*update edge only connected to dst*/
    /*delete edge between src and dst*/
}

void RAG::DelEdge(int srcID, int dstID)
{
    if(adjList.find(srcID) == adjList.end())
        return;

    std::map<int, RAGEdge>& temp = adjList.at(srcID);
    if(temp.find(dstID) == temp.end())
        return;
    temp[dstID] = INF_EDGE;
}

float RAG::rgbDistance(rgb a, rgb b)
{
    float res = (a.r - b.r)^2 + (a.g - b.g)^2 + (a.b - b.b)^2;
    res = sqrt(res);

    return res;
}

void RAG::CalEdgeWeight()
{
    std::map<int, std::map<int, RAGEdge> >::iterator i;
    for (i = adjList.begin(); i != adjList.end(); i++)
    {
        std::map<int, RAGEdge>::iterator j;
        for (j = (*i).second.begin(); j != (*i).second.end(); j++)
        {
            if (!(*j).second.IsCaled)
            {
                RAGNode &srcNode = get_node((*i).first);
                RAGNode &dstNode = get_node((*j).first);
                RAGEdge &tmpEdge = adjList[(*i).first][(*j).first];
//                float purityBefore = (srcNode.calEntropy() + dstNode.calEntropy()) / 2;
//                float purityAfter = (srcNode.calFreqEntropy() + dstNode.calFreqEntropy()) / 2;
//                (*j).second.weight = purityAfter - purityBefore;
                tmpEdge.weight = (2*tmpEdge.rgbDistSum/(float)tmpEdge.rgbDistNum)
                                  /(srcNode.rgbDistSum/(float)srcNode.rgbDistNum
                                    + dstNode.rgbDistSum/(float)dstNode.rgbDistNum);
                (*j).second.IsCaled = true;
            }
        }
    }
}

void RAG::CalEdgeWeight(int srcID, int dstID)
{
    RAGNode &srcNode = get_node(srcID);
    RAGNode &dstNode = get_node(dstID);
    RAGEdge &tmpEdge = adjList[srcID][dstID];
//                float purityBefore = (srcNode.calEntropy() + dstNode.calEntropy()) / 2;
//                float purityAfter = (srcNode.calFreqEntropy() + dstNode.calFreqEntropy()) / 2;
//                (*j).second.weight = purityAfter - purityBefore;
    tmpEdge.weight = (2*tmpEdge.rgbDistSum/(float)tmpEdge.rgbDistNum)
                      /(srcNode.rgbDistSum/(float)srcNode.rgbDistNum
                        + dstNode.rgbDistSum/(float)dstNode.rgbDistNum);
}

#endif // RAG_H
