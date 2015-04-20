# -*- coding: utf-8 -*-
from break2Build import buildFinalLabel
import rag_graph
import pickle

from skimage.segmentation import relabel_sequential
import matplotlib.pyplot as plt
import numpy as np
from skimage import io, color,filter
from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float
import PIL.Image

file_g = open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/g.obj', 'r')
file_fh=open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/fh.obj','r')
file_num=open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/num.obj','r')
file_g_final = open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/g_final.obj', 'r')
file_final = open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/final.obj', 'r')
file_map = open('/Users/yuan/BaiduYun/MyDocument/workspace/Python/Superpixel/Hybrid/map.obj', 'r')
map=pickle.load(file_map)
g=pickle.load(file_g)
g_final=pickle.load(file_g_final)
finalLabel=pickle.load(file_final)


region_fh=pickle.load(file_fh)
fhNum=pickle.load(file_num)
finalLabel=buildFinalLabel(g,region_fh,fhNum,100)
#finalNum=len(np.unique(finalLabel))
print('befor merge, num is:')
print(fhNum)
print('final num is: ')
#print(finalNum)

img = np.array(PIL.Image.open("fishMan.jpg"))
plt.figure()
plt.imshow(mark_boundaries(img,finalLabel,color=(1,1,1)))
plt.show()
