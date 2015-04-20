# -*- coding: utf-8 -*-
from skimage import graph, data, io, segmentation, color
from matplotlib import pyplot as plt
from skimage.measure import regionprops
from skimage import draw
import numpy as np
 
 
def show_img(img):
    width = 10.0
    height = img.shape[0]*width/img.shape[1]
    f = plt.figure(figsize=(width, height))
    plt.imshow(img)
 
img = data.coffee()
show_img(img)

labels = segmentation.slic(img, compactness=30, n_segments=400,enforce_connectivity=True)
labels = labels + 1  # So that no labelled region is 0 and ignored by regionprops
#Measure properties of labeled image regions.把region的一些attribute求出来，比如pixel num等等
regions = regionprops(labels)
label_rgb = color.label2rgb(labels, img, kind='avg')
show_img(label_rgb)


label_rgb = segmentation.mark_boundaries(label_rgb, labels, (0, 0, 0))
show_img(label_rgb)

rag = graph.rag_mean_color(img, labels)

#把graph中node的region的centroid这个property赋值为之前用regions = regionprops(labels)求出来的
#region.centroid
for region in regions:
    rag.node[region['label']]['centroid'] = region['centroid']

def display_edges(image, g, threshold):
    """Draw edges of a RAG on its image

    Returns a modified image with the edges drawn.Edges are drawn in green
    and nodes are drawn in yellow.

    Parameters
    ----------
    image : ndarray
        The image to be drawn on.
    g : RAG
        The Region Adjacency Graph.
    threshold : float
        Only edges in `g` below `threshold` are drawn.

    Returns:
    out: ndarray
        Image with the edges drawn.
    """
    image = image.copy()
    for edge in g.edges_iter():
        n1, n2 = edge

        r1, c1 = map(int, rag.node[n1]['centroid'])
        r2, c2 = map(int, rag.node[n2]['centroid'])

        line  = draw.line(r1, c1, r2, c2)
        circle = draw.circle(r1,c1,2)

        if g[n1][n2]['weight'] < threshold :
            image[line] = 0,1,0
        image[circle] = 1,1,0

    return image
edges_drawn_all = display_edges(label_rgb, rag, np.inf )
show_img(edges_drawn_all)
edges_drawn_29 = display_edges(label_rgb, rag, 29 )
show_img(edges_drawn_29)

#构建了一个map_array，当这个array输入的index时labels时候，就能得到我的final_label=map_array[label]
final_labels = graph.cut_threshold(labels, rag, 29)
final_label_rgb = color.label2rgb(final_labels, img, kind='avg')
show_img(final_label_rgb)

plt.show()