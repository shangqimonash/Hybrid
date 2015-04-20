#-*- coding: utf-8 -*-
from edge import *
from imageProcessing import *

"""
输入
img是一个numpy的3-D array
labelMask是一个2-D array
currLabel是当前扫描的label区域

输出
edgeList
"""
def creatEdgeList(img,labelMask,currLabel):
    height=img.shape[0]
    width=img.shape[1]
    edgeList=[]
    #已经遍历了所有currSeg之后，就跳出，加快速度
    currSegSize=count_nonzero(labelMask==currLabel)
    #currSegSize=sum([row.count(currLabel) for row in labelMask ])
    countPixel=0
    for y in range(height):
        #首先保证我当前seg没有遍历完，否则直接退
        if countPixel>currSegSize:
            break

        for x in range(width):
            if labelMask[y][x]==currLabel:
                countPixel=countPixel+1

                #有向右edge
                if(x<width-1):
                    if labelMask[y][x+1]==currLabel:
                        src=y*width+x;
                        des=y*width+x+1;
                        weight=vectorDif(img[y,x,:],img[y,x+1,:])
                        edgeList.append(myEdge(src,des,weight))

                #有向下的edge
                if y<height-1:
                    if labelMask[y+1][x]==currLabel:
                        src=y*width+x;
                        des=(y+1)*width+x;
                        weight=vectorDif(img[y,x,:],img[y+1,x,:])
                        edgeList.append(myEdge(src,des,weight))

                #有右下的edge
                if y<height-1 and x<width-1:
                    if labelMask[y+1][x+1]==currLabel:
                        src=y*width+x
                        des=(y+1)*width+(x+1)
                        weight=vectorDif(img[y,x,:],img[y+1,x+1,:])
                        edgeList.append(myEdge(src,des,weight))
                #有右上edge
                if y>0 and x<width-1:
                    if labelMask[y-1][x+1]==currLabel:
                        src=y*width+x
                        des=(y-1)*width+(x+1)
                        weight=vectorDif(img[y,x,:],img[y,x,:])
                        edgeList.append(myEdge(src,des,weight))

    return edgeList






"""
下面是通过传入的label构建edgeList
"""




"""
似乎这里传入object，你在函数里面改变了pixelSet和thresholList之后，在外面就可以
看到，所以不用返回值

你在function 里面创建的object，如果可以return 也可以返回到外面，
"""
def segment_graph(edgeList,pixelSet,thresholdList,c,min_seg_size):
    sorted(edgeList)
    for edge in edgeList:
        srcSet=pixelSet.find(edge.src)
        desSet=pixelSet.find(edge.dest)
        if srcSet!=desSet:
            if edge.weight<=thresholdList[srcSet] and edge.weight<=thresholdList[desSet]:
                pixelSet.union(srcSet,desSet)
                newSet=pixelSet.find(srcSet)
                newSetSize=pixelSet.size(newSet)
                thresholdList[newSet]=edge.weight+THRESHOLD(newSetSize,c)

    # 对于小于min_seg_size的进行合并
    for edge in edgeList:
        src=pixelSet.find(edge.srt)
        des=pixelSet.find(edge.dest)
        if src!=des and (pixelSet.size(src)<min_seg_size or pixelSet.size(des)<min_seg_size):
            pixelSet.join(src,des)




def THRESHOLD(size,c):
    return 1.0*size/c


