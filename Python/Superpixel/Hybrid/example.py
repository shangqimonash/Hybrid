# -*- coding: utf-8 -*-
'''
这个是演示的主程序，通过选择一下两种产生rag的方式，可以决定slic mask之间是否能merge
g=rag.construct_rag(lab,segments_slic,region_fh,slicNum,binNum)
#g=global_rag.construct_rag(img,segments_slic,region_fh,slicNum,binNum)

'''

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

'''
[::2,::2]表示每隔两个index娶一个sample，就是down-sampling
'''

img = numpy.array(PIL.Image.open("fishMan.jpg"))
imgFloat=numpy.array(img,dtype=float)
print (type(img[1,1,1]))
print(img.shape)
lab = color.rgb2lab(img)
print (type(lab[1,1,1]))
print (lab.shape)
n_segments=100

'''
#best
c=10
min_size=100
sigma=0.8
binNum=5
'''
c=5
min_size=100
sigma=0.7
binNum=15
plt.figure()
time1=datetime.now()
segments_slic = slic(img, n_segments, compactness=10, sigma=1,enforce_connectivity=True)
slicLai=numpy.loadtxt('/Users/yuan/Downloads/slic.txt')
slicNum=len(numpy.unique(segments_slic))
print (type(segments_slic))
#这一句是吧slic的结果保存下来，在cpp中调试用的
numpy.savetxt('test.txt', segments_slic, fmt='%1i')

segments_slic=segments_slic.astype(numpy.int32)
print('kakaka')
region_fh2=fh.fh_seg(img,segments_slic,slicNum,sigma,c,min_size)
region_fh=region_fh2
time2=datetime.now()
print("FH matrix")
print(type(region_fh))
print(region_fh)
fhNum=len(numpy.unique(region_fh))
print("FH number of segments: %d" % fhNum)
print("Slic number of segments: %d" % slicNum)
print('the time to do SLIC and FH (ms): %d' % ((time2-time1).total_seconds()*1000))


time1=datetime.now()
#img=gaussian_filter(img,0.1)
#下面是dynamic histogram
#g=rag.construct_rag(imgFloat,segments_slic,region_fh,slicNum,binNum)
g=rag_staticHist.construct_rag(imgFloat,segments_slic,region_fh,slicNum,binNum)

g.freqUpdate()

#g=global_rag.construct_rag(img,segments_slic,region_fh,slicNum,binNum)

#还可以使用lab颜色空间来建立histogram，有一些会过多分出，但是有些手臂又可以识别，所以要找到一个合适的颜
#区分的方法
#g=rag.construct_rag(lab,segments_slic,region_fh,slicNum,binNum)



'''
test
#####################################################################################
{
'''

'''
array的序号是region_fh[y][x]这样index
'''
grassX1=202
grassY1=229
grassX2=197
grassY2=238
grassX3=194
grassY3=254

beltX=221
beltY=247
skyX=222
skyY=77
hairX=220
hairY=103
handX=245
handY=105
leg1X=229
leg1Y=368
leg2X=236
leg2Y=285
water1X=142
water1Y=477
water2X=138
water2Y=451
feetX=226
feetY=446
waterFeetX=204
waterFeetY=439



grassLabel1=region_fh[grassY1][grassX1]
grassLabel2=region_fh[grassY2][grassX2]
grassLabel3=region_fh[grassY3][grassX3]
beltLabel=region_fh[beltY][beltX]
skyLabel=region_fh[skyY][skyX]
hairLabel=region_fh[hairY][hairX]
handLabel=region_fh[handY,handX]
leg1Label=region_fh[leg1Y,leg1X]
leg2Label=region_fh[leg2Y,leg2X]
water1Label=region_fh[water1Y,water1X]
water2Label=region_fh[water2Y,water2X]
feetLabel=region_fh[feetY,feetX]
waterFeetLabel=region_fh[waterFeetY,waterFeetX]

hist1=g.node[grassLabel1]['hist']
hist2=g.node[grassLabel2]['hist']
hist3=g.node[grassLabel3]['hist']
hist4=g.node[beltLabel]['hist']
histSky=g.node[skyLabel]['hist']
histHair=g.node[hairLabel]['hist']
histHand=g.node[handLabel]['hist']
leg1Hist=g.node[leg1Label]['hist']
leg2Hist=g.node[leg2Label]['hist']
water1Hist=g.node[water1Label]['hist']
water2Hist=g.node[water2Label]['hist']
feetHist=g.node[feetLabel]['hist']
waterFeetHist=g.node[waterFeetLabel]['hist']

print('######################################################################')
print('grassUp')
print(hist1)

print('grassMiddle')
print(hist2)
print('grassDown')
print(hist3)
print('belt')
print(hist4)
print('SKy')
print(histSky)
print('Hair')
print(histHair)
print('hand')
print(histHair.size)
print(histHand)

print('leg1')
print(leg1Hist)
print('leg2')
print(leg2Hist)

print('water1')
print(water1Hist)
print('water2')
print(water2Hist)

print('feet histogram')
print(feetHist)
nonZeroIndex=numpy.where(feetHist!=0)
print(nonZeroIndex)
print(feetHist[nonZeroIndex])

print('water near feet')
print(waterFeetHist)
nonZeroIndex=numpy.where(waterFeetHist!=0)
print(nonZeroIndex)
print(waterFeetHist[nonZeroIndex])

print('######################################################################')

'''
test
#####################################################################################
}
'''


time2=datetime.now()
collapse=time2-time1
print('total processing time:')
print(collapse.total_seconds())
print ('here is node info')
print(g.edges())
print(g.nodes())
print('edge num')
print(len(g.edges()))
print('graph node num')
print(len(g.nodes()))
numpy.savetxt('fh_label.txt', region_fh, fmt='%1i')
numpy.savetxt('slic_label.txt', segments_slic, fmt='%1i')
relab, fw, inv = relabel_sequential(segments_slic)
numpy.savetxt('relabel_slic.txt', segments_slic, fmt='%1i')

file_g = open('g.obj', 'w')
file_g_final = open('g_final.obj', 'w')
file_fh=open('fh.obj','w')
file_num=open('num.obj','w')
pickle.dump(g, file_g)
pickle.dump(region_fh,file_fh)
pickle.dump(fhNum,file_num)
file_final=open('final.obj','w')
file_map = open('map.obj', 'w')


'''
test
{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
'''
print('before merge, the pixelCount is: ')
sky2X=235
sky2Y=99
sky2Label=region_fh[sky2Y,sky2X]
beforeCount=g.node[sky2Label]['pixelCount']+g.node[skyLabel]['pixelCount']+g.node[handLabel]['pixelCount']

print(beforeCount)
print(numpy.sum(g.node[sky2Label]['hist'])+numpy.sum(g.node[skyLabel]['hist'])+numpy.sum(g.node[handLabel]['hist']))
print(g.node[sky2Label]['hist']+g.node[skyLabel]['hist']+g.node[handLabel]['hist'])


'''
test
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
'''


finalLabel,map_array,afterGraph=buildFinalLabel(g,region_fh,fhNum,n_segments)

'''
test
#####################################################################################
{
'''
finalHandLabel=finalLabel[handY,handX]
finalHandHist=afterGraph.node[finalHandLabel]['hist']
finalLeg1Label=finalLabel[leg1Y,leg1X]
finalLeg2Label=finalLabel[leg2Y,leg2X]
finalWater1Label=finalLabel[water1Y,water1X]
finalWater2Label=finalLabel[water2Y,water2X]

print('finalHand')
print('after merge, the pixel count is :')
print(afterGraph.node[finalHandLabel]['pixelCount'])
print(numpy.sum(finalHandHist))
print(finalHandHist)
print('problem: the histgoram count is not equal!!!')

print('leg1: ')
print(afterGraph.node[finalLeg1Label]['hist'])
print('leg2: ')
print(afterGraph.node[finalLeg2Label]['hist'])

print('water1')
print(afterGraph.node[finalWater1Label]['hist'])
numpy.savetxt('water1.txt', afterGraph.node[finalWater1Label]['hist'], fmt='%1i')
nonZeroIndex=numpy.where(afterGraph.node[finalWater1Label]['hist']!=0)
print(nonZeroIndex)
print(afterGraph.node[finalWater1Label]['hist'][nonZeroIndex])
print('water2')
print(afterGraph.node[finalWater2Label]['hist'])
numpy.savetxt('water2.txt', afterGraph.node[finalWater2Label]['hist'], fmt='%1i')
nonZeroIndex=numpy.where(afterGraph.node[finalWater2Label]['hist']!=0)
print(nonZeroIndex)
print(afterGraph.node[finalWater2Label]['hist'][nonZeroIndex])



'''
#####################################################################################
test
}
'''


map_array=numpy.array(map_array)
finalLabel=numpy.array(finalLabel)
pickle.dump(g, file_g_final)
pickle.dump(finalLabel,file_final)
pickle.dump(map_array,file_map)
finalNum=len(numpy.unique(finalLabel))
print('finalNum')
print(finalNum)



plt.imshow(img)
plt.figure()
plt.imshow(mark_boundaries(img, segments_slic,color=(1,0,0)))
plt.figure()
plt.imshow(mark_boundaries(img, slicLai,color=(1,0,0)))
plt.imsave("fishMan_slic.jpg",mark_boundaries(img, segments_slic,color=(1,0,0)))
plt.figure()
plt.imshow(mark_boundaries(img,region_fh,color=(1,0,0)))
plt.imsave("fishMan_Hybrid.jpg",mark_boundaries(img,region_fh,color=(1,0,0)))
plt.figure()
plt.imshow(mark_boundaries(img,finalLabel,color=(1,0,0)))
plt.imsave("fishMan_final.jpg",mark_boundaries(img,finalLabel,color=(1,0,0)))
plt.show()
numpy.savetxt('finalLabel.txt', finalLabel, fmt='%1i')

fh = felzenszwalb(img, scale=100, sigma=0.5, min_size=50)
#savePath='/Users/yuan/BaiduYun/MyDocument/workspace/MyMatlab/superpixel_benchmark/result/fh/fh_200/62096..mat'
#sio.savemat(savePath,{'finalLabel':fh})
