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
import PIL.Image
import rag_graph
from datetime import datetime
from break2Build import buildFinalLabel
import pickle
from scipy.ndimage.filters import gaussian_filter
import copy

def fh_pure(img,n_segments,c,min_size,sigma):

    segments_slic = slic(img, n_segments, compactness=10, sigma=1,enforce_connectivity=True)
    segments_slic=numpy.zeros(img.shape[0:2])
    segments_slic=segments_slic.astype(numpy.int32)
    slicNum=len(numpy.unique(segments_slic))
    #imgArray=copy.deepcopy(img)
    imgArray=img
    region_fh=fh.fh_seg(imgArray,segments_slic,slicNum,sigma,c,min_size)
    fhNum=len(numpy.unique(region_fh))
    '''
    g=rag.construct_rag(lab,segments_slic,region_fh,slicNum,binNum)
    finalLabel,map_array=buildFinalLabel(g,region_fh,fhNum,n_segments)
    finalLabel=numpy.array(finalLabel)
    '''
    finalLabel=numpy.array(region_fh)
    return finalLabel,fhNum
