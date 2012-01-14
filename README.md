# ExoQuant v0.7

ExoQuant is a high-quality, easy to use color quantization library. This is for you if you need one or more of the following:

* Very high-quality color reduction
* Reduction of images including alpha
* Creation of a shared palette for more than one image (or mipmap level)
* Dithering of the reduced image with very little noise

## Usage:

First, of course, you need to include `exoquant.h`:

    #include "exoquant.h"

Then for each image or texture to convert follow the following steps:

### Step 1: Initialise and set options.

First you need to init the library by calling `exq_init`:

    exq_data *pExq;
    pExq = exq_init();

Then you can set the following options:

#### Option: Alpha is no transparency

Use this options if you don't use the alpha channel of your image/texture as transparency or if the color is already premultiplied by the alpha. To set this options just call `exq_no_transparency`:

    exq_no_transparency(pExq);

### Step 2: Feed the image data

Now you need to feed the image data to the quantizer. The image data needs to be 32 bit per pixel. The first byte of each pixel needs to be the red channel, the last byte needs to be alpha.

To feed the image data you have to call `exq_feed`, which can be called more than once to create a shared palette for more than one image, for examples for a texture with several mipmap levels:

    exq_feed(pExq, pImage_data, num_pixels);

### Step 3: Color reduction

    exq_quantize(pExq, num_colors);
    exq_quantize_hq(pExq, num_colors);
    exq_quantize_ex(pExq, num_colors, high_quality);

### Step 4: Retrieve the palette

    exq_get_palette(pExq, pPalette, num_colors);

### Step 5: Map the image to the palette

    exq_map_image(pExq, num_pixels, pIn, pOut);
    exq_map_image_ordered(pExq, width, height, pIn, pOut);

### Step 6: Clean up

Call `exq_free` to free the qunatization data:

   exq_free(pExq);

## Licence

ExoQuant v0.7

Copyright (c) 2004 Dennis Ranke

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
