# -*- coding: utf-8 -*-
import numpy as np
x = y = z = np.arange(0.0,5.0,1.0)
x=np.array([[1,2,3],[4,5,6]])
x=x.astype(np.int32)
print(x)
np.savetxt('test.txt', x, fmt='%1i')