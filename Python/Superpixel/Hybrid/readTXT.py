# -*- coding: utf-8 -*-
from __future__ import print_function
from skimage.segmentation import relabel_sequential
import matplotlib.pyplot as plt
import numpy
from skimage import io, color,filter
from skimage.data import lena
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.util import img_as_float
import fh
import rag
import global_rag
import rag_staticHist
import PIL.Image
import rag_graph
from datetime import datetime
from break2Build import buildFinalLabel
import pickle
from scipy.ndimage.filters import gaussian_filter
import scipy.io as sio

numpy.loadtxt('/Users/yuan/Downloads/SLIC.txt')