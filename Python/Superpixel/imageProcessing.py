
# -*- coding: utf-8 -*-
import numpy
filename='/Users/yuan/Downloads/lena_org.jpg'
from skimage import io, color,filter
from numpy import *
from scipy.ndimage import filters
import matplotlib.pyplot as plt


"""
输入是一个3-item array，求两个array差的2-norm
"""
def vectorDif(pixelA,pixelB):
    return numpy.linalg.norm(pixelA-pixelB)



filename='/Users/yuan/Downloads/lena_org.jpg'
from skimage import io, color,filter
from numpy import *
from scipy.ndimage import filters
import matplotlib.pyplot as plt



'''
读取图片并且转换为lab的array， 读进来之后，rgb是(518, 242, 3)，第一维就是图片的
行数，518就是row，242就是column,这个图片是一个竖着者长条状的图片

'''

rgb = io.imread(filename)
print (type (rgb))
lab = color.rgb2lab(rgb)
print (type(lab))
print lab.shape
l,a,b=color.rgb2lab(rgb)
print l.shape,a.shape,b.shape
print(type(l))
filterLAB=filter.gaussian_filter(lab,sigma=2,multichannel=True)
filterRGB=filter.gaussian_filter(rgb,sigma=2,multichannel=True)

plt.figure()
plt.imshow(rgb)
plt.figure()
plt.imshow(lab)
plt.figure()
plt.imshow(filterRGB)
plt.figure()
plt.imshow(filterLAB)



edgeX=zeros(lab.shape)
edgeY=zeros(lab.shape)

for i in range(3):
    filters.sobel(lab[:,:,i],1,edgeX[:,:,i])
    filters.sobel(lab[:,:,i],0,edgeY[:,:,i])
edge=numpy.sqrt(edgeX**2+edgeY**2)

io.imshow(lab)
io.show()

