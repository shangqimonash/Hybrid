# -*- coding: utf-8 -*-
from multiprocessing import Pool
from hybrid import hybrid
import os
import PIL.Image
import PIL.Image

import scipy.io as sio
import numpy

imgFilePath = '/Users/yuan/inputSuperpixel/BSR/BSDS500/data/images/val/'
outputPath = "/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/hybrid/"
listing = os.listdir(imgFilePath)
n_segments=300
while n_segments<=300:
    i=59
    while i<len(listing):
        file=listing[i]
    #for file in listing:
        if file!='.DS_Store':
            img =img = numpy.array( PIL.Image.open(imgFilePath + file))
            info='processing: '+file
            print(info)
            finalLabel=hybrid(img,n_segments)
            fileName,extension=os.path.splitext(file)
            folder=n_segments.__str__()
            folder='hybrid_'+folder+'/'
            directory=outputPath+folder
            if not os.path.exists(directory):
                os.makedirs(directory)
            savePath=directory+fileName+'.mat'
            sio.savemat(savePath,{'finalLabel':finalLabel})

        i+=1


    n_segments+=50
