/* ldmisc.c
   Copyright (C) 1991 Free Software Foundation, Inc.

   Written by Steve Chamberlain of Cygnus Support.

This file is part of GLD, the Gnu Linker.

GLD is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GLD is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GLD; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
$Id$ 


 */

#include "bfd.h"
#include "sysdep.h"
#include <varargs.h>

#include "ld.h"
#include "ldmisc.h"
#include "ldlang.h"
#include "ldlex.h"
/* IMPORTS */

extern char *program_name;

extern FILE *ldlex_input_stack;
extern char *ldfile_input_filename;
extern ld_config_type config;

void
yyerror(arg) 
char *arg;
{ 
  einfo("%P%F: %S %s\n",arg);
}

extern int errno;
extern   int  sys_nerr;
extern char *sys_errlist[];

/*
 %F error is fatal
 %P print progam name
 %S print script file and linenumber
 %E current bfd error or errno
 %I filename from a lang_input_statement_type
 %B filename from a bfd
 %T symbol table entry
 %X no object output, fail return
 %V hex bfd_vma
 %C Clever filename:linenumber 
 %R info about a relent
 %
*/
static void
vfinfo(fp, fmt, arg)
     FILE *fp;
     char *fmt;
     va_list arg;
{
  boolean fatal = false;
  while (*fmt) {
    while (*fmt != '%' && *fmt != '\0') {
      putc(*fmt, fp);
      fmt++;
    }
    if (*fmt == '%') {
      fmt ++;
      switch (*fmt++) {
      case 'X':
	config.make_executable = false;
	break;
      case 'V':
	  {
	    bfd_vma value = va_arg(arg, bfd_vma);
	    fprintf_vma(fp, value);
	  }
	break;
      case 'T':
	{
	  asymbol *symbol = va_arg(arg, asymbol *);
	  if (symbol) 
	  {
	    asection *section = symbol->section;
	    CONST char *section_name =  section->name;
	    fprintf(fp,"%s (%s)", symbol->name, section_name);
	  }
	  else 
	  {
	    fprintf(fp,"no symbol");
	  }
	}
	break;
      case 'B':
	{ 
	  bfd *abfd = va_arg(arg, bfd *);
	  if (abfd->my_archive) {
	    fprintf(fp,"%s(%s)", abfd->my_archive->filename,
		    abfd->filename);
	  }
	  else {
	    fprintf(fp,"%s", abfd->filename);

	  }
	}
	break;
      case 'F':
	fatal = true;
	break;
      case 'P':
	fprintf(fp,"%s", program_name);
	break;
      case 'E':
	/* Replace with the most recent errno explanation */


	fprintf(fp, bfd_errmsg(bfd_error));


	break;
      case 'I':
	{
	  lang_input_statement_type *i =
	    va_arg(arg,lang_input_statement_type *);
	
	  fprintf(fp,"%s", i->local_sym_name);
	}
	break;
      case 'S':
	/* Print source script file and line number */

	if (ldlex_input_stack) {
	  extern unsigned int lineno;
	  if (ldfile_input_filename == (char *)NULL) {
	    fprintf(fp,"command line");
	  }
	  else {
	    fprintf(fp,"%s:%u", ldfile_input_filename, lineno );
	  }
	}
	else {
	  int ch;
	  int n = 0;
	  fprintf(fp,"command (just before \"");
	  ch = lex_input();
	  while (ch != 0 && n < 10) {
	    fprintf(fp, "%c", ch);
	    ch = lex_input();
	    n++;
	  }
	  fprintf(fp,"\")");
	    
	}
	break;

      case 'R':
	/* Print all that's interesting about a relent */
      {
	arelent *relent = va_arg(arg, arelent *);
	
	fprintf(fp,"%s+0x%x (type %s)",
		(*(relent->sym_ptr_ptr))->name,
		relent->addend,
		relent->howto->name);
	

      }
	break;
	


	
      case 'C':
	{
	 CONST char *filename;
	 CONST char *functionname;
	  unsigned int linenumber;
	  bfd *abfd = va_arg(arg, bfd *);
	  asection *section = va_arg(arg, asection *);
	  asymbol **symbols = va_arg(arg, asymbol **);
	  bfd_vma offset = va_arg(arg, bfd_vma);
	 
	  if (bfd_find_nearest_line(abfd,
				    section,
				    symbols,
				    offset,
				    &filename,
				    &functionname,
				    &linenumber))
	    {
		if (filename == (char *)NULL) 	
		    filename = abfd->filename;
		if (functionname != (char *)NULL)
		    fprintf(fp,"%s:%u: (%s)", filename, linenumber,  functionname);
		else if (linenumber != 0) 
		    fprintf(fp,"%s:%u", filename, linenumber);
		else
		    fprintf(fp,"%s(%s+%0x)", filename,
			    section->name,
			    offset);

	    }
	  else {
	    fprintf(fp,"%s(%s+%0x)", abfd->filename,
		    section->name,
		    offset);
	  }
	}
	break;
		
      case 's':
	fprintf(fp,"%s", va_arg(arg, char *));
	break;
      case 'd':
	fprintf(fp,"%d", va_arg(arg, int));
	break;
      default:
	fprintf(fp,"%s", va_arg(arg, char *));
	break;
      }
    }
  }
  if (fatal == true) {
    extern char *output_filename;
    if (output_filename)
      unlink(output_filename);
    exit(1);
  }
}

/* Format info message and print on stdout. */

void info(va_alist)
va_dcl
{
  char *fmt;
  va_list arg;
  va_start(arg);
  fmt = va_arg(arg, char *);
  vfinfo(stdout, fmt, arg);
  va_end(arg);
}

/* ('e' for error.) Format info message and print on stderr. */

void einfo(va_alist)
va_dcl
{
  char *fmt;
  va_list arg;
  va_start(arg);
  fmt = va_arg(arg, char *);
  vfinfo(stderr, fmt, arg);
  va_end(arg);
}

void 
info_assert(file, line)
char *file;
unsigned int line;
{
  einfo("%F%P internal error %s %d\n", file,line);
}

/* Return a newly-allocated string
   whose contents concatenate those of S1, S2, S3.  */

char *
DEFUN(concat, (s1, s2, s3),
      CONST char *s1 AND
      CONST char *s2 AND
      CONST char *s3)
{
  bfd_size_type len1 = strlen (s1);
  bfd_size_type len2 = strlen (s2);
  bfd_size_type len3 = strlen (s3);
  char *result = ldmalloc (len1 + len2 + len3 + 1);

  if (len1 != 0)
    memcpy(result, s1, len1);
  if (len2 != 0)
    memcpy(result+len1, s2, len2);
  if (len3 != 0)
    memcpy(result+len1+len2, s2, len3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}



PTR
DEFUN(ldmalloc, (size),
bfd_size_type size)
{
  PTR result =  malloc ((int)size);

  if (result == (char *)NULL && size != 0)
    einfo("%F%P virtual memory exhausted\n");

  return result;
} 



char *DEFUN(buystring,(x),
	    CONST char *CONST x)
{
  bfd_size_type  l = strlen(x)+1;
  char *r = ldmalloc(l);
  memcpy(r, x,l);
  return r;
}


/*----------------------------------------------------------------------
  Functions to print the link map 
 */

void 
DEFUN_VOID(print_space)
{
  printf(" ");
}
void 
DEFUN_VOID(print_nl)
{
  printf("\n");
}
void 
DEFUN(print_address,(value),
      bfd_vma value)
{
  printf_vma(value);
}
