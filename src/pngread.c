
#include <png.h>
#include <stdint.h>

int32_t readpng(char *fn, png_bytep *row_pointers, int32_t *maxx, int32_t *maxy, int32_t *maxxbyte)  /* file is already open */
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 width, height;
   int bit_depth, color_type, interlace_type, xsize, ysize, xbyte;

   FILE *fp = fopen(fn, "rb");
   if (!fp)
      return -4;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {

      fclose(fp);
      return -1;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return -1;
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */

    if (setjmp(png_jmpbuf(png_ptr)))
   {
      // Free all of the memory associated with the png_ptr and info_ptr 
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      // If we get here, we had a problem reading the file 
      return -1;
   }

   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED
    */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, NULL, NULL);

   /* tell libpng to strip 16 bit/color files down to 8 bits/color */
   png_set_strip_16(png_ptr);


   /* test if with alpha channel */
   if (*maxxbyte >= *maxx*4)
   {
     if(color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_PALETTE)
       png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
   }
   else
     /* strip alpha bytes from the input data without combining with th
      * background (not recommended) */
     png_set_strip_alpha(png_ptr);


   /* extract multiple pixels with bit depths of 1, 2, and 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).
    */
   png_set_packing(png_ptr);

   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png_ptr);

   if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand(png_ptr);

   /* expand paletted colors into true RGB triplets */
   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);

   /* optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (ie you selected such a transform above).
    */
   png_read_update_info(png_ptr, info_ptr);

   xbyte = png_get_rowbytes(png_ptr, info_ptr);
   xsize = png_get_image_width(png_ptr, info_ptr);
   ysize = png_get_image_height(png_ptr, info_ptr);

   if (ysize > *maxy || xsize > *maxx || xbyte > *maxxbyte)
   {
     *maxx = xsize;
     *maxy = ysize;
     *maxxbyte = xbyte;

      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return -2;
   }

   *maxx = xsize;
   *maxy = ysize;
   *maxxbyte = xbyte;

   /* Now it's time to read the image.*/
   png_read_image(png_ptr, row_pointers);


   /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
   png_read_end(png_ptr, info_ptr);

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

   /* close the file */
   fclose(fp);

   return 0;
}
