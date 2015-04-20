# -*- coding: utf-8 -*-

'''
发现在skimage中已经写好了FH 和SLIC了，接下来就是你直接调用
'''
from __future__ import print_function

import matplotlib.pyplot as plt
import numpy as np

from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float

'''
[::2,::2]表示每隔两个index娶一个sample，就是down-sampling
'''
img = img_as_float(lena()[::2, ::2])
segments_fz = felzenszwalb(img, scale=100, sigma=0.5, min_size=50)
print (segments_fz)
print (segments_fz.shape)
segments_slic = slic(img, n_segments=4, compactness=10, sigma=1)
np.savetxt('test.txt', segments_slic, fmt='%1i')
segments_quick = quickshift(img, kernel_size=3, max_dist=6, ratio=0.5)

print("Felzenszwalb's number of segments: %d" % len(np.unique(segments_fz)))
print("Slic number of segments: %d" % len(np.unique(segments_slic)))
print("Quickshift number of segments: %d" % len(np.unique(segments_quick)))

fig, ax = plt.subplots(1, 3)
fig.set_size_inches(8, 3, forward=True)
fig.subplots_adjust(0.05, 0.05, 0.95, 0.95, 0.05, 0.05)

ax[0].imshow(mark_boundaries(img, segments_fz))
ax[0].set_title("Felzenszwalbs's method")
ax[1].imshow(mark_boundaries(img, segments_slic))
ax[1].set_title("SLIC")
ax[2].imshow(mark_boundaries(img, segments_quick))
ax[2].set_title("Quickshift")
for a in ax:
    a.set_xticks(())
    a.set_yticks(())
plt.show()