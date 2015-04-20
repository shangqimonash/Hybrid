import numpy as np
import PIL.Image

import matplotlib.pyplot as plt
import slic

im = np.array(PIL.Image.open("grass.jpg"))
region_labels = slic.slic_n(im, 1000, 10)
print type(region_labels)
contours = slic.contours(im, region_labels, 10)
plt.imshow(contours[:, :, :-1].copy())
plt.show()
