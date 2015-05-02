img = imread('101087.jpg');



%%
%//=======================================================================
%// Superpixel segmentation
%//=======================================================================
%// nC is the target number of superpixels.
nC = 200;
%// Call the mex function for superpixel segmentation\
%// !!! Note that the output label starts from 0 to nC-1.
t = cputime;
[labels] = mexSEEDS(img,nC);


[height width] = size(grey_img);

%// Compute the boundary map and superimpose it on the input image in the
%// green channel.
%// The seg2bmap function is directly duplicated from the Berkeley
%// Segmentation dataset which can be accessed via
%// http://www.eecs.berkeley.edu/Research/Projects/CS/vision/bsds/
[bmap] = seg2bmap(labels,width,height);
bmapOnImg = img;
idx = find(bmap>0);
timg = grey_img;
timg(idx) = 255;
bmapOnImg(:,:,2) = timg;
bmapOnImg(:,:,1) = grey_img;
bmapOnImg(:,:,3) = grey_img;

%// Randomly color the superpixels
[out] = random_color( double(img) ,labels,nC);

%// Compute the superpixel size histogram.
siz = zeros(nC,1);
for i=0:(nC-1)
    siz(i+1) = sum( labels(:)==i );
end
[his bins] = hist( siz, 20 );

%%
%//=======================================================================
%// Display 
%//=======================================================================
gcf = figure(1);
subplot(2,3,1);
imshow(grey_img,[]);
title('input grey scale image.');
subplot(2,3,2);
imshow(bmapOnImg,[]);
title('superpixel boundary map');
subplot(2,3,3);
imshow(out,[]);
title('randomly-colored superpixels');
subplot(2,3,5);
bar(bins,his,'b');
title('the distribution of superpixel size');
ylabel('# of superpixels');
xlabel('superpixel sizes in pixel');
scnsize = get(0,'ScreenSize');
set(gcf,'OuterPosition',scnsize);


figure();
imshow(bmapOnImg,[]);
title('superpixel boundary map');
figure();
imshow(out,[]);
title('randomly-colored superpixels');