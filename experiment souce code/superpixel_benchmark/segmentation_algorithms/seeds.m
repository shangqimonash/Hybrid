function [S,time] = seeds(img, numSuperpixels)
tic
grey_img = double(rgb2gray(img));
[labels] = mexSEEDS(img,numSuperpixels);
S=labels;
%S= mexSEEDS(img,numSuperpixels);
S = uint16(S);
time=toc

