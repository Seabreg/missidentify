
/* -------------------------------------------------------------
   Miss Identify by Jesse Kornblum
   ------------------------------------------------------------- */

/* $Id: main.h 8 2008-02-16 00:16:49Z jessekornblum $ */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# error inttypes.h is required for this program
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#ifdef HAVE_FCNTL_H
// This will be required if/when we add AFF support
// #include <fcntl.h>
#endif

#include "tchar-local.h"

#define AUTHOR "Jesse Kornblum"

#define FALSE               0
#define TRUE                1
#define MISS_BUFSIZE   0x2000

#define MAX_FILENAME_LENGTH  LINE_LENGTH-41
#define VERBOSE_DISPLAY_NUM  10

#define BLANK_LINE  "                                                                                 "

#define MD5DEEP_ALLOC(TYPE,VAR,SIZE)     \
VAR = (TYPE *)malloc(sizeof(TYPE) * SIZE);  \
if (NULL == VAR)  \
   return EXIT_FAILURE; \
memset(VAR,0,SIZE * sizeof(TYPE));



#ifdef __GLIBC__
extern char *__progname;
#else
char *__progname;
#endif 

// Windows specific defines
#ifdef _WIN32

#define LINE_LENGTH 72
#define DIR_SEPARATOR '\\'
#define NEWLINE "\r\n"
char *basename(char *a);

/* We create macros for the Windows equivalent UNIX functions.
   No worries about lstat to stat; Windows doesn't have symbolic links */
#define lstat(A,B)      stat(A,B)
#define realpath(A,B)   _fullpath(B,A,PATH_MAX)

#else  // ifndef _WIN32

#define LINE_LENGTH 74
#define DIR_SEPARATOR '/'
#define NEWLINE "\n"

#endif


#define MIN(A,B)    (A<B)?A:B
#define MAX(A,B)    (A>B)?A:B

/* This is a special case of strings equal. We use the MINIMUM number of
   characters to make the match. We *want* the user to get their answer
   with the minimum number of letters. That way, the user can put in 
   "hashk" and have it match against "hashkeeper" */
#define MIN_STRINGS_EQUAL(A,B)   (!strncasecmp(A,B,MIN(strlen(A),strlen(B))))
#define STRINGS_EQUAL(A,B)       (!strncasecmp(A,B,MAX(strlen(A),strlen(B))))


/* The program's operation is controlled through modes.
   The mode is stored in a 64-bit value in the states. */
#define mode_none             0
#define mode_recursive        1<<1
#define mode_barename         1<<2
#define mode_relative         1<<3
#define mode_all_executables  1<<4
#define mode_silent           1<<5
#define mode_strings          1<<6
#define mode_strings_first    1<<7
#define mode_verbose          1<<8


/* Note that mode_expert (and its related modes) are not actually used
   in this program. The original code came from md5deep. */

#define mode_regular       1<<23
#define mode_directory     1<<24
#define mode_door          1<<25
#define mode_block         1<<26
#define mode_character     1<<27
#define mode_pipe          1<<28
#define mode_socket        1<<29
#define mode_symlink       1<<30
#define mode_expert        1<<31

/* These are the types of files we can encounter */
#define file_regular    0
#define file_directory  1
#define file_door       3
#define file_block      4
#define file_character  5
#define file_pipe       6
#define file_socket     7
#define file_symlink    8
#define file_unknown  254




#ifndef _WIN32

typedef struct _IMAGE_SECTION_HEADER {
  /* 000 */ unsigned char                Name[8];
  union {
    /* 008 */ uint32_t                   PhysicalAddress;
    /* 008 */ uint32_t                   VirtualSize;
  } Misc;
  /* 00c */ uint32_t                     VirtualAddress;
  /* 010 */ uint32_t                     SizeOfRawData;
  /* 014 */ uint32_t                     PointerToRawData;
  /* 018 */ uint32_t                     PointerToRelocations;
  /* 01c */ uint32_t                     PointerToLinenumbers;
  /* 020 */ uint16_t                     NumberOfRelocations;
  /* 022 */ uint16_t                     NumberOfLinenumbers;
  /* 024 */ uint32_t                     Characteristics;
  /* Size 028 */
} IMAGE_SECTION_HEADER;


typedef struct _IMAGE_DATA_DIRECTORY {
  /* 000 */ uint32_t                     VirtualAddress;
  /* 004 */ uint32_t                     Size;
  /* Size 008 */
} IMAGE_DATA_DIRECTORY;


typedef struct _IMAGE_FILE_HEADER {
  /* 000 */ uint16_t                     Machine;
  /* 002 */ uint16_t                     NumberOfSections;
  /* 004 */ uint32_t                     TimeDateStamp;
  /* 008 */ uint32_t                     PointerToSymbolTable;
  /* 00c */ uint32_t                     NumberOfSymbols;
  /* 010 */ uint16_t                     SizeOfOptionalHeader;
  /* 012 */ uint16_t                     Characteristics;
  /* Size 014 */
} IMAGE_FILE_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER {
  /* 000 */  uint16_t                    Magic;
  /* 002 */  unsigned char               MajorLinkerVersion;
  /* 003 */  unsigned char               MinorLinkerVersion;
  /* 004 */  uint32_t                    SizeOfCode;
  /* 008 */  uint32_t                    SizeOfInitializedData;
  /* 00c */  uint32_t                    SizeOfUninitializedData;
  /* 010 */  uint32_t                    AddressOfEntryPoint;
  /* 014 */  uint32_t                    BaseOfCode;
  /* 018 */  uint32_t                    BaseOfData;
  /* 01c */  uint32_t                    ImageBase;
  /* 020 */  uint32_t                    SectionAlignment;
  /* 024 */  uint32_t                    FileAlignment;
  /* 028 */  uint16_t                    MajorOperatingSystemVersion;
  /* 02a */  uint16_t                    MinorOperatingSystemVersion;
  /* 02c */  uint16_t                    MajorImageVersion;
  /* 02e */  uint16_t                    MinorImageVersion;
  /* 030 */  uint16_t                    MajorSubsystemVersion;
  /* 032 */  uint16_t                    MinorSubsystemVersion;
  /* 034 */  uint32_t                    Win32VersionValue;
  /* 038 */  uint32_t                    SizeOfImage;
  /* 03c */  uint32_t                    SizeOfHeaders;
  /* 040 */  uint32_t                    CheckSum;
  /* 044 */  uint16_t                    Subsystem;
  /* 046 */  uint16_t                    DllCharacteristics;
  /* 048 */  uint32_t                    SizeOfStackReserve;
  /* 04c */  uint32_t                    SizeOfStackCommit;
  /* 050 */  uint32_t                    SizeOfHeapReserve;
  /* 054 */  uint32_t                    SizeOfHeapCommit;
  /* 058 */  uint32_t                    LoaderFlags;
  /* 060 */  IMAGE_DATA_DIRECTORY        DataDirectory[16];
  /* Size 0e0 */
} IMAGE_OPTIONAL_HEADER;


typedef struct _IMAGE_NT_HEADERS {
  /* 000 */ uint32_t                     Signature;
  /* 004 */ IMAGE_FILE_HEADER            FileHeader;
  /* 008 */ IMAGE_OPTIONAL_HEADER        OptionalHeader;
} IMAGE_NT_HEADERS;

#endif



typedef struct _state
{
  uint64_t        mode;
  uint64_t        bytes_read;
  TCHAR            *file_extension;
  TCHAR            *fn;
  unsigned char   *buf;
  unsigned char   *strings;
  FILE            *handle;
  int             has_executable_extension;
  int             is_dll;
  int             is_exe;
  int             is_driver;
  uint16_t        string_len;
  uint64_t        files_processed;

  int             argc;
  TCHAR           **argv;
  
  TCHAR           *short_name;
  TCHAR           *msg;
  
} state;



/* ---------------------------------------------------------------
   ENGINE
   --------------------------------------------------------------- */
int process_normal(state *s, TCHAR *fn);
int process_win32(state *s, TCHAR *fn);
int check_file(state *s, TCHAR *fn);


/* ---------------------------------------------------------------
   HELPERS
   --------------------------------------------------------------- */
/* The basename function kept misbehaving on OS X, so I rewrote it.
   This function isn't perfect, nor is it designed to be. Because
   we're guarenteed to be working with a file here, there's no way
   that s will end with a DIR_SEPARATOR (e.g. /foo/bar/). This function
   will not work properly for a string that ends in a DIR_SEPARATOR */
int my_basename(TCHAR *s);
//     void shift_string(char *fn, size_t start, size_t new_start);
int my_dirname(TCHAR *c);

char * suffix(uint64_t n);
void shorten_filename(TCHAR *dest, TCHAR *src);

/* ---------------------------------------------------------------
   USER INTERFACE
   --------------------------------------------------------------- */
void print_status(char *fmt, ...);
void print_error(state *s, char *fmt, ...);
void fatal_error(char *fmt, ...);
void display_filename(FILE *out, TCHAR *fn);
void print_error_unicode(state *s, TCHAR *fn, char *fmt, ...);
/* Internal errors are so serious that we ignore the user's wishes
   about silent mode. Our need to debug the program outweighs their
   preferences. Besides, the program is probably crashing anyway... */
void internal_error(char *fmt, ... );

/* ---------------------------------------------------------------
   CYCLE CHECKING
   --------------------------------------------------------------- */
int have_processed_dir(TCHAR *fn);
int processing_dir(TCHAR *fn);
int done_processing_dir(TCHAR *fn);

#endif // ifndef __MAIN_H
