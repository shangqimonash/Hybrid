# -*- coding: utf-8 -*-

from __future__ import print_function

import matplotlib.pyplot as plt
import numpy
from skimage import io, color,filter
from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float
import fh
import PIL.Image

c=10
min_size=100
sigma=0.8

img = numpy.array(PIL.Image.open("fishMan.jpg"))


slicNum=1
segments_slic=numpy.zeros(img.shape[0:2])

segments_slic=segments_slic.astype(numpy.int32)


region_fh=fh.fh_seg(img,segments_slic,slicNum,sigma,c,min_size)
#region_fh=felzenszwalb(img,scale=1,sigma=0.8,min_size=20)
print("FH matrix")
print(type(region_fh))
print(region_fh)

fhNum=len(numpy.unique(region_fh))
print("FH number of segments: %d" % fhNum)

plt.figure()
plt.imshow(mark_boundaries(img,region_fh,color=(1,1,1)))


plt.show()