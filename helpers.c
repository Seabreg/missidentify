
/* MD5DEEP - helpers.c
 *
 * By Jesse Kornblum
 *
 * This is a work of the US Government. In accordance with 17 USC 105,
 * copyright protection is not available for any work of the US Government.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/* $Id: helpers.c 8 2008-02-16 00:16:49Z jessekornblum $ */

#include "main.h"

/*
void make_newline(state *s)
{
  if (s->mode & mode_zero)
    printf("%c", 0);
  else
    printf("%s", NEWLINE);
  fflush(stdout);
}
*/

char * suffix(uint64_t n)
{
  switch (n % 10)
    {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    }
  return "th";
}  


void shorten_filename(TCHAR *dest, TCHAR *src)
{
  TCHAR *basen;

  if (_tcslen(src) < MAX_FILENAME_LENGTH)
    {
      _tcsncpy(dest,src, MAX_FILENAME_LENGTH);
      return;
    }

  basen = _tcsdup(src);
  if (NULL == basen)
    return;

  my_basename(basen);  

  if (_tcslen(basen) < MAX_FILENAME_LENGTH)
    {
      _tcsncpy(dest,basen,MAX_FILENAME_LENGTH);
      return;
    }

  basen[MAX_FILENAME_LENGTH - 3] = 0;
  _sntprintf(dest,MAX_FILENAME_LENGTH,_TEXT("%s..."),basen);
  free(basen);
}






/* Shift the contents of a string so that the values after 'new_start'
   will now begin at location 'start' */
void shift_string(char *fn, size_t start, size_t new_start)
{
  if (start > strlen(fn) || new_start < start)
    return;

  while (new_start < strlen(fn))
    {
      fn[start] = fn[new_start];
      new_start++;
      start++;
    }

  fn[start] = 0;
}


/* Find the index of the next comma in the string s starting at index start.
   If there is no next comma, returns -1. */
int find_next_comma(char *s, unsigned int start)
{
  size_t size=strlen(s);
  unsigned int pos = start; 
  int in_quote = FALSE;
  
  while (pos < size)
  {
    switch (s[pos]) {
    case '"':
      in_quote = !in_quote;
      break;
    case ',':
      if (in_quote)
	break;

      /* Although it's potentially unwise to cast an unsigned int back
	 to an int, problems will only occur when the value is beyond 
	 the range of int. Because we're working with the index of a 
	 string that is probably less than 32,000 characters, we should
	 be okay. */
      return (int)pos;
    }
    ++pos;
  }
  return -1;
}

 
/* Returns the string after the nth comma in the string s. If that
   string is quoted, the quotes are removed. If there is no valid 
   string to be found, returns TRUE. Otherwise, returns FALSE */
int find_comma_separated_string(char *s, unsigned int n)
{
  int start = 0, end;
  unsigned int count = 0; 
  while (count < n)
  {
    if ((start = find_next_comma(s,start)) == -1)
      return TRUE;
    ++count;
    // Advance the pointer past the current comma
    ++start;
  }

  /* It's okay if there is no next comma, it just means that this is
     the last comma separated value in the string */
  if ((end = find_next_comma(s,start)) == -1)
    end = strlen(s);

  /* Strip off the quotation marks, if necessary. We don't have to worry
     about uneven quotation marks (i.e quotes at the start but not the end
     as they are handled by the the find_next_comma function. */
  if (s[start] == '"')
    ++start;
  if (s[end - 1] == '"')
    end--;

  s[end] = 0;
  shift_string(s,0,start);
  
  return FALSE;
}


/* The basename function kept misbehaving on OS X, so I rewrote it.
   This function isn't perfect, nor is it designed to be. Because
   we're guarenteed to be working with a file here, there's no way
   that s will end with a DIR_SEPARATOR (e.g. /foo/bar/). This function
   will not work properly for a string that ends in a DIR_SEPARATOR */
int my_basename(TCHAR *s)
{
  size_t len;
  TCHAR *tmp = _tcsrchr(s,DIR_SEPARATOR);

  if (NULL == tmp)
    return FALSE;

  len = _tcslen(tmp);

  // We advance tmp one character to move us past the DIR_SEPARATOR
  _tmemmove(s,tmp+1,len);

  return FALSE;
}


int my_dirname(TCHAR *c)
{
  TCHAR *tmp;

  if (NULL == c)
    return TRUE;

  /* If there are no DIR_SEPARATORs in the directory name, then the 
     directory name should be the empty string */
  tmp = _tcsrchr(c,DIR_SEPARATOR);
  if (NULL != tmp)
    tmp[1] = 0;
  else
    c[0] = 0;

  return FALSE;
}
