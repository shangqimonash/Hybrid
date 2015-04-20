# -*- coding: utf-8 -*-
import rag_graph
cimport numpy as np
import numpy as np
import copy
from datetime import datetime
from math import sqrt
import math
import cython



"""
输入当前pixel的lab值，和lab的cutoff value，
输出当前pixel应该被分到histogram哪个bin里面
"""
@cython.boundscheck(False)
def histBin(np.ndarray[np.double_t, ndim=1] cutoff_l,
            np.ndarray[np.double_t, ndim=1] cutoff_a,
            np.ndarray[np.double_t, ndim=1] cutoff_b,
            l,a,b, int binNum):

    cdef int i
    cdef int bin1=-1
    i=0
    while l>cutoff_l[i]:
        bin1+=1
        i+=1
    if bin1==-1:
        bin1=0

    cdef int bin2=-1
    i=0
    while l>cutoff_l[i]:
        bin2+=1
        i+=1
    if bin2==-1:
        bin1=0
    cdef int bin3=-1
    i=0
    while l>cutoff_l[i]:
        bin3+=1
        i+=1
    if bin3==-1:
        bin3=0

    return bin1+binNum*bin2+binNum*binNum*bin3



'''
输入：
slic_seg:       2D slic label matrix
fh_seg:         2D fh label matrix
img:            3D img, containing lab chanel
                也就是这里传进来的img是经过 img = color.rgb2lab(img)
                转换之后的img，直接就是lab的值，<type 'numpy.ndarray'>，
                shape(481, 321, 3)，而且这里cython函数可以直接传入ndarray
                所以也不用担心转换问题
currSlicLabel:  当前的slic region的label
'''

@cython.boundscheck(False)
def construct_rag(np.ndarray[double, ndim=3] img,
                       np.ndarray[int, ndim=2, mode="c"] slic_seg not None,
                        np.ndarray[int, ndim=2, mode="c"] fh_seg not None,
                        int slicNum,
                        int binNum):
    cdef int height, width, y, x, k, slicLabel,pixelCount,MaxslicSize,minSlicSize,done,
    cdef int current, next,i,regionLength
    height = img.shape[0]
    width = img.shape[1]
    MaxslicSize=height*width/slicNum*5
    minSlicSize=MaxslicSize/9
    cdef int minslicWidth,slicPerRow
    minslicWidth=math.ceil(sqrt(minSlicSize))
    slicPerRow=width/minslicWidth

    cdef np.ndarray[np.int_t, ndim=1] yList
    cdef np.ndarray[np.int_t, ndim=1] xList
    cdef unsigned int * xCoor
    cdef unsigned int * yCoor



    #用来作为中间变量，方便求histogram
    cdef np.ndarray l_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    cdef np.ndarray a_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    cdef np.ndarray b_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    histogram=np.zeros(binNum**3,dtype=int)
    freqHistogram=np.zeros(binNum**3,dtype=float)
    y=0
    while y<height:
        x=0
        while x<width:
            l_chanel[<unsigned int>y,<unsigned int>x]=img[<unsigned int>y,<unsigned int>x,0]
            a_chanel[<unsigned int>y,x]=img[<unsigned int>y,x,1]
            b_chanel[<unsigned int>y,x]=img[<unsigned int>y,x,2]
            x+=1
        y+=1


    g = rag_graph.RAG()
    #实现是把graph建立起来，slic region之间不会有edge，每个node有hist，pixelCount,
    # mergeList三个attribute，之后第二次遍历，再把pixelCount和histogram更新

    slicLabel=0
    while slicLabel<slicNum:
        pixelCount=0
        done=0
        y=0
        while  y<height:
            x=0
            while x<width :

                if slic_seg[<unsigned int>y,<unsigned int>x]==slicLabel:

                    current=fh_seg[<unsigned int>y,<unsigned int>x]
                    list=[current]
                    g.add_node(current,
                               pixelCount=0,
                               hist=copy.copy(histogram),
                               freqHist=copy.copy(histogram),
                               mergeList=list)

                    if x<width-1:
                        if slic_seg[y,x+1]==slicLabel:
                            next=fh_seg[y,x+1]
                            current=fh_seg[y,x]
                            if current!=next:
                                g.add_edge(current,next)


                    if y<height-1:
                        if slic_seg[y+1,x]==slicLabel:
                            next=fh_seg[y+1,x]
                            current=fh_seg[y,x]
                            if current!=next:
                                g.add_edge(current,next)


                    if x<width-1 and y<height-1:
                        if slic_seg[y+1,x+1]==slicLabel:
                            next=fh_seg[y+1,x+1]
                            current=fh_seg[y,x]
                            if current!=next:
                                g.add_edge(current,next)


                    if x<width-1 and y>0:
                        if slic_seg[y-1,x+1]==slicLabel:
                            next=fh_seg[y-1,x+1]
                            current=fh_seg[y,x]
                            if current!=next:
                                g.add_edge(current,next)

                    pixelCount+=1
                    if pixelCount>MaxslicSize:
                        done=1


                x=<unsigned int>(x+1)
            y=<unsigned int>(y+1)

        slicLabel+=1

    #更新rag的node attribute，重新遍历一边image
    time1=datetime.now()
    slicLabel=0

    #Compute the histogram of a set of data.后面去[1]，因为会返回两个item，第2个是
    # histogram的cutoffValue，也就是你bin分割的那几个值是多少
    lCutoff=np.histogram(l_chanel,bins=binNum)[1]
    aCutoff=np.histogram(a_chanel ,bins=binNum)[1]
    bCutoff=np.histogram(b_chanel ,bins=binNum)[1]
    while slicLabel<slicNum:
        #找到slicLabel 为1的所有pixel的index
        regionIndex=np.where(slic_seg==slicLabel)
        regionRchanel=l_chanel[regionIndex]
        regionGchanel=a_chanel[regionIndex]
        regionBchanel=b_chanel[regionIndex]

        regionRcutoff=np.histogram(regionRchanel,bins=binNum)[1]
        regionGcutoff=np.histogram(regionGchanel,bins=binNum)[1]
        regionBcutoff=np.histogram(regionBchanel,bins=binNum)[1]


        time3=datetime.now()
        #开始进行遍历,更新graph的node attribute
        yList=regionIndex[0]
        xList=regionIndex[1]
        xCoor=<unsigned int *>xList.data
        yCoor=<unsigned int *>yList.data
        i=0
        regionLength=yList.size

        #在一个slic region内，有很多fh region
        while i <regionLength:
            y=yList[i]
            x=xList[i]
            fhLabel=fh_seg[y,x]

            #这个是全图的cutoff value

            bin=histBin(lCutoff,aCutoff,bCutoff,
                                l_chanel[y,x],a_chanel[y,x],
                                b_chanel[y,x],binNum)
            '''
            bin=histBin(regionRcutoff,regionGcutoff,regionBcutoff,
                                l_chanel[y,x],a_chanel[y,x],
                                b_chanel[y,x],binNum)
            '''
            g.node[fhLabel]['hist'][bin]+=1

            g.node[fhLabel]['pixelCount']+=1

            i=<unsigned int>(1+i)

        time4=datetime.now()
        #print((time4-time3).total_seconds())







        '''
        #当slicLabel比较大的时候，从下面y就不从0开始找
        pixelCount=0
        done=0
        y=slicNum/slicPerRow*minslicWidth
        #y=0
        while  done!=1 and y<height:
            x=0
            while done!=1 and  x<width:
                if done!=1 and slic_seg[y,x]==slicLabel:

                    fhLabel=fh_seg[y,x]

                    bin=histBin(lCutoff,aCutoff,bCutoff,
                                l_chanel[y,x],a_chanel[y,x],
                                b_chanel[y,x],binNum)
                    g.node[fhLabel]['hist'][bin]+=1

                    g.node[fhLabel]['pixelCount']+=1
                    pixelCount+=1
                    if pixelCount>MaxslicSize:
                        done=1

                x=<unsigned int>(x+1)

            y=<unsigned int>(y+1)
        '''





        slicLabel+=1
    time2=datetime.now()
    print("second part time")
    print((time2-time1).total_seconds())

    return g
