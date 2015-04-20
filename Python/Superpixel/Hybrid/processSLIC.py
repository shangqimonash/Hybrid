# -*- coding: utf-8 -*-
from multiprocessing import Pool
from hybrid import hybrid
import os
import PIL.Image
import PIL.Image
from skimage.segmentation import felzenszwalb, slic, quickshift
import scipy.io as sio
import numpy

imgFilePath = '/Users/yuan/inputSuperpixel/BSR/BSDS500/data/images/val/'
outputPath = "/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/slic/"
listing = os.listdir(imgFilePath)
n_segments=350
while n_segments<=400:

    i=0

    while i<len(listing):
        file=listing[i]
    #for file in listing:
        if file!='.DS_Store':
            img =img = numpy.array( PIL.Image.open(imgFilePath + file))
            info='processing: '+file
            print(info)
            finalLabel = slic(img, n_segments, compactness=10, sigma=1,enforce_connectivity=True)
            fileName,extension=os.path.splitext(file)
            folder=n_segments.__str__()
            folder='slic_'+folder+'/'
            directory=outputPath+folder
            if not os.path.exists(directory):
                os.makedirs(directory)
            savePath=directory+fileName+'.mat'
            sio.savemat(savePath,{'finalLabel':finalLabel})

        i+=1




    n_segments+=50
