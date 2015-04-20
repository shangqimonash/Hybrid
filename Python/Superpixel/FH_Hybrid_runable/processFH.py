# -*- coding: utf-8 -*-
from multiprocessing import Pool
from fh_pure import fh_pure
import os
import PIL.Image
import PIL.Image
from skimage.segmentation import felzenszwalb, slic, quickshift
import scipy.io as sio
import numpy
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


        num_set=sum(numSetArray)

        slicNum=1
        scale=50
        min_size=50
        sigma=0.2
        setNumRange=25
        time=1
        pre_segNum=0
        stationaryPoint=0
        while num_set!=8:
            #print(time)
            time+=1
            region_fh,fhNum=fh_pure(img,10,scale,min_size,sigma)
            n_segments=len(numpy.unique(region_fh))

            numSetIndex=(n_segments-setNumRange)/50
            if numSetIndex<8:
                if numSetArray[numSetIndex]!=1:
                    leftRange=(numSetIndex+1)*50-setNumRange
                    rightRange=(numSetIndex+1)*50+setNumRange
                    if n_segments<rightRange and n_segments>leftRange:
                        numSetArray[numSetIndex]=1
                        fileName,extension=os.path.splitext(file)
                        folderNum=(numSetIndex+1)*50
                        folder=folderNum.__str__()
                        folder='fh_'+folder+'/'
                        directory=outputPath+folder
                        if not os.path.exists(directory):
                            os.makedirs(directory)
                        savePath=directory+fileName+'.mat'
                        sio.savemat(savePath,{'finalLabel':region_fh})


                        '''
                        plt.figure()
                        plt.imshow(mark_boundaries(img,region_fh,color=(1,1,1)))
                        plt.show()
                        '''
            num_set=sum(numSetArray)
            if num_set==7:
                scale+=60
                min_size+=30

            else:
                if  n_segments-pre_segNum<10 and num_set!=0:
                    stationaryPoint+=1
                    pre_segNum=n_segments
                    if stationaryPoint>25 :
                        if stationaryPoint>40:
                            min_size+=5
                        scale+=15
                    else:
                        scale+=10

                else:
                    pre_segNum=n_segments
                    stationaryPoint=0
                    scale+=10

            out='now img order: '+i.__str__()+'scale is: '+scale.__str__()
            print(out)
            out='segment Num: '+n_segments.__str__()
            print(out)
            print(numSetArray.__str__())

    i+=1



