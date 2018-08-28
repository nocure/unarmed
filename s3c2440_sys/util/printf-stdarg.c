/******************************************************************************
 * printf-stdarg.c
 * ============================================================================
 *
 *
 * 2008-0915 ww added: convert \n to \r\n
 *
 *
 *(\~~/)(\~~/)
 *('.' )(^.^ )
 * (_(__ (_(__)~*
 *****************************************************************************/

#include <stdarg.h>
#include <sct.h>
#include "platform/yl_sys.h"


static void printchar(char **str, int c) {

	if (str) {
		**str = c;
		++(*str);
	}
	else {
		putch(c);	// ww 2014-0121 putch is modified to interpret \n as \r\n
	}
}

#if 0
//****************************************************************************
//  This version returns the length of the output string.
//  It is more useful when implementing a walking-string function.
//****************************************************************************
static double round_nums[8] = {
   0.5,
   0.05,
   0.005,
   0.0005,
   0.00005,
   0.000005,
   0.0000005,
   0.00000005
} ;
#endif


static unsigned dbl2stri(char *outbfr, double dbl, int dec_digits, int shortest) {
#if defined(LOCAL_BUFFER)
   static char local_bfr[81] ;
   char *output = (outbfr == 0) ? local_bfr : outbfr ;
#else
   char *output = outbfr;
#endif
   double round_num;
   DWORD mult;
   DWORD idx ;
   QWORD wholeNum;
   QWORD decimalNum;
   char tbfr[21];	// 20 +1 null should be enough for 64bit 0xffffffffffffffff -> 18446744073709551615

   if (dec_digits<0) dec_digits=4;		// 2014-0123 decimal digits is 4 if not designated.
   else if ((dec_digits>9)||(shortest&&(dec_digits<0))) dec_digits=9;	// 2013-1231 ww max dec digits is 9.
   //*******************************************
   //  extract negative info
   //*******************************************
   if (dbl < 0.0) {
      *output++ = '-' ;
      dbl *= -1.0 ;
   }

#if 0
   //  handling rounding by adding .5LSB to the floating-point data
   if (dec_digits < 8) {
      dbl += round_nums[dec_digits] ;
   }
#endif

   //**************************************************************************
   //  construct fractional multiplier for specified number of digits.
   //**************************************************************************
   round_num=0.5;
   mult = 1 ;
   for (idx=0; idx < dec_digits; idx++) {
	      mult *= 10 ;
	      round_num/=10;	// 2013-1231 ww calculate rounding .5LSB
   }
   //  handling rounding by adding .5LSB to the floating-point data
   dbl+=round_num;

   // printf("mult=%u\n", mult) ;
   wholeNum = (QWORD) dbl ;
   if ((wholeNum+1)==0) {
	   strcpy(outbfr,"OVERFLOW");
	   goto out;
   }
   decimalNum = (QWORD) ((dbl-wholeNum) * mult);

   if (shortest) {	// remove tailing zeros of decimal number
	   idx=0;
	   if (decimalNum) {
		   for (;idx<dec_digits;idx++) {
			   round_num=(double)decimalNum/mult;
			   if ((round_num-(DWORD)round_num)==0) break;
			   mult/=10;
		   }
		   decimalNum/=mult;
	   }
	   dec_digits=idx;
   }

   //*******************************************
   //  convert integer portion
   //*******************************************
   idx = 0 ;
   while (wholeNum != 0) {
      tbfr[idx++] = '0' + (wholeNum % 10) ;
      wholeNum /= 10 ;
   }
   // printf("%.3f: whole=%s, dec=%d\n", dbl, tbfr, decimalNum) ;
   if (idx == 0) {
      *output++ = '0' ;
   } else {
      while (idx > 0) {
         *output++ = tbfr[idx-1] ;  //lint !e771
         idx-- ;
      }
   }
   if (dec_digits > 0) {
      *output++ = '.' ;

      //*******************************************
      //  convert fractional portion
      //*******************************************
      idx = 0 ;
      while (decimalNum != 0) {
         tbfr[idx++] = '0' + (decimalNum % 10) ;
         decimalNum /= 10 ;
      }
      //  pad the decimal portion with 0s as necessary;
      //  We wouldn't want to report 3.093 as 3.93, would we??
      while (idx < dec_digits) {
         tbfr[idx++] = '0' ;
      }
      // printf("decimal=%s\n", tbfr) ;
      if (idx == 0) {
         *output++ = '0' ;
      }
      else {
         while (idx > 0) {
            *output++ = tbfr[idx-1] ;
            idx-- ;
         }
      }
   }
   *output = 0 ;

out:
   //  prepare output
#if defined(LOCAL_BUFFER)
   output = (outbfr == 0) ? local_bfr : outbfr ;
#else
   output = outbfr;
#endif
   return strlen(output) ;
}

//****************************************************************************
#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if ((pad & PAD_RIGHT)||(pad & PAD_ZERO)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}


int yl_vprintf(char **out, const char *format, va_list args ) {
register int width, pad;
register int dec_width = 0 ;
register int pc = 0;
char strbuf[42];	// 41 +1 null should be enough for the max integral & fractional number.
// 0xffffffffffffffff.0xffffffffffffffff -> 18446744073709551615.18446744073709551615
register char *s;


	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			dec_width=-1;
			if (*format == '.' || (*format >= '0' && *format <= '9')) {
				register int post_decimal=0;
				while (1) {
					if (*format == '.') {
						post_decimal = 1;
						dec_width = 0;
						format++;
					}
					else if ((*format >= '0' && *format <= '9')) {
						if (post_decimal) {
							dec_width *= 10;
							dec_width += *format - '0';
						}
						else {
							width *= 10;
							width += *format - '0';
						}
						format++;
					}
					else break;
				}
			}
			if (*format == 'l') ++format;

			switch (*format) {
				case 's': {
					s = (char *)va_arg( args, int );
					pc += prints (out, s?s:"(null)", width, pad);
				} break;

				case 'd':
				case 'i':
					pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
					break;

				case 'x':
					pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
					break;

				case 'X':
					pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
					break;

				case 'p':
					pc += prints (out, "0x", 0, 0);
					pc += printi (out, va_arg( args, int ), 16, 0, 8, PAD_ZERO, 'a');
					break;

				case 'u':
					pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
					break;

				case 'c':
					/* char are converted to int then pushed on the stack */
					strbuf[0] = (char)va_arg( args, int );
					strbuf[1] = '\0';
					pc += prints (out, strbuf, width, pad);
					break;

				case 'g':
					dbl2stri(strbuf, va_arg( args, double ), dec_width, 1) ;
					pc += prints (out, strbuf, width, pad);
					break;

				case 'f':
					dbl2stri(strbuf, va_arg( args, double ), dec_width, 0) ;
					pc += prints (out, strbuf, width, pad);
					break;

				default:
					printchar(out, '%');
					printchar(out, *format);
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

//int printf(const char *format, ...)
int yl_uartPrintf(const char *format, ...) {
va_list args;
register int c;

	wai_sem(SEMID_UARTPRINTF);
	va_start( args, format );
	c=yl_vprintf( 0, format, args );
	va_end( args );
	sig_sem(SEMID_UARTPRINTF);
	return c;
}

int yl_sprintf(char *out, const char *format, ...) {
va_list args;
register int c;

    va_start( args, format );
    c=yl_vprintf( &out, format, args );
	va_end( args );
	return c;
}



#ifdef TEST_PRINTF
int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	return 0;
}

/*
 * if you compile this file with
 *   gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 */

#endif
