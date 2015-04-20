# -*- coding: utf-8 -*-

from __future__ import print_function

import matplotlib.pyplot as plt
import numpy
from skimage import io, color,filter
from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float
import fh_pure
import PIL.Image

'''
[::2,::2]表示每隔两个index娶一个sample，就是down-sampling
'''

img = numpy.array(PIL.Image.open("shirtBoy.jpg"))
print (type(img))
lab = color.rgb2lab(img)
print (type(lab))
print (lab.shape)
#img = img_as_float(lena()[::2, ::2])

segments_slic = slic(img, n_segments=100, compactness=10, sigma=1,enforce_connectivity=True)

slicNum=1
segments_slic=numpy.zeros(lab.shape[0:2])

segments_slic=segments_slic.astype(numpy.int32)


c=100
min_size=100
sigma=0.5
plt.figure()



region_fh=fh_pure.fh_seg(img,sigma,c,min_size)
print("FH matrix")
print(type(region_fh))
print(region_fh)

#region_fh_teskMask=fh.fh_seg(img,segments_slic,1,sigma,c,min_size)
fhNum=len(numpy.unique(region_fh))

print("FH number of segments: %d" % fhNum)
print("Slic number of segments: %d" % slicNum)

plt.imshow(mark_boundaries(img, segments_slic,color=(1,0,0)))
plt.imsave("fishMan_slic.jpg",mark_boundaries(img, segments_slic,color=(1,1,1)))
plt.figure()
plt.imshow(mark_boundaries(img,region_fh,color=(1,0,0)))
plt.imsave("fishMan_Hybrid.jpg",mark_boundaries(img,region_fh,color=(1,1,1)))
#plt.imshow(mark_boundaries(img,region_fh_teskMask))

plt.show()