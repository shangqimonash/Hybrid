# -*- coding: utf-8 -*-

#根据label，求出index出对应的image的pixel
import numpy as np
from scipy.ndimage import filters
from scipy import ndimage as nd
import math
from skimage import draw, measure, segmentation, util, color

img=np.arange(36).reshape(6,6)
img.astype(float)
for y in range(img.shape[0]):
    for x in range(img.shape[1]):
        if img[y][x]>1 and img[y][x]<16:
            img[y][x]=100

fp = nd.generate_binary_structure(img.ndim, connectivity=2)
for d in range(fp.ndim):
    fp = fp.swapaxes(0, d)
    fp[0, ...] = 0
    fp = fp.swapaxes(0, d)
    # For example
    # if labels.ndim = 2 and connectivity = 1
    # fp = [[0,0,0],
    #       [0,1,1],
    #       [0,1,0]]
    #
    # if labels.ndim = 2 and connectivity = 2
    # fp = [[0,0,0],
    #       [0,1,1],
    #       [0,1,1]]
def fourTime(value):
    for index in range(len(value)):
        if value[index]!=100:
            return value[0]
    sum=0
    for index in range(len(value)):
        value[index]*=4
        sum+=value[index]
    return sum

def buildRAG(values, graph,slic_seg,fh_seg):
    """Create edge in `g` between the first element of `values` and the rest.

    Add an edge between the first element in `values` and
    all other elements of `values` in the graph `g`. `values[0]`
    is expected to be the central value of the footprint used.

    Parameters
    ----------
    values : array, 是FH_seg的那个label matrix
        The array to process.
    graph : RAG
        The graph to add edges in.
    slic_seg:是slic分出来的label matrix
    fh_seg: 是fh进一步分出来的label matrix
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



#一定要有一个返回值啊原来，他不会修改之前img的内容哦
out_img=filters.generic_filter(
    img,
    function=fourTime,
    footprint=fp,
    mode='nearest'
    )
print "处理之后"
print out_img
'''
下面是对np.where()的练习
'''
print "下面是对np.where()的练习"
label=np.arange(9).reshape(3,3)
#python全部是reference，所以你这样是不能新建一个object img的，仅仅是吧img指向了label，修改label，
#img也跟着修改了
#img=label
img=np.arange(9).reshape(3,3)
for x in range(3):
    label[0][x]=0
label[2][0]=0
print label
index=np.where(label==0)
#这里直接把img[index]是一个1D array，输入到histogram的函数中，就可以得到bin了
print img[index]
sum=0
for pixel in img[index]:
    sum=sum+pixel
print "pixel sum is 0+1+2+6 "
print sum


