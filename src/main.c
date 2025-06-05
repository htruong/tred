#include <dos.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>

#include "vscreen.h"
#include "vncvt.h"
#include "vgatxt.h"

enum p_state {
   P_NORMAL,
   P_EXPECT_ONE_CONT_B,
   P_EXPECT_TWO_CONT_B,
   P_EXPECT_THREE_CONT_B,
   P_INVALID
};

void consume(unsigned char c, enum p_state *s, unsigned int * out) {
   if (*s == P_NORMAL) {
      if (c < 128) {
         //printf("=a=");
         *out = c;
      } else if ( (c & 0xf8) == 0xf0) {
         //printf("=3=");
         *out = (unsigned int) (c & 0x07);
         *s = P_EXPECT_THREE_CONT_B;
      } else if ( (c & 0xf0) == 0xe0) {
         //printf("=2=");
         *out = (unsigned int) (c & 0x0f);
         *s = P_EXPECT_TWO_CONT_B;
      } else if ( (c & 0xe0) == 0xc0) {
         //printf("=1=");
         *out = (unsigned int) (c & 0x1f);
         *s = P_EXPECT_ONE_CONT_B;
      } else {
         //printf("=i=");
         *s = P_INVALID;
      }
   } else if ((*s == P_EXPECT_THREE_CONT_B) || (*s == P_EXPECT_TWO_CONT_B) || (*s == P_EXPECT_ONE_CONT_B) ) {
      if ( (c & 0xc0) == 0x80 ) {
         //printf("=c=");
         *out = (*out << 6) + (c & 0x3f);
         *s = *s - 1;
      } else {
         //printf("=e=");
         *s = P_INVALID;
      }
   } else {
      //printf("=?=");
      *s = P_INVALID;
   }
}

int display_file(char * filename) {
   FILE *fptr;
   static char buff[80*25];
   static short buff_cp437e[80*25*2 + 1];
   unsigned short buff_cp437_cur;
   int cur;
   enum p_state utf8_parser_state;
   unsigned int cur_wchar;
   char schar;

   virtual_screen *vs = vscreen_init(80, 23);

   vscreen_set_cursor(vs, 0, 0);

   fptr = fopen(filename,"r");

   if (fptr == NULL) {
      printf("Cannot open file for reading.\n");
      return -1;
   }

   utf8_parser_state = P_NORMAL;
   while ( fgets( buff, 80*25, fptr ) != NULL ) {
      buff_cp437_cur = 0;
      for (cur = 0; cur < strlen(buff); cur++) {
         consume(buff[cur], &utf8_parser_state, &cur_wchar);
         if (utf8_parser_state == P_INVALID) {
            fclose(fptr);
            printf("Error occured while parsing the file at char %c (0x%x) position %d.\n", buff[cur], buff[cur], cur);
            return -1;
         } else if (utf8_parser_state == P_NORMAL) {
            if (cur_wchar != '\n') {
               buff_cp437e[buff_cp437_cur] = unicode_to_cp437e(cur_wchar);
               buff_cp437_cur++;
            } else if (cur_wchar == '\r') {
               // Ignore the \r
            } else {
               buff_cp437e[buff_cp437_cur] = '\r';
               buff_cp437_cur++;
               buff_cp437e[buff_cp437_cur] = '\n';
               buff_cp437_cur++;
            }
            //putch((char) unicode_to_cp437e(cur_wchar));
         }
      }
      buff_cp437e[buff_cp437_cur] = 0;
      //puttexte(buff_cp437e, buff_cp437_cur);
      vscreen_puts(vs, buff_cp437e, buff_cp437_cur);
      vscreen_flush(vs);
   }
   fclose(fptr);
   vscreen_free(vs);
}


int main(int argc, char **argv) {
   if (argc != 2) {
      printf("No file name given.\n");
      return 1;
   }
   
   setup_vietnamese_font(0);
   display_file(argv[1]);
   return 0;
}
