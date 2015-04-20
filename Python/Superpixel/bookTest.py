# -*- coding: utf-8 -*-
'''
直接调用slic这个模块，返回分别看了一下有没有connectivity的不同

'''


from skimage.segmentation import slic, mark_boundaries
from skimage.data import lena

import matplotlib.pyplot as plt

img = lena()
path='/Users/yuan/inputSuperpixel/'
filename='/Users/yuan/Downloads/lena_org.jpg'
filename='/Users/yuan/inputSuperpixel/101087.jpg'
img=plt.imread(filename)

plt.figure()
plt.title('Original version')
plt.imshow(img)
plt.figure()
slicWithConnectity=mark_boundaries(img, slic(img, 40, 10, sigma=1, enforce_connectivity=False))
plt.imshow(slicWithConnectity)
plt.imsave(path+'Noconnect.jpg',slicWithConnectity)
plt.title('skimage version\n enforce_connectivity=False', size=9)

plt.figure()
slicNoConnnectity=mark_boundaries(img, slic(img, 40, 10, sigma=1, enforce_connectivity=True))
plt.imshow(slicNoConnnectity)
plt.imsave(path+'Connect.jpg',slicNoConnnectity)
plt.title('skimage version\n  enforce_connectivity=True', size=9)
plt.show()