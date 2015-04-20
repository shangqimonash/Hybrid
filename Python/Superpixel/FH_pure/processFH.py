# -*- coding: utf-8 -*-
from multiprocessing import Pool
import os
import PIL.Image
import PIL.Image
from skimage.segmentation import felzenszwalb, slic, quickshift
import scipy.io as sio
import numpy
import fh_pure
import matplotlib.pyplot as plt
from skimage import color
from skimage.segmentation import mark_boundaries

imgFilePath = '/Users/yuan/inputSuperpixel/BSR/BSDS500/data/images/val/'
outputPath = "/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/fh/"
listing = os.listdir(imgFilePath)

file=listing[1]
img = numpy.array( PIL.Image.open(imgFilePath + file))
info='processing: '+file
print(info)


print (type(img))
lab = color.rgb2lab(img)
print (type(lab))
print (lab.shape)
#img = img_as_float(lena()[::2, ::2])


i=0
lenth=len(listing)
while i<len(listing):
    file=listing[i]
#for file in listing:
    if file!='.DS_Store':
        img = numpy.array( PIL.Image.open(imgFilePath + file))
        info='processing: '+file
        print(info)
        numSetArray=[0]*8
        slicNum=1

        scale=50
        min_size=200
        sigma=0.8
        setNumRange=25
        num_set=sum(numSetArray)

        while num_set!=8:
            segments_slic = slic(img, n_segments=100, compactness=10, sigma=1,enforce_connectivity=True)
            region_fh=fh_pure.fh_seg(img,sigma,scale,min_size)
            n_segments=len(numpy.unique(region_fh))
            numSetIndex=(n_segments-setNumRange)/50
            if numSetIndex<8:
                if numSetArray[numSetIndex]!=1:
                    leftRange=(numSetIndex+1)*50-setNumRange
                    rightRange=(numSetIndex+1)*50+setNumRange
                    if n_segments<rightRange and n_segments>leftRange:
                        numSetArray[numSetIndex]=1
                        fileName,extension=os.path.splitext(file)
                        folder=n_segments.__str__()
                        folder='fh_'+folder+'/'
                        directory=outputPath+folder
                        if not os.path.exists(directory):
                            os.makedirs(directory)
                        savePath=directory+fileName+'.mat'
                        sio.savemat(savePath,{'finalLabel':region_fh})
            scale+=20
            num_set=sum(numSetArray)
    i+=1



