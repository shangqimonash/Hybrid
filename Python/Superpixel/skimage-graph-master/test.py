# -*- coding: utf-8 -*-
import time
import numpy as np
import graph_csr as graph
from skimage import segmentation
import memory_profiler as mp
import cProfile


def test():
    arr = np.load("../data/watershed.npy")
    #这里最后的那个[0] index，是因为relabel_sequential()会返回三个元素，relabeled，
    #forward_map, inverse_map,这里就是取一个relabeled这个变量
    #这里的返回值arr也是一个numpy.ndarray，所以可以放心的使用了
    arr = segmentation.relabel_sequential(arr)[0]
    t = time.time()

    cProfile.runctx(
        "g = graph.construct_rag(arr)",
        globals(),
        locals(),
        "const.prof")

    cProfile.runctx(
        "g = g.random_merge(10)",
        globals(),
        locals(),
        "merge.prof")

    # print "Memory for Merging =",memory[0] - base[0]

test()
