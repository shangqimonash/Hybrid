/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/* a simple image class */

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>
#include <inttypes.h>

class image_error{};

template <class T>
class image
{
public:
    /* create an image */
    image(const int width, const int height, const bool init = true);

    /* delete an image */
    ~image();

    /* init an image */
    void init(const T &val);

    /* copy an image */
    image<T> *copy() const;

    /* get 32bit RGB expression of the data */
    uint32_t* getRGBData();

    /* get the width of an image. */
    int width() const
    {
        return w;
    }

    /* get the height of an image. */
    int height() const
    {
        return h;
    }

    /* image data. */
    T *data;

    /* row pointers. */
    T **access;

    /* 32bit expression of RGB */
    uint32_t* RGB;

    /* Result of Superpixel */
    int *labels;

private:
    int w, h;
};

/* use imRef to access image data. */
#define imRef(im, x, y) (im->access[y][x])

/* use imPtr to get pointer to image data. */
#define imPtr(im, x, y) &(im->access[y][x])

/* get the 32bit expression of RGB*/
#define RGB32BIT(r,g,b) ((b) + ((g) << 8) + ((r) << 16) + (0 << 24))

template <class T>
image<T>::image(const int width, const int height, const bool init)
{
    w = width;
    h = height;
    data = new T[w * h];  // allocate space for image data
    access = new T*[h];   // allocate space for row pointers

    // initialize row pointers
    for (int i = 0; i < h; i++)
        access[i] = data + (i * w);

    if (init)
        memset(data, 0, w * h * sizeof(T));
}

template <class T>
image<T>::~image()
{
    delete [] data;
    delete [] access;
}

template <class T>
void image<T>::init(const T &val)
{
    T *ptr = imPtr(this, 0, 0);
    T *end = imPtr(this, w-1, h-1);
    while (ptr <= end)
        *ptr++ = val;
}


template <class T>
image<T> *image<T>::copy() const
{
    image<T> *im = new image<T>(w, h, false);
    memcpy(im->data, data, w * h * sizeof(T));
    return im;
}

template <class T>
uint32_t* image<T>::getRGBData()
{
    if((h > 0) || (w > 0))
    {
        RGB = new uint32_t[h * w];
        for(int y = 0; y < h ; y++)
        {
            for(int x = 0; x < w; x++)
            {
                RGB[y * w + x]=RGB32BIT((imPtr(this,x,y))->r, (imPtr(this,x,y))->g, (imPtr(this,x,y))->b);
            }
        }
        return RGB;
    }
    throw image_error();

}

#endif

