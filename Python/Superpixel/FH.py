#-*- coding: utf-8 -*-
'''
一个没有完成的FH python version

'''


filename='/Users/yuan/Downloads/test.jpg'
from skimage import io, color,filter
from numpy import *
from unionFind import *
from seg_graph import *
from scipy.ndimage import filters
import matplotlib.pyplot as plt





"""
输入
原始图片，SLIC的labelMask
输出
更具体的labelMask，
"""
def FH_Seg(filename,labelMask,minSize,c):
    rgbImg = io.imread(filename)
    width=rgbImg.shape[1]
    height=rgbImg.shape[0]
    labImg = color.rgb2lab(rgbImg)
    smooth_img=filter.gaussian_filter(labImg,sigma=0.8,multichannel=True)
    labelList=unique(labelMask)
    numSeg=len(labelList)
    pixelSet=UnionSet(width*height)
    threshold=[1.0/c]*(width*height)
    for currLabel in labelList:
        edgeList=creatEdgeList(smooth_img,labelMask,currLabel)
        segment_graph(edgeList,pixelSet,threshold,c,minSize)

    """
    现在已经有了pixelSet，接下来把每个labelMask标记成root的index，之后，用uniqueList找到对应的独立
    item，然后再用item的index（0：segNum）代替label中的不规则编号
    """
    segNum=pixelSet.setNum()
    mySegNum=0
    for i in range(width*height):
        if i==pixelSet.find(i):
            pixelSet.setNodeLabel(i,mySegNum)
            mySegNum+=1

    FH_Label=numpy.zeros((height,width))
    for y in range(height):
        for x in range(width):
            index=y*width+x
            FH_Label[y,x]=pixelSet.segLabel(index)

    return FH_Label

img=plt.imread(filename)
labelMask=numpy.zeros((img.shape[0],img.shape[1]))
segments_fz = FH_Seg(filename,labelMask,minSize=100,c=100)
img=plt.imread(filename)
plt.figure()
plt.imshow(plt.mark_boundaries(img, segments_fz))
plt.show()





