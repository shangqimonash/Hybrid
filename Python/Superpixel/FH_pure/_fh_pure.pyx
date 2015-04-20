#-*- coding: utf-8 -*-

cimport numpy as np
import numpy as np

np.import_array()

cdef extern from "segment-image.h":
    void segment_image(unsigned int* bitImg, int width_img,  int height_img, int* outLabels_img,float sigma_img, float c_img, int min_size_img,
                       int * num_ccs_img)

def fh_seg(np.ndarray[np.uint8_t, ndim=3] img, float sigma, float c, int min_size):

    cdef np.ndarray[np.uint8_t, ndim=3] img_ = np.empty((img.shape[0], img.shape[1], 4), dtype=np.uint8)
    img_[:, :, 1:] = img
    cdef int height=img.shape[0]
    cdef int width=img.shape[1]
    cdef int * outLabels
    #outLabels=new int[width*height]
    cdef np.ndarray[np.int32_t, ndim=1] myarr = np.empty(width*height, dtype=np.int32)
    cdef int* num_ccs
    cdef int labelData=< int >myarr.data
    cdef int * labelPointer=&labelData
    #cdef int* mask=&slicMask[0,0]
    segment_image(<unsigned int *>img_.data,width,height,labelPointer,sigma,c,min_size,num_ccs)


    cdef np.npy_intp shape[2]
    shape[0] = height
    shape[1] = width
    label_array = np.PyArray_SimpleNewFromData(2, shape, np.NPY_INT32, <void*> outLabels)
    return label_array
