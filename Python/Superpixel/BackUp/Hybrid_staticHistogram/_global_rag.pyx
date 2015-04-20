# -*- coding: utf-8 -*-

'''
用如果不用slic 把region 隔开，最后merge的graph效果很不好，所以还是不用这个吧


'''
import rag_graph
cimport numpy as np
import numpy as np
import copy
from datetime import datetime
from math import sqrt
import math
import cython
from scipy.ndimage import filters
from scipy import ndimage as nd
import math
from skimage import draw, measure, segmentation, util, color


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

def _add_edge_filter(values, graph):
    """Create edge in `g` between the first element of `values` and the rest.

    Add an edge between the first element in `values` and
    all other elements of `values` in the graph `g`. `values[0]`
    is expected to be the central value of the footprint used.

    Parameters
    ----------
    values : array
        The array to process.
    graph : RAG
        The graph to add edges in.

    Returns
    -------
    0 : int
        Always returns 0. The return value is required so that `generic_filter`
        can put it in the output array.

    """
    values = values.astype(int)
    current = values[0]
    for value in values[1:]:
        #就是在这里判断，如果mask上面的label时相同的，就不要添加new edge 进去
        if value != current:
            #graph这个数据结构在add edge的时候，就自动添加了node了，
            # 这个时候就已经有了graph.node[current]和graph.node[value]
            graph.add_edge(current, value)

    return 0


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
def construct_rag(np.ndarray[np.uint8_t, ndim=3] img,
                       np.ndarray[int, ndim=2, mode="c"] slic_seg not None,
                        np.ndarray[int, ndim=2, mode="c"] fh_seg not None,
                        int slicNum,
                        int binNum):
    cdef int height, width, y, x, k, slicLabel,pixelCount,MaxslicSize,minSlicSize,done,
    cdef int current, next,i,regionLength
    height = img.shape[0]
    width = img.shape[1]
    MaxslicSize=height*width/slicNum*3
    minSlicSize=MaxslicSize/9
    cdef int minslicWidth,slicPerRow
    minslicWidth=math.ceil(sqrt(minSlicSize))
    slicPerRow=width/minslicWidth
    cdef unsigned int * xCoor
    cdef unsigned int * yCoor



    #用来作为中间变量，方便求histogram
    cdef np.ndarray l_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    cdef np.ndarray a_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    cdef np.ndarray b_chanel=np.zeros([<unsigned int>height,<unsigned int>width],dtype=int)
    histogram=np.zeros(binNum**3,dtype=int)
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

    fp = nd.generate_binary_structure(fh_seg.ndim, connectivity=2)
    for d in range(fp.ndim):
        fp = fp.swapaxes(0, d)
        fp[0, ...] = 0
        fp = fp.swapaxes(0, d)
    fp[0,2]=True
    #就是建了一个右，下，右上，右下这样的一个mask，和你之前cpp中建edge的浏览顺序有一样
    '''
    fp=array([[False, False,  True],
       [False,  True,  True],
       [False,  True,  True]], dtype=bool)
    '''
    filters.generic_filter(
    fh_seg,
    function=_add_edge_filter,
    footprint=fp,
    mode='nearest',
    extra_arguments=(g,))
    for n in g:
        #node的属性里面'label’是一个list，也就是当merge之后，这个list里面会含有多个iterm
        g.node[n].update({'mergeList': [n],
                          'hist': copy.copy(histogram),
                          'pixelCount': 0})




    #实现是把graph建立起来，slic region之间不会有edge，每个node有hist，pixelCount,
    # mergeList三个attribute，之后第二次遍历，再把pixelCount和histogram更新

    '''
    y=0
    while  y<height:
        x=0
        while x<width :


            current=fh_seg[<unsigned int>y,<unsigned int>x]
            g.add_node(current,
                       pixelCount=0,
                       hist=copy.copy(histogram),
                       mergeList=[])

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


    '''
    #更新rag的node attribute，重新遍历一边image
    time1=datetime.now()
    slicLabel=0
    lCutoff=np.histogram(l_chanel,bins=binNum)[1]
    aCutoff=np.histogram(a_chanel ,bins=binNum)[1]
    bCutoff=np.histogram(b_chanel ,bins=binNum)[1]


    time3=datetime.now()
    #开始进行遍历,更新graph的node attribute
    #当slicLabel比较大的时候，从下面y就不从0开始找
    y=0
    while   y<height:
        x=0
        while  x<width:


            fhLabel=fh_seg[y,x]

            bin=histBin(lCutoff,aCutoff,bCutoff,
                        l_chanel[y,x],a_chanel[y,x],
                        b_chanel[y,x],binNum)
            g.node[fhLabel]['hist'][bin]+=1

            g.node[fhLabel]['pixelCount']+=1
            pixelCount+=1


            x=<unsigned int>(x+1)

        y=<unsigned int>(y+1)

    time2=datetime.now()
    print("second part time")
    #print((time2-time1).total_seconds())

    return g
