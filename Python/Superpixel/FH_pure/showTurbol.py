# -*- coding: utf-8 -*-
'''
显示自己turbol处理后的那个boundary，mat文件在benchmark那个文件夹中
'''
from __future__ import print_function

import matplotlib.pyplot as plt
import numpy
from skimage import io, color,filter
from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float
import scipy.io as sio
import PIL.Image

alg='fh'
file='101087'
path1='/Users/yuan/inputSuperpixel/BSR/BSDS500/data/images/val/'+file+'.jpg'
img=numpy.array(PIL.Image.open(path1))
#img = numpy.array(PIL.Image.open("/Users/yuan/inputSuperpixel/BSR/BSDS500/data/images/val/101087.jpg"))
plt.figure()
plt.imshow(img)
path2='/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/'+alg+'/'+alg+'_100/'+file+'.mat'
mat_contents=sio.loadmat(path2)
#mat_contents = sio.loadmat('/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/entropy/entropy_150/101087.mat')
#a=mat_contents['S']
#mat_contents = sio.loadmat('/Users/yuan/BaiduYun/MyDocument/CS/MSc Project/Superpixel/Program/seeds/101087.mat')
a=mat_contents['finalLabel']
plt.figure()
plt.imshow(mark_boundaries(img,a,color=(1,0,0)))
plt.show()
