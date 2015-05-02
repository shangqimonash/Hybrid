function [S,time] = entropy(img, numSuperpixels)
grey_img = double(rgb2gray(img));
S=mex_ers(grey_img,numSuperpixels);
S = uint16(S);
time=toc