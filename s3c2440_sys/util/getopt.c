/*
  Parse the command line options, System V style.

  Standard option syntax is:

    option ::= SW [optLetter]* [argLetter space* argument]

  where
    - SW is either '/' or '-', according to the current setting
      of the MSDOS switchar (int 21h function 37h).
    - there is no space before any optLetter or argLetter.
    - opt/arg letters are alphabetic, not punctuation characters.
    - optLetters, if present, must be matched in optionS.
    - argLetters, if present, are found in optionS followed by ':'.
    - argument is any white-space delimited string.  Note that it
      can include the SW character.
    - upper and lower case letters are distinct.

  There may be multiple option clusters on a command line, each
  beginning with a SW, but all must appear before any non-option
  arguments (arguments not introduced by SW).  Opt/arg letters may
  be repeated: it is up to the caller to decide if that is an error.

  The character SW appearing alone as the last argument is an error.
  The lead-in sequence SWSW ("--" or "//") causes itself and all the
  rest of the line to be ignored (allowing non-options which begin
  with the switch char).

  The string *optionS allows valid opt/arg letters to be recognized.
  argLetters are followed with ':'.  Getopt () returns the value of
  the option character found, or EOF if no more options are in the
  command line.  If option is an argLetter then the global yl_optarg is
  set to point to the argument string (having skipped any white-space).

  The global yl_optind is initially 1 and is always left as the index
  of the next argument of argv[] which getopt has not taken.  Note
  that if "--" or "//" are used then yl_optind is stepped to the next
  argument before getopt() returns EOF.

  If an error occurs, that is an SW char precedes an unknown letter,
  then getopt() will return a '?' character and normally prints an
  error message via perror().  If the global variable opterr is set
  to false (zero) before calling getopt() then the error message is
  not printed.

  For example, if the MSDOS switch char is '/' (the MSDOS norm) and

    *optionS == "A:F:PuU:wXZ:"

  then 'P', 'u', 'w', and 'X' are option letters and 'F', 'U', 'Z'
  are followed by arguments.  A valid command line may be:

    aCommand  /uPFPi /X /A L someFile

  where:
    - 'u' and 'P' will be returned as isolated option letters.
    - 'F' will return with "Pi" as its argument string.
    - 'X' is an isolated option.
    - 'A' will return with "L" as its argument.
    - "someFile" is not an option, and terminates getOpt.  The
      caller may collect remaining arguments using argv pointers.
*/

#include "platform/YL_SYS.h"
#include <string.h>

int yl_optind;         /* index of which argument is next      */
char *yl_optarg;         /* pointer to argument of current option */

static char *letP;    /* remember next option char's location */
static const char SW = '-';         /* DOS switch character, either '-' or '/' */

int yl_getopt(int argc, char **argv, char *optionS) {
	unsigned char ch;
	char *optP;

	if (argc==0) {
		letP=NULL;
		yl_optind=1;
		return 0;
	}

	if (argc > yl_optind) {
		if (letP == NULL) {
			if ((letP = argv[yl_optind]) == NULL ||
				*(letP++) != SW)  goto gopEOF;
			if (*letP == SW) {
				yl_optind++;  goto gopEOF;
			}
		}
		if (0 == (ch = *(letP++))) {
			yl_optind++;  goto gopEOF;
		}
		if (':' == ch  ||  (optP = strchr(optionS, ch)) == NULL)
			goto gopError;
		if (':' == *(++optP)) {
			yl_optind++;
			if (0 == *letP) {
				if (argc <= yl_optind)  goto  gopError;
				letP = argv[yl_optind++];
			}
			yl_optarg = letP;
			letP = NULL;
		} else {
			if (0 == *letP) {
				yl_optind++;
				letP = NULL;
			}
			yl_optarg = NULL;
		}
		return ch;
	}
gopEOF:
	yl_optarg = letP = NULL;
	return -1;

gopError:
	yl_optarg = NULL;
	printf("Unknown switch or option needs an argument (%s).\n",argv[yl_optind]);
	return ('?');
}

