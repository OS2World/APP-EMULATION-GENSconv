/* DEVPAC source file to text file converter.
 *
 * This program will read source files produces with DEVPAC's Gens assembler
 * and convert it to pure ascii text.
 * The program can read the files made by one of the following:
 *      Real +3 DOS files.
 *      Tape files from Z80 v2.01.
 *      Tape files from xzx.
 * Since +3 and xzx files can be detected automatic, the program will do so.
 * This means that if the program doesn't find the source file to be from
 * either +3 or xzx, it assumes Z80 tape format.
 *
 * Syntax: gens2txt file
 *
 * The resulting ascii file will be written to stdout, so if you want to
 * save it to a file, remember to redirect stdout.
 *
 * Made by:
 *      Thomas A. Kjaer 
 *      email: takjaer@daimi.aau.dk
 *
 * Any bugs and improvements could be reported to me, and then if I have the
 * time for it, I'll try to fix it.
 *
 * Made with TC 1.0 under MS-DOS & GCC under Linux 1.0
 */

#include <stdio.h>
#include <stdlib.h> /* for malloc() */

#define Z80     255
#define XZX     254
#define PLUS3   253

void read_z80_header(void);
void read_xzx_header(void);
void read_plus3_header(void);
unsigned int read_line(void);

char XZXID[4] = "ZXF1";
char PLUS3ID[8] = "PLUS3DOS";

FILE *fp;                /* The source file */
unsigned int source_len; /* Length of the source code, as seen from DEVPAC. */

main(int argc, char *argv[])
{
  char checkid[8];          /* For filetype determination. */
  unsigned int bytes_read;  /* How may bytes has been read??*/
  char *filename;           /* Name of file. */
  unsigned char filetype;   /* Who produced this file?? */
  
  if (argc != 2) {
    fprintf(stderr, "\ngens2txt - Devpac assembler source to ascii converter.\n");
    fprintf(stderr, "1994 by Thomas A. Kjaer takjaer@daimi.aau.dk\n\n");
    fprintf(stderr, "Usage: gens2txt file\n");
    fprintf(stderr, "\nThis program takes a DEVPAC gens source file in one of the following formats:\n");
    fprintf(stderr, "\n Spectrum +3 DOS file\n");
    fprintf(stderr, " Spectrum emulator Z80 v2.01 .TAP file\n");
    fprintf(stderr, " Spectrum emulator XZX tape file\n");
    fprintf(stderr, "\nand writes to stdout, the ascii representation of the source file.\n");
    fprintf(stderr, "The program will automatic determine the filetype after the following rules:\n");
    fprintf(stderr, "\n Plus 3 & XZX files can be detected automatic, meaning that if the program\n");
    fprintf(stderr, "doesn't find the file to be one of these, it will assume Z80 v2.01 .TAP file\n");
    fprintf(stderr, "The program doesn't try to figure out, if the file really is a DEVPAC file.\n");
    exit(1);
  };
  
  filename = malloc(strlen(argv[1]));
  strcpy(filename, argv[1]);
  
  fprintf(stderr,"\nConverting file: %s\n", filename);
  
  /* open source file and try to determine the filetype. */
  if ((fp = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "%s: can't open %s\n\n", argv[0], argv[1]);
    exit(1);
  }
  
  filetype = Z80;             /* default filetype */
  fscanf(fp, "%8s", checkid); /* read eventually filetype indicator. */
  
  if (strncmp(XZXID, checkid, 4) == 0) filetype = XZX;
  if (strncmp(PLUS3ID, checkid, 8) == 0) filetype = PLUS3;
  
  fprintf(stderr, "Reading file from ");
  switch(filetype) {
  case Z80 :
    fprintf(stderr, "Z80\n");
    read_z80_header();
    break;
  case XZX :
    fprintf(stderr, "XZX\n");
    read_xzx_header();
    break;
  case PLUS3 :
    fprintf(stderr, "+3 DOS\n");
    read_plus3_header();
    break;
  default :
    fprintf(stderr, "unknown filetype, FATAL error caused by the programmer\n");
    break;
  }
  fprintf(stderr, "\n");

  bytes_read = 0;  /* haven't seen anything yet :) */
  while (bytes_read < source_len)
    bytes_read = bytes_read + read_line();
  
  fclose(fp);
  return 0;
}

/* Read header functions, finds the length of the source file as specified
 * by DEVPAC, and sets the filepositions to the beginning of the data area.
 */
void read_z80_header(void)
{
  fseek(fp, 14, SEEK_SET);
  source_len = fgetc(fp) + (unsigned int)(fgetc(fp) << 8);
  fseek(fp, 24, SEEK_SET);      /* set the fileposition. */
}

void read_xzx_header(void)
{
  fseek(fp, 15, SEEK_SET);
  source_len = fgetc(fp) + (unsigned int)(fgetc(fp) << 8);
  fseek(fp, 21, SEEK_SET);      /* set the fileposition. */
}

void read_plus3_header(void)
{
  fseek(fp, 11, SEEK_SET);
  source_len = fgetc(fp) + (unsigned int)(fgetc(fp) << 8) - 128;
  fseek(fp, 128, SEEK_SET);     /* set the fileposition. */
}

/* Reads and prints one line of DEVPAC code.
 * Returns number of bytes read.
 *
 * The output from this is in this format.
 *
 * LABEL MNEMONIC OPERANDS COMMENT
 */
unsigned int read_line(void)
{
  int len, end_of_line;
  char data;
  
  /* ignore the line number */
  fseek(fp, 2, SEEK_CUR);
  len = 2;
  end_of_line = 0;
  
  while(!end_of_line) {
    if ((data = fgetc(fp)) == -1) {
      /* length error ! */
      fprintf(stderr, "\npremature EOF, error");
      fclose(fp);
      exit(1);
    }
    
    len++;
    switch(data) {
    case 0x0d :
      end_of_line = !end_of_line;
      break;
    case 0x09 :
      printf("\t");
      break;
    default :
      fputc(data, stdout);
      break;
    }
  }
  printf("\n");
  
  return len;
}
