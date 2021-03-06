
#include "gif.hpp"

static int gif_error=0;

int gif_get_last_error(void)
{
   return gif_error;
}

gif* new_gif(void)
{  
   gif* g = (gif*) malloc(sizeof(gif));
   g->global_color_table_size=0;
	 g->global_color_table=nullptr;
   g->images_count=0;
	 g->images=nullptr;
	 g->application=nullptr;
	 g->screen=nullptr;
	 g->previous=nullptr;
   return g;
}

gif_image* new_gif_image(void)
{  
   gif_image* i = (gif_image*) malloc(sizeof(gif_image));
   i->local_color_table_size = 0;
	 i->local_color_table = nullptr;
	 i->pixels=nullptr;
	 i->control=nullptr;
   return i;
}


gif_application_extension* new_gif_app_ext(void)
{
   gif_application_extension* ae = 
      (gif_application_extension*) malloc(sizeof(gif_application_extension));
	 ae->data=nullptr;
   return ae;
}


void delete_gif_app_ext(gif_application_extension* ae)
{
   if (ae)
   {
      if (ae->data)
         free((void*)ae->data);
      free((void*)ae);
   }
}


void delete_gif_image(gif_image* i)
{
   if (i->local_color_table_size && i->local_color_table)
   {
      free((void*)i->local_color_table);
   }
   if (i->pixels)
   {
      free((void*)i->pixels);
   }
   if (i->control)
   {
      free((void*)i->control);
   }
   free((void*)i);
}

void delete_gif(gif* g)
{
   unsigned int i;
   if (g->global_color_table_size && g->global_color_table)
   {
      free((void*)g->global_color_table);      
   }
   if (g->images_count && g->images)
   {
      for (i=0;i<g->images_count;i++)
      {
         delete_gif_image(g->images[i]);
      }
      free((void*)g->images);
   }
   if (g->application)
   {
      delete_gif_app_ext(g->application);
   }

	 if (g->screen) free(g->screen);
   if (g->previous) free(g->previous);
   free((void*)g);
}


void gif_init_screen(gif* g, int mode, void* hdc)
{
   if (g)
   {
      const unsigned s = g->screen_descriptor.height*g->screen_descriptor.width*4;
      (void) hdc;
			if (g->screen) free(g->screen);
			g->previous=(uint8_t*)malloc(s);
			g->screen=(uint8_t*)malloc(s);
			memset((void*)g->screen,0,s);
      memcpy((void*)g->previous,(void*)g->screen,s);
      
   }
}

int gif_lzw_uncompress( unsigned int code_length,
                        const uint8_t *input,
                        unsigned int input_length,
                        uint8_t *out )
{
  int i, bit;
  int code, prev = -1;
  gif_dictionary_entry  *dictionary;
  int dictionary_ind;
  unsigned int mask = 0x01;
  int reset_code_length;
  int clear_code; /* This varies depending on code_length */
  int stop_code;  /* one more than clear code */
  int match_len;

  clear_code = 1 << ( code_length );
  stop_code = clear_code + 1;
  /* To handle clear codes */
  reset_code_length = code_length;

  /* 
   Create a dictionary large enough to hold "code_length" entries.
   Once the dictionary overflows, code_length increases
   */
   
   dictionary = ( gif_dictionary_entry * ) 
   malloc( sizeof( gif_dictionary_entry ) * ( 1 << ( code_length + 1 ) ) );
   /*
   Initialize the first 2^code_len entries of the dictionary with their
   indices.  The rest of the entries will be built up dynamically.

   Technically, it shouldn't be necessary to initialize the
   dictionary.  The spec says that the encoder "should output a
   clear code as the first code in the image data stream".  It doesn't
   say must, though...
   */
  for ( dictionary_ind = 0; 
        dictionary_ind < ( 1 << code_length ); 
        dictionary_ind++ )
  {
    dictionary[ dictionary_ind ].byte = dictionary_ind;
    /* XXX this only works because prev is a 32-bit int (> 12 bits) */
    dictionary[ dictionary_ind ].prev = -1;
    dictionary[ dictionary_ind ].len = 1;
  }

  /* 2^code_len + 1 is the special "end" code; don't give it an entry here */
  dictionary_ind++;
  dictionary_ind++;
  
  /* TODO verify that the very last byte is clear_code + 1 */
  while ( input_length )
  {
    code = 0x0;
    /* Always read one more bit than the code length */
    for ( i = 0; (unsigned int) i < ( code_length + 1 ); i++ )
    {
     /* 
      This is different than in the file read example; that 
      was a call to "next_bit"
      */
      bit = ( *input & mask ) ? 1 : 0;
      mask <<= 1;

      if ( mask == 0x100 )
      {
        mask = 0x01;
        input++;
        input_length--;
      }

      code = code | ( bit << i );
    }

    if ( code == clear_code )
    {
      code_length = reset_code_length;
      dictionary = ( gif_dictionary_entry * ) realloc( dictionary,
        sizeof( gif_dictionary_entry ) * ( 1 << ( code_length + 1 ) ) );

      for ( dictionary_ind = 0; 
            dictionary_ind < ( 1 << code_length ); 
            dictionary_ind++ )
      {
        dictionary[ dictionary_ind ].byte = dictionary_ind;
        /* XXX this only works because prev is a 32-bit int (> 12 bits) */
        dictionary[ dictionary_ind ].prev = -1;
        dictionary[ dictionary_ind ].len = 1;
      }
      dictionary_ind++;
      dictionary_ind++;
      prev = -1;
      continue;
    }
    else if ( code == stop_code )
    {
      if ( input_length > 1 )
      {
        return GIF_ERROR_BAD_COMPRESSION;
      }
      break;
    }

    /* 
      Update the dictionary with this character plus the _entry_
      (character or string) that came before it
    */
    if ( ( prev > -1 ) && ( code_length < 12 ) )
    {
      if ( code > dictionary_ind )
      {
        return GIF_ERROR_BAD_COMPRESSION;
      }

      /* Special handling for KwKwK  */
      if ( code == dictionary_ind )
      {
        int ptr = prev;

        while ( dictionary[ ptr ].prev != -1 )
        {
          ptr = dictionary[ ptr ].prev;
        }
        dictionary[ dictionary_ind ].byte = dictionary[ ptr ].byte;
      }
      else
      {
        int ptr = code;
        while ( dictionary[ ptr ].prev != -1 )
        {
          ptr = dictionary[ ptr ].prev;
        }
        dictionary[ dictionary_ind ].byte = dictionary[ ptr ].byte;
      }

      dictionary[ dictionary_ind ].prev = prev;

      dictionary[ dictionary_ind ].len = dictionary[ prev ].len + 1;

      dictionary_ind++;

      /* GIF89a mandates that this stops at 12 bits */
      if ( ( dictionary_ind == ( 1 << ( code_length + 1 ) ) ) &&
           ( code_length < 11 ) )
      {
        code_length++;

        dictionary = ( gif_dictionary_entry * ) realloc( dictionary,
          sizeof( gif_dictionary_entry ) * ( 1 << ( code_length + 1 ) ) );
      }
    }

    prev = code;

    /* Now copy the dictionary entry backwards into "out" */
    match_len = dictionary[ code ].len;
    while ( code != -1 )
    {
      out[ dictionary[ code ].len - 1 ] = dictionary[ code ].byte;
      if ( dictionary[ code ].prev == code )
      {
        return GIF_ERROR_BAD_COMPRESSION;
      }
      code = dictionary[ code ].prev;
    }

    out += match_len;
  }
  return GIF_NO_ERROR;
}

/* sub blocks are just blocks to reassemble */

int gif_read_sub_blocks(uint8_t* stream, 
                        unsigned int size, 
                        unsigned int* pos,
                        uint8_t **data )
{
  unsigned int data_length;
  unsigned int index;
  uint8_t block_size;

  /*  
   Everything following are data sub-blocks, until a 0-sized block is
   encountered.
   */

  data_length = 0;
	*data = nullptr;
  index = 0;

   while (1)
   {
      if (size < *pos + 1)
      {
         gif_error = GIF_ERROR_SIZE;
         if (*data)
            free((void*)data);
         return 0;
      } else {
         block_size = stream[(*pos)++];
      }
      
      if ( block_size == 0 )  /* end of sub-blocks */
      {
         break;
      }

      data_length += block_size;
      *data = (uint8_t*) realloc( *data, data_length );
      
      if (size < *pos + block_size) {
         gif_error = GIF_ERROR_SIZE;
         if (*data)
            free((void*)data);
         return 0;
      }
      else
      {
         memcpy((void*) ((*data)+index),(void*)&(stream[*pos]),block_size);
         index += block_size;
         *pos += block_size;
      }
   }
   return data_length;
}


gif_graphic_control_extension* process_gif_control_stream(const int8_t* stream,
                                                          unsigned int size,
                                                          unsigned int *pos)
{
	 gif_graphic_control_extension* gce = nullptr;
   uint8_t fields;
   if (size < *pos + 5) {
      gif_error=GIF_ERROR_SIZE;
   } else if (stream[*pos] != 0x4){
      gif_error=GIF_ERROR_BAD_BLOCK;
   } else {
      gce = (gif_graphic_control_extension*) 
                malloc(sizeof(gif_graphic_control_extension));
      fields=stream[++(*pos)];
      gce->delay_time=stream[++(*pos)];
      gce->delay_time|=stream[++(*pos)] << 8;
      gce->transparent_color_index=stream[++(*pos)];
      gce->transparent_flag = fields & 1;
      gce->need_user_input = (fields >> 1) & 1;
      gce->disposal_method = (fields >> 2) & 7;
      
      (*pos)++;
      
      printf("transparent %u, need_user_input %u\n",gce->transparent_flag,
                                                    gce->need_user_input);
   }
   return gce;
}

gif_application_extension* process_gif_appext_stream(int8_t* const stream, 
                                                     unsigned int size,
                                                     unsigned int *pos)
{
	 gif_application_extension* gae=nullptr;
   if (size < *pos + 12) {
      gif_error=GIF_ERROR_SIZE;
   } else if (stream[(*pos)] != 11){
      gif_error=GIF_ERROR_BAD_BLOCK;
   }
   else {
      gae=new_gif_app_ext();
      (*pos)++;
      memcpy((void*)gae->application_id,(void*)&(stream[*pos]),8);
      *pos += 8;
      memcpy((void*)gae->version,(void*)&(stream[*pos]),3);
      *pos += 3;
      
      gif_read_sub_blocks((uint8_t*)stream, size, pos, &(gae->data));
   }
   return gae;
}

gif_image* process_gif_image_stream(const int8_t* stream,
                                    unsigned int size,
                                    unsigned int *pos)
{
   gif_image* gi = new_gif_image();
   unsigned int compressed_data_length;
   unsigned int uncompressed_data_length;
	 uint8_t *compressed_data = nullptr;
   int error;
   uint8_t lzw_code_size;
   if (*pos + 9 < size)
   {
      memcpy((void*)&(gi->descriptor),(void*)&(stream[*pos]),9);
      *pos += 9;
   }
   else
   {
      delete_gif_image(gi);
      gif_error=GIF_ERROR_SIZE;
			return nullptr;
   }
   /*Load the local color table if exists... */
   if ( gi->descriptor.fields & 0x80 )
   {
      /* If bit 7 is set, the next block is a local color table; read it */
      gi->local_color_table_size = 1 << 
      ( ( ( gi->descriptor.fields & 0x07 ) + 1 ) );
      if (size > *pos + 3 * gi->local_color_table_size)
      {
         gi->local_color_table = ( uint8_t* ) 
                                    malloc(3 * gi->local_color_table_size);
                                    
         memcpy((void*) gi->local_color_table,
                (void*) &(stream[*pos]),
                3 * gi->local_color_table_size);
         *pos += 3 * gi->local_color_table_size;
      }
      else {
         delete_gif_image(gi);
         gif_error=GIF_ERROR_SIZE;
				 return nullptr;
      }
   }
   
   if (*pos + 1 < size)
   {
      lzw_code_size=stream[*pos];
      *pos += 1;
   }
   else
   {
      delete_gif_image(gi);
      gif_error=GIF_ERROR_SIZE;
			return nullptr;
   }
   
   compressed_data_length = gif_read_sub_blocks( (uint8_t*)stream,
                                                size,
                                                pos,
                              (uint8_t**) &compressed_data);
   if (!compressed_data_length)
   {
      printf("Data sub block error ! here we are offset 0x%X ,byte 0x%X\n",*pos,stream[*pos]);
      delete_gif_image(gi);
      free((void*)compressed_data);
			return nullptr;
   }
   
   uncompressed_data_length = gi->descriptor.width * gi->descriptor.height;
   gi->pixels = (uint8_t *) malloc( uncompressed_data_length );
   error = gif_lzw_uncompress( lzw_code_size, 
                               compressed_data, 
                               compressed_data_length, 
                               gi->pixels );
   if (error)
   {
      printf("Decompression block error ! here we are offset 0x%X, byte 0x%X\n",*pos,stream[*pos]);
      gif_error=error;
      delete_gif_image(gi);
      free((void*)compressed_data);
			return nullptr;
   }
   return gi;
}


gif* load_gif_from_stream(const int8_t*  stream,
                        unsigned int size)
{
	 gif* p_gif=nullptr;
   unsigned int pos=0;
   uint8_t block_type = 0x0;
   uint8_t ext_label = 0x0;
	 gif_image* current_image=nullptr;
	 gif_graphic_control_extension* current_control=nullptr;
   
   if (size < 23)
   {
      gif_error=GIF_ERROR_SIZE;
      return p_gif;
   }
  
   p_gif = new_gif();

   /* A GIF file starts with a Header (section 17) */
   memcpy((void*)p_gif->header,(void*)&(stream[pos]),6);
   p_gif->header[6] = 0x0;

   /* 
    there's another format, GIF87a, that you may still find
    floating around.
    */
   if ( strcmp( "GIF89a", p_gif->header ) && 
        strcmp( "GIF87a", p_gif->header ))
   {
      gif_error=GIF_ERROR_SIGNATURE;
      delete_gif(p_gif);
			return nullptr;
   }

   /* 
    Followed by a logical screen descriptor
    Note that this works because GIFs specify little-endian order; on a
    big-endian machine, the height & width would need to be reversed.

    Can't use sizeof here since GCC does byte alignment
   */
   pos+=6;
   memcpy((void*)&(p_gif->screen_descriptor),(void*)&(stream[pos]),7);
   pos+=7;
   /*
   printf("Dimenssions : %ux%u\n",p_gif->screen_descriptor.width,
                                  p_gif->screen_descriptor.height);
   */
   p_gif->color_resolution_bits=((p_gif->screen_descriptor.fields & 0x70)>>4);
   p_gif->color_resolution_bits++;
   
   /* Load the global color table if exists... */
   if ( p_gif->screen_descriptor.fields & 0x80 )
   {
      /* If bit 7 is set, the next block is a global color table; read it */
      p_gif->global_color_table_size = 1 << 
      ( ( ( p_gif->screen_descriptor.fields & 0x07 ) + 1 ) );
      if (size > pos + 3 * p_gif->global_color_table_size)
      {
         p_gif->global_color_table = ( uint8_t* ) 
                                    malloc(3 * p_gif->global_color_table_size);
                                    
         memcpy((void*) p_gif->global_color_table,
                (void*) &(stream[pos]),
                3 * p_gif->global_color_table_size);
         pos += 3 * p_gif->global_color_table_size;
      }
      else {
         delete_gif(p_gif);
         gif_error=GIF_ERROR_SIZE;
				 return nullptr;
      }
   } 
   
   while ( block_type != GIF_TRAILER )
   {
      if (size < pos + 1)
      {
         gif_error=GIF_ERROR_SIZE;
         delete_gif(p_gif);
         if (current_control)
            free((void*)current_control);
				 return nullptr;
      }
      else
      {
         block_type = stream[pos];
         pos++;
      }

      switch ( block_type )
      {
         case GIF_IMAGE_DESCRIPTOR:
            printf("processing image descriptor\n");
            current_image=process_gif_image_stream(stream,size,&pos);
            if (current_image)
            {
               if (current_control)
               {
                  current_image->control=current_control;
									current_control=nullptr;
               }
               p_gif->images_count++;
               printf("Image added !\n");
               p_gif->images = (gif_image**)
                        realloc((void*)p_gif->images,
                                 p_gif->images_count*sizeof(gif_image*));
               p_gif->images[p_gif->images_count-1]=current_image;
            }
            else
            {
               delete_gif(p_gif);
							 return nullptr;
            }
            break;
            
         case GIF_EXTENSION_INTRODUCER:
            printf("processing extension\n");
            if (size < pos + 1)
            {
               gif_error=GIF_ERROR_SIZE;
               delete_gif(p_gif);
               if (current_control)
                  free((void*)current_control);
							 return nullptr;
            }
            else
            {
               ext_label = stream[pos];
               pos++;
            }
            switch (ext_label)
            {
               case GIF_GRAPHIC_CONTROL:
                  printf("processing graphic control\n");
                  if (current_control)
                     free((void*)current_control);
                  current_control=process_gif_control_stream(stream,size,&pos);
                  if (!current_control)
                  {
                     delete_gif(p_gif);
										 return nullptr;
                  }
                  break;
                  
               case GIF_APPLICATION_EXTENSION:
                  printf("processing app ext\n");
                  if (p_gif->application)
                  {
                     delete_gif_app_ext(p_gif->application);
                  }
                  break;
               case GIF_COMMENT_EXTENSION:
                  printf("Gif Comment extension not supported !\n");
                  break;
               case GIF_PLAINTEXT_EXTENSION:
                  printf("Gif PlainText Extension not supported !\n");
                  break;
                  
               default:
                  gif_error=GIF_ERROR_UNKNOW_EXTENSION;
                  delete_gif(p_gif);
                  if (current_control)
                     free((void*)current_control);
									return nullptr;
            }
            if (size < pos + 1 || stream[pos++])
            {
               gif_error=GIF_ERROR_SIZE;
               delete_gif(p_gif);
               if (current_control)
                  free((void*)current_control);
							 return nullptr;
            }
            break;
            
         case GIF_TRAILER:
            break;
           
         default:
            gif_error=GIF_ERROR_UNKNOW_BLOCK;
            printf("block_type 0x%X\n",block_type);
            delete_gif(p_gif);
            if (current_control)
               free((void*)current_control);
						return nullptr;
      }
   }
   if (current_control)
      free((void*)current_control);
   return p_gif;
}

int gif_draw_frame(uint8_t* screen,
                   const gif* g, 
                   unsigned int n,
                   uint8_t* previous,
                   int mode)
{
   uint8_t table[256*4]; /* rgba table mode */
   const uint16_t sw = g->screen_descriptor.width;
   const uint16_t sh = g->screen_descriptor.height;
   uint16_t start_x;
   uint16_t x;
   uint16_t start_y;
   uint16_t y;
   uint16_t w;
   uint16_t h;
   uint32_t p;
   uint32_t pt;
   uint32_t skip;
   uint16_t transparent_index =0xFFFF;
   uint8_t method=0;
   
   gif_image* f;
   if (!g) {
			return GIF_ERROR_NULL;
   }
   else if (g->images_count <= n) { /*Problem ?*/
      return GIF_ERROR_BAD_INDEX; /*Solution !*/
   }
   
   if (g->images && g->images[n]) {
      f=g->images[n];
      start_x=f->descriptor.left;
      start_y=f->descriptor.top;
      w=f->descriptor.width;
      h=f->descriptor.height;
   } else {
			return GIF_ERROR_NULL;
   }
   
   /* convert table to rgba */
   if (f->local_color_table_size && f->local_color_table) {
      const unsigned int size = f->local_color_table_size << 2;
      unsigned i=0,j=0;
      for (;i<size;)
      {
         table[i++]=f->local_color_table[j++];
         table[i++]=f->local_color_table[j++];
         table[i++]=f->local_color_table[j++];
         table[i++]=255;
      }
   }
   else if (g->global_color_table_size && g->global_color_table) {
      const unsigned int size = g->global_color_table_size << 2;
      unsigned i=0,j=0;
      for (;i<size;)
      {
         table[i++]=g->global_color_table[j++];
         table[i++]=g->global_color_table[j++];
         table[i++]=g->global_color_table[j++];
         table[i++]=255;
      }
   }
   else {
			return GIF_ERROR_NULL;
   }
   
   
   if (f->control)
   {
      /* get the transparent color */
      if (f->control->transparent_flag)
      {
         table[f->control->transparent_color_index*4+3]=0;
         transparent_index = f->control->transparent_color_index;
      }
      method=f->control->disposal_method;
   }
   
   if (!method)
   {
      if (previous && screen)
         memcpy(screen,previous,sw*sh*4);
      else 
				 return GIF_ERROR_NULL;
      
   }
   if (mode == GIF_MODE_BITMAP)
   {
      p = ((sw*sh - (start_y*sw+start_x)) << 2) - 1;
      skip = (sw-w) << 2;
      for (y=0;y < h; y++)
      {
         for (x=w ; x > 0; x--)
         {
            pt = f->pixels[y*w+x-1];
            if (pt != transparent_index)
            {
               pt = pt << 2;
               screen[p--] = table[pt+3];
               screen[p--] = table[pt];
               screen[p--] = table[pt+1];
               screen[p--] = table[pt+2];
            }
            else
            {
               p-=4;
            }
         }
         p-=skip;
      }
   }
   else
   {
      p = (start_y*sw+start_x) << 2;
      skip = (sw-w) << 2;
      for (y=0;y < h; y++)
      {
         for (x=0 ; x < w; x++)
         {
            pt = f->pixels[y*w+x];
            if (pt != transparent_index)
            {
               pt <<= 2;
               screen[p++] = table[pt++];
               screen[p++] = table[pt++];
               screen[p++] = table[pt++];
               screen[p++] = table[pt];
            }
            else
            {
               p+=4;
            }
         }
         p+=skip;
      }
   }
   
   
   
   if (!method)
   {
      if (previous && screen)
         memcpy(previous,screen,sw*sh*4);
      else 
				 return GIF_ERROR_NULL;
   }
   return GIF_NO_ERROR;
}
