
/* $Id: main.c 8 2008-02-16 00:16:49Z jessekornblum $ */

#include "main.h"

void usage(void)
{
  print_status("%s version %s by %s", __progname, VERSION, AUTHOR);

  print_status ("Usage: %s [-Vh] [-rablv] [-s|-S len] [FILES]%s",__progname,NEWLINE);

  print_status ("-r  Recursive mode. All subdirectories are traversed");
  print_status ("-q  Silent mode. No error messages are displayed");
  print_status ("-a  Display all executable files regardless of extension");
  print_status ("-b  Bare filename. No path information displayed");
  print_status ("-l  Relative paths in filenames");
  print_status ("-v  Verbose mode. Displays the filename for every %"PRIu64"%s file processed",
		(uint64_t)VERBOSE_DISPLAY_NUM,
		suffix((uint64_t)VERBOSE_DISPLAY_NUM));
  print_status ("-s|-S Display strings");

  print_status ("-V  Display version number and exit");
  print_status ("-h  Display this help message");
}


void try_msg(void)
{
  fprintf (stderr,"Try `%s -h` for more information.%s", __progname, NEWLINE);
}


int process_cmd_line(state *s, int argc, char **argv)
{
  int i;
  uint16_t len;

  while ((i=getopt(argc,argv,"rqas:S:lbvhV")) != -1) {
    switch(i) {

    case 'r':
      s->mode |= mode_recursive; break;

    case 'q':
      s->mode |= mode_silent; break;

    case 'a':
      s->mode |= mode_all_executables; break;

    case 'S':
      s->mode |= mode_strings_first;
      // Note no break here
    case 's':
      len = (uint16_t)atol(optarg);
      if (len == 0)
	fatal_error("Invalid string length");

      s->strings = (unsigned char *)malloc(sizeof(char) * (len + 1));
      if (NULL == s->strings)
	fatal_error("%s: Out of memory", __progname);
      
      s->string_len = (uint8_t)len;
      s->mode |= mode_strings;
      break;

    case 'l':
      s->mode |= mode_relative; break;

    case 'b':
      s->mode |= mode_barename; break;

    case 'v':
      s->mode |= mode_verbose; break;

    case 'h':      
      usage();
      exit(EXIT_SUCCESS);

    case 'V':
      printf ("%s%s", VERSION, NEWLINE);
      exit(EXIT_SUCCESS);
      
    default:
      try_msg();
      exit(EXIT_FAILURE);
    }
  }

  return FALSE;
}




int initialize_state(state *s)
{
  s->mode = mode_none;

  s->buf = (unsigned char *)malloc(MISS_BUFSIZE);
  if (NULL == s->buf)
    return TRUE;

  s->files_processed = 0;

  MD5DEEP_ALLOC(TCHAR,s->short_name,(LINE_LENGTH * sizeof(TCHAR)));
  MD5DEEP_ALLOC(TCHAR,s->msg,(LINE_LENGTH * sizeof(TCHAR)));
  
  return FALSE;
}

#ifdef _WIN32
static int prepare_windows_command_line(state *s)
{
  int argc;
  TCHAR **argv;

  argv = CommandLineToArgvW(GetCommandLineW(),&argc);
  
  s->argc = argc;
  s->argv = argv;

  return FALSE;
}
#endif



static int is_absolute_path(TCHAR *fn)
{
  if (NULL == fn)
    internal_error("Unknown error in is_absolute_path");
  
#ifdef _WIN32
  return FALSE;
#endif

  return (DIR_SEPARATOR == fn[0]);
}



void generate_filename(state *s, TCHAR *fn, TCHAR *cwd, TCHAR *input)
{
  if (NULL == fn || NULL == input)
    internal_error("Error calling generate_filename");

  if ((s->mode & mode_relative) || is_absolute_path(input))
    _tcsncpy(fn,input,PATH_MAX);
  else
    {
      /* Windows systems don't have symbolic links, so we don't
       have to worry about carefully preserving the paths
       they follow. Just use the system command to resolve the paths */   
#ifdef _WIN32
      _wfullpath(fn,input,PATH_MAX);
#else     
      if (NULL == cwd)
	/* If we can't get the current working directory, we're not
         going to be able to build the relative path to this file anyway.
         So we just call realpath and make the best of things */
	realpath(input,fn);
      else
	snprintf(fn,PATH_MAX,"%s%c%s",cwd,DIR_SEPARATOR,input);
#endif
    }
}






int main(int argc, char **argv)
{
  int count, status;
  state *s;
  TCHAR *fn, *cwd;

#ifndef __GLIBC__
  __progname  = basename(argv[0]);
#endif

  s = (state *)malloc(sizeof(state));
  // We can't use fatal_error because it requires a valid state
  if (NULL == s)
    fatal_error("%s: Unable to allocate state variable", __progname);

  if (initialize_state(s))
    fatal_error("%s: Unable to initialize state variable", __progname);

  process_cmd_line(s,argc,argv);

#ifdef _WIN32
  if (prepare_windows_command_line(s))
    fatal_error("%s: Unable to process command line arguments", __progname);
#else
  s->argc = argc;
  s->argv = argv;
#endif

  /* Anything left on the command line at this point is a file
     or directory we're supposed to process. If there's nothing
     specified, we should tackle standard input */
  if (optind == argc)
    fatal_error("Processing stdin is not supported in this version");
  else
    {
      MD5DEEP_ALLOC(TCHAR,fn,PATH_MAX);
      MD5DEEP_ALLOC(TCHAR,cwd,PATH_MAX);

      cwd = _tgetcwd(cwd,PATH_MAX);
      if (NULL == cwd)
	fatal_error("%s: %s", __progname, strerror(errno));

      count = optind;

      while (count < s->argc)
	{  
	  generate_filename(s,fn,cwd,s->argv[count]);

#ifdef _WIN32
	  status = process_win32(s,fn);
#else
	  status = process_normal(s,fn);
#endif

	  //      if (status != STATUS_OK)
	  //        return status;

	  ++count;
	}

      free(fn);
      free(cwd);
    }



  return (EXIT_SUCCESS);
}
