
/* $Id: check.c 11 2008-02-19 22:25:30Z jessekornblum $ */

#include "main.h"

#ifdef WORDS_BIGENDIAN
#define BYTES_SWAP16(A)   A = bytes_swap16(A);

uint16_t bytes_swap16(uint16_t a)
{
  uint16_t tmp = a;
  return (tmp >> 8) + ((tmp & 0xff) << 8);
}

#define BYTES_SWAP32(A)  A = bytes_swap32(A);
uint32_t bytes_swap32(uint32_t a)
{
  uint32_t tmp = a;
  return (tmp >> 24) + ((tmp & 0xff0000) >> 8) + ((tmp & 0xff00) << 8) + 
    ((tmp & 0xff) << 24);
}
#endif // ifdef WORDS_BIGENDIAN


int is_executable_extension(TCHAR *str)
{
#define CHECK_EXEC(A)  if (!_tcsnicmp(A,str,4)) return TRUE;

  CHECK_EXEC(_TEXT(".exe"));
  CHECK_EXEC(_TEXT(".dll"));
  CHECK_EXEC(_TEXT(".com"));
  CHECK_EXEC(_TEXT(".sys"));
  CHECK_EXEC(_TEXT(".cpl"));
  CHECK_EXEC(_TEXT(".hxs"));
  CHECK_EXEC(_TEXT(".hxi"));
  CHECK_EXEC(_TEXT(".olb"));
  CHECK_EXEC(_TEXT(".rll"));
  CHECK_EXEC(_TEXT(".tlb"));

  return FALSE;
}


/* Determine if this buffer contains a valid PE header. At this point
   we know that the buffer starts with MZ and contans at least 0x3e bytes */
int check_buffer(state *s)
{
  uint16_t * pe_offset = (uint16_t *)(s->buf + 0x3c);

#ifdef WORDS_BIGENDIAN
  uint16_t tmp = *pe_offset;
  BYTES_SWAP16(tmp);
  pe_offset = &tmp;
#endif

  if (*pe_offset > s->bytes_read)
  {
    print_error_unicode(s,s->fn,"PE header is beyond the normal range. This is suspicious");
    return TRUE;
  }

  IMAGE_NT_HEADERS * h = (IMAGE_NT_HEADERS *)(s->buf + *pe_offset);
  uint32_t signature = h->Signature;

#ifdef WORDS_BIGENDIAN
  BYTES_SWAP32(signature);
#endif

  if (0x4550 != (uint16_t)signature)
    return TRUE;
  
  // We could add more checks here. Validate the section table, for example
  // We could also determine if this is a DLL or normal executable
 
  return FALSE;
}


#define MIN_STRING_LEN  4


static int compute_strings(state *s)
{
  int count, recording = FALSE;
  uint16_t inpos = 0, outpos = 0;

  while (outpos < s->string_len && inpos < s->bytes_read)
  {
    if (recording)
    {
      if (isprint(s->buf[inpos]))
      {
	s->strings[outpos] = s->buf[inpos];
	++outpos;
      }
      else
      {
	recording = FALSE;
	s->strings[outpos] = ',';
	++outpos;
      }
    }
    else 
    {
      count = 0;
      while (isprint(s->buf[inpos+count]) && count < MIN_STRING_LEN)
	++count;
      if (MIN_STRING_LEN == count)
      {
	recording = TRUE;
	s->strings[outpos] = s->buf[inpos];
	++outpos;
      }
    }

    ++inpos;
  }

  return FALSE;
}


static int display_result(state *s)
{
  if ( ! (s->mode & mode_strings))
  {
    display_filename(stdout,s->fn);
    print_status("");
    return FALSE;
  }

  memset(s->strings,0,s->string_len);

  compute_strings(s);

  if (s->mode & mode_strings_first)
    {
      printf("%s", s->strings);
      display_filename(stdout,s->fn);
      printf("\n");
    }
  else
    {
      display_filename(stdout,s->fn);
      printf("%s\n", s->strings);
    }

  return FALSE;
}


int check_file(state *s, TCHAR *fn)
{
  TCHAR *basen = NULL;

  if (s->mode & mode_barename)
  {
    basen = _tcsdup(fn);
    if (my_basename(basen))
    {
      free(basen);
      print_error_unicode(s,fn,"Illegal filename");
      return TRUE;
    }
    s->fn = basen;
  }
  else
    s->fn = fn;

  if (s->mode & mode_verbose)
    {
      s->files_processed++;
      if (0 == s->files_processed % VERBOSE_DISPLAY_NUM)
	{
	  shorten_filename(s->short_name,fn);	  
	  _sntprintf(s->msg,LINE_LENGTH-1,
		     _TEXT("%s%s"),
		     s->short_name,
		     _TEXT(BLANK_LINE));
	  fprintf(stderr,"\r");
	  display_filename(stderr,s->msg);
	}
    }

  
  s->handle = _tfopen(fn,_TEXT("rb"));
  if (NULL == s->handle)
  {
    if (s->mode & mode_barename)
      free(basen);
    return TRUE;
  }

  s->file_extension = fn + _tcslen(fn) - 1;
  while (s->file_extension[0] != _TEXT('.') && (s->file_extension != fn))
    --(s->file_extension);

  s->has_executable_extension = is_executable_extension(s->file_extension);

  s->bytes_read = fread(s->buf,1,MISS_BUFSIZE,s->handle);
  fclose(s->handle);

  if (s->bytes_read < 2)
  {
    /* There's nothing wrong with small files per se, 
       but they're too small to check for being executables */

    if (s->mode & mode_barename)
      free(basen);
    return TRUE;
  }
  
  uint16_t * mz_header = (uint16_t *)s->buf;  

#ifdef WORDS_BIGENDIAN
  uint16_t tmp = * mz_header;
  BYTES_SWAP16(tmp);
  mz_header = &tmp;
#endif

  if (0x5a4d == *mz_header)
  {
    // Not enough bytes read to find the offset to the PE header
    if (s->bytes_read < 0x3e)
      return FALSE;

    if ( ! check_buffer(s) )
    {
      if (s->mode & mode_all_executables)
      {
	display_result(s);
	if (s->mode & mode_barename)
	  free(basen);
	return FALSE;
      }

      if ( ! s->has_executable_extension )
	display_result(s);
    }
  }

	
  if (s->mode & mode_barename)
    free(basen);
  return FALSE;
}
