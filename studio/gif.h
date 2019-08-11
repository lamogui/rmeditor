#ifndef _MY_GIF_H_
#define _MY_GIF_H_


#define GIF_NO_ERROR                  0
#define GIF_ERROR_SIZE               -1
#define GIF_ERROR_SIGNATURE          -2
#define GIF_ERROR_BAD_BLOCK          -3
#define GIF_ERROR_BAD_COMPRESSION    -4
#define GIF_ERROR_BAD_INDEX          -5
#define GIF_ERROR_NULL               -6
#define GIF_ERROR_UNKNOW_EXTENSION   -7
#define GIF_ERROR_UNKNOW_BLOCK       -8

#define GIF_IMAGE_DESCRIPTOR       0x2C
#define GIF_TRAILER                0x3B
#define GIF_EXTENSION_INTRODUCER   0x21
#define GIF_GRAPHIC_CONTROL        0xF9
#define GIF_APPLICATION_EXTENSION  0xFF
#define GIF_COMMENT_EXTENSION      0xFE
#define GIF_PLAINTEXT_EXTENSION    0x01

#define GIF_MODE_RGBA              0x1
#define GIF_MODE_BITMAP            0x2

#include <stdlib.h> /*Malloc*/
#include <stdio.h> /*Files*/
#include <string.h> /*String compare*/




#if defined(_WIN32) || defined(__WIN32__)

#ifndef WINVER
#define WINVER 0x500
#endif
#include <Windows.h>
//#define GIF_WINAPI

#include <stdint.h> /*needed for C89 compiler support*/

#elif defined(__unix__) || defined(linux)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long int uint32_t;

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#define GIF_X11

#else

#include "pstdint.h" /*needed for C89 compiler support*/

#endif




#ifdef __cplusplus
   extern "C" {
#endif




/**
 * \struct describe the graphic control extention which allow animated gifs
 * \warning this structure cannot be loaded/saved as is
 */
typedef struct
{
  uint8_t disposal_method;
  uint8_t transparent_color_index;
  uint8_t transparent_flag;
  uint8_t need_user_input;
  uint16_t delay_time;
}
gif_graphic_control_extension;

/**
 * \struct description the application extension which is used by the cretor 
 * application when the gif is created
 * \warning this structure cannot be loaded/saved as is
 */
typedef struct
{
  uint8_t* data;
  uint8_t application_id[ 8 ];
  uint8_t version[ 3 ];
}
gif_application_extension;

/**
 * \struct describe a text extension. The text will normally be showed by the
 * application which load the gif
 */
typedef struct
{
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  uint8_t cell_width;
  uint8_t cell_height;
  uint8_t foreground_color;
  uint8_t background_color;
}
gif_plaintext_extension;

/**
 * \struct describe a dictionary for the lzw compression
 */
typedef struct
{
  uint8_t byte;
  int prev;
  int len;
}
gif_dictionary_entry;

/**
 * \struct describe the final screen which the gif is rendered
 * \warning sizeof give the wrong size of the structure in the file (real size 
 * is 7 bytes)
 */
typedef struct
{
  uint16_t width;
  uint16_t height;
  uint8_t fields;
  uint8_t background_color_index;
  uint8_t pixel_aspect_ratio;
}
gif_screen_descriptor; /*First GIF Head*/

/**
 * \struct placed before all gif_images the descriptor give the position/size
 * of each frames
 * \warning sizeof give the wrong size of the structure in the file (real size
 * is 9 bytes)
 */
typedef struct
{
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  uint8_t fields;
}
gif_image_descriptor;

/**
 * \struct for manipulate each frames of the animation
 */
typedef struct
{
  uint8_t* local_color_table;
  uint8_t* pixels;
  gif_graphic_control_extension* control;
  uint16_t local_color_table_size;
  gif_image_descriptor descriptor;
  
} gif_image;

/**
 * \struct contains all the gif files informations...
 */
typedef struct
{
   uint8_t* global_color_table;
   gif_image** images;
   gif_application_extension* application; 
   uint8_t* screen;
   uint8_t* previous;
   #ifdef GIF_WINAPI
   UINT transparent; /*transparent color*/
   HBITMAP hBitmap; /*winapi bitmap*/
   #endif
   int color_resolution_bits;
   unsigned int images_count;
   uint16_t global_color_table_size;
   gif_screen_descriptor screen_descriptor;
   char header[7];
} gif;

/**
 * @return the last error code 
 */
int gif_get_last_error(void);


/**
 * @return a new allocated gif structure
 */
gif* new_gif(void);

/**
 * @return a new allocated gif frame structure
 */
gif_image* new_gif_image(void);

/**
 * @return a new allocated gif application extension
 */
gif_application_extension* new_gif_app_ext(void);

/**
 * @param the gif application extension
 */
void delete_gif_app_ext(gif_application_extension* ae);

/**
 * @param the gif frame to detroy
 */
void delete_gif_image(gif_image* i);

/**
 * @param the gif to detroy
 */
void delete_gif(gif* g);

/**
 * @param the gif which you init the screen
 * @param screen mode : RGBA or reversed BGR winapi bitmap
 * @param optionnal HDC for BGR winapi mode
 */
void gif_init_screen(gif* g, int mode, void* hdc);

int gif_lzw_uncompress( unsigned int code_length,
                        const uint8_t *input,
                        unsigned int input_length,
                        uint8_t *out );
                        
/**
 * @return the number of bytes readed
 * @param 
 */                     
int gif_read_sub_blocks(uint8_t* stream, 
                        unsigned int size, 
                        unsigned int* pos,
                        uint8_t **data );


/**
 * @return the gif_image extracted from the stream
 * @param stream the file stream which must be decoded as a gif_image
 * @param size the size of the stream
 * @param pos the actual position in the stream
 */
gif_image* process_gif_image_stream(int8_t* const stream, 
                                    unsigned int size,
                                    unsigned int *pos);


/**
 * @return the gif_control extracted from the stream
 * @param stream the file stream which must be decoded as a gif_image
 * @param size the size of the stream
 * @param pos the actual position in the stream
 */                                    
gif_graphic_control_extension* process_gif_control_stream(int8_t* const stream, 
                                                          unsigned int size,
                                                          unsigned int *pos);                             


/**
 * @return the gif_application_extension extracted from the stream
 * @param stream the file stream which must be decoded as a gif_appext
 * @param size the size of the stream
 * @param pos the actual position in the stream
 */                                                            
gif_application_extension* process_gif_appext_stream(int8_t* const stream, 
                                                     unsigned int size,
                                                     unsigned int *pos);
                                                          
                                                          
/**
 * @return the complete gif extrated from the stream
 * @param stream the file stream which must be decoded as a gif
 * @param size the size of the stream
 */
gif* load_gif_from_stream(const int8_t* stream,
                          unsigned int size);

                        
/**
 * @return non zero if error 
 * @param the "screen" rgba buffer
 * @param the gif struct to decode
 * @param the frame number
 * @param the last frame marked as "disposal"
 * @param draw output mode BITMAP or RGBA
 */                        
int gif_draw_frame(uint8_t* screen,
                   const gif*, 
                   unsigned int n,
                   uint8_t* previous,
                   int mode);
                   
                   


                        
#ifdef __cplusplus
   }
#endif

#endif
