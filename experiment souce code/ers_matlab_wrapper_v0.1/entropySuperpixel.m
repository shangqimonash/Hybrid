function [S,time] = entropySuperpixel(img, numSuperpixels)
grey_img = rgb2gray(img);
S=mex_ers(grey_img,numSuperpixels);
S = uint16(S);
time=toc