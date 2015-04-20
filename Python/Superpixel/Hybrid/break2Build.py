# -*- coding: utf-8 -*-
import heapq
from scipy.stats import entropy
import numpy as np
import networkx as nx

'''
找到现在src_org这个region是属于那个大region的。原理和union-set中找root node
是一样的
'''
def getMapIndex(src_org,map_array):
    src=src_org
    while map_array[src]!=src:
        src=map_array[src]
    map_array[src_org]=src
    return src

'''
输入参数就是rag那个graph
'''
def buildFinalLabel(g,region_fh,fhNum,N):
    heap_edgeList=[]
    for edge in g.edges():
        src=edge[0]
        dst=edge[1]

        #下面是用histogram value，被注释掉落，我试一试用frequency

        purity_before=(entropy(g.node[src]['hist'])+entropy(g.node[dst]['hist']))/2.0
        #选用百分比的相加，是为了防止，value的相加，导致那个region size小的部分，没有办法起
        #太多的作用
        hist_after=(1.0*g.node[src]['hist']/g.node[src]['pixelCount']+\
                   1.0*g.node[dst]['hist']/g.node[dst]['pixelCount'])/2.0
        purity_after=entropy(hist_after)
        '''

        purity_before=(entropy(g.node[src]['freqHist'])+entropy(g.node[dst]['freqHist']))/2.0
        hist_after=(1.0*g.node[src]['freqHist']+\
                   1.0*g.node[dst]['freqHist'])/2.0
        purity_after=entropy(hist_after)
        '''


        loss=purity_after-purity_before
        edge_info=[]
        edge_info.append(loss)
        edge_info.append(src)
        edge_info.append(dst)
        heap_edgeList.append(edge_info)

    heapq.heapify(heap_edgeList)
    superPixelNum=fhNum
    map_arrary=np.arange(fhNum)
    while superPixelNum>N and len(heap_edgeList)>0:
        src_org=heap_edgeList[0][1]
        dst_org=heap_edgeList[0][2]
        #找到现在heap top两个链接的node是否在一个set中
        src=getMapIndex(src_org,map_arrary)
        dst=getMapIndex(dst_org,map_arrary)
        if  src==dst:
            heapq.heappop(heap_edgeList)
        else:
            #print(src)
            #print(src_org)
            #print(g.node[src]['hist'])

            a=entropy(g.node[src]['hist'])
            b=entropy(g.node[dst]['hist'])
            purity_before=(a+b)/2.0
            hist_after=1.0*g.node[src]['hist']/g.node[src]['pixelCount']+\
                       1.0*g.node[dst]['hist']/g.node[dst]['pixelCount']
            purity_after=entropy(hist_after)
            '''
            a=entropy(g.node[src]['freqHist'])
            b=entropy(g.node[dst]['freqHist'])
            purity_before=(a+b)/2.0
            hist_after=1.0*g.node[src]['freqHist']+\
                       1.0*g.node[dst]['freqHist']
            purity_after=entropy(hist_after)
            '''
            loss=purity_after-purity_before



            if loss!=heap_edgeList[0][0]:
                heap_edgeList[0][0]=loss
                heapq.heapify(heap_edgeList)
            else:
                g.merge_nodes(src,dst)
                superPixelNum-=1
                heapq.heappop(heap_edgeList)
                #src和dst merge之后，union set中，是src指向dst
                while map_arrary[dst]!=dst:
                    dst=map_arrary[dst]

                map_arrary[src]=dst

    #map_arrary[]有bug，最后输出的map不完全，但是可以勉强完成merge的算法，但是完成不了map的算法，
    #所以下面又重新根据mergeList写了一个新的map算法。因为1和2融合了，1指向2，之后2和3融合了，2指
    #向3，这个时候1还是指向2，没有更新，就会出现问题。其实再次对这个map_array[]进行整理也可以实现



    # We construct an array which can map old labels to the new ones.
    # All the labels within a connected component are assigned to a single
    # label in the output.

    map_array = np.arange(fhNum, dtype=int)
    for i,node in enumerate(g.nodes()):
        #finalList=g.node[node]['mergeList']
        #print(finalList)
        #finalLabel=finalList[0]
        #print(g.node[node]['mergeList'])
        for label in g.node[node]['mergeList']:
            map_array[label] = node


    return map_array[region_fh],map_array,g




