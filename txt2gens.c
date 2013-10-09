/* Ascii-text file to DEVPAC source file converter.
 *
 * This program will read text files and convert it to DEVPAC's Gens
 * assembler file.
 * The program can write files in one of the following formats:
 *      Real +3 DOS files.
 *      Tape files read by Z80 v2.01.
 *      Tape files read by xzx.
 *
 * Syntax: txt2gens -[p3|z80|xzx] file
 *
 * The ascii file will be read from stdin, and there won't be any case
 * conversion so remember to use upper-case when writing your source.
 * Also all comments will be ignored, making the DEVPAC source a little
 * smaller than the ascii-source, but it shouldn't matter at all.
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

#define MAXLEN  80 /* Maximum length of a line, DEVPAC has a limit */

unsigned int read_line(int);
unsigned int write_line(int, int);
unsigned char header_xor, block_xor; /* for Z80 only */
void prepare_plus3(void);
void prepare_z80(char *);
void prepare_xzx(char *);
void close_plus3(int);
void close_z80(int);
void close_xzx(int);
void fput_16bit(unsigned int);

char XZXID[4] = "ZXF1";
char PLUS3ID[8] = "PLUS3DOS";
unsigned char header[128]; /* All types needs a header */

FILE *fp;                /* the destination file */
unsigned int source_len; /* Length of the source code, as seen from DEVPAC. */
char line[MAXLEN];       /* For one line of input. */

main(int argc, char *argv[])
{
  char *filename;          /* name of file. */
  unsigned char filetype;  /* Who is this file intended for?? */
  unsigned int bytes_written, line_no;
  int len;                 /* The length of a line. */
  
  if (argc != 3) {
    fprintf(stderr, "\ntxt2gens - Ascii file to Devpac assembler source converter.\n");
    fprintf(stderr, "1994 by Thomas A. Kjaer takjaer@daimi.aau.dk\n\n");
    fprintf(stderr, "Usage: txt2gens -[p3|z80|xzx] file\n");
    fprintf(stderr, "\nThis program takes a ascii file from stdin and convert it to a DEVPAC gens\n");
    fprintf(stderr, "source file. The result will be written in one of the following formats:\n");
    fprintf(stderr, "\n -p3  Spectrum +3 DOS file\n");
    fprintf(stderr, " -z80 Spectrum emulator Z80 v2.01 .TAP file\n");
    fprintf(stderr, " -xzx Spectrum emulator XZX tape file\n");
    fprintf(stderr, "\nTape users, watch out! This program won't check for illegal filenames.\n");
    exit(1);
  };
  
  filename = malloc(strlen(argv[2]));
  strcpy(filename, argv[2]);
  
  /* open destination file. */
  if ((fp = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "%s: can't create %s\n\n", argv[0], filename);
    exit(1);
  }
  
  fprintf(stderr, "\nConverting to file: %s\n", filename);
  fprintf(stderr, "Writing to ");
  filetype = 0;
  if (strcmp(argv[1], "-p3") == 0) {
    filetype = PLUS3;
    prepare_plus3();
    fprintf(stderr, "+3 DOS\n");
  }
  if (strcmp(argv[1], "-z80") == 0) {
    filetype = Z80;
    prepare_z80(filename);
    fprintf(stderr, "Z80\n");
  }
  if (strcmp(argv[1], "-xzx") == 0) {
    filetype = XZX;
    prepare_xzx(filename);
    fprintf(stderr, "XZX\n");
  }
  if (filetype == 0) {
    fprintf(stderr, "unknown type, must be one of -p3, -z80 or -xzx\n");
    fclose(fp);
    exit(1);
  }
  
  line_no = 1;
  bytes_written = 0; /* haven't done anything yet :) */
  while ((len = read_line(MAXLEN)) != 0) {
    bytes_written = bytes_written + write_line(line_no, len);
    ++line_no;
    if (line_no > 32767) {
      fprintf(stderr, "\0x07 warning : DEVPAC can only handle max 32767 lines\n");
      break;
    }
  }
  
  switch(filetype) {
  case PLUS3 :
    close_plus3(bytes_written);
    break;
  case XZX :
    close_xzx(bytes_written);
    break;
  case Z80 :
    close_z80(bytes_written);
    break;
  }
  
  fclose(fp);
  return 0;
}

/* fput_16bit: writes the 16bit value at current position. */
void fput_16bit(unsigned int value)
{
  fputc((unsigned char)(value % 256), fp);
  fputc((unsigned char)(value / 256), fp);
}

/* Prepare and write as much of the header as possible. */
void prepare_plus3(void)
{
  int i;
  
  memcpy(header, PLUS3ID, 8);
  header[8] = 0x1a; header[9] = 0x01;
  /* next comes the length, but we don't know yet.
   * rest of the 128 bytes header is zero filled, except the checksum byte
   * which is the last bytes, but we don't know that either.
   */
  for (i = 10; i < 128; header[i++] = 0);
  if (fwrite(header, sizeof(unsigned char), 128, fp) != 128) {
    fprintf(stderr, "error in write header!\n");
    fclose(fp);
    exit(1);
  }
}

void prepare_z80(char *filename)
{
  int i;
  
  /* header block, 19 bytes. */
  fput_16bit(0x0013);
  header[0] = 0x00; /* header */
  header[1] = 0x03; /* code block */
  for (i = 2; i < 12; header[i++] = 0x20); /* fill with spaces */
	i = 0;
  while(*filename != 0 && i < 10)
    header[2+i++] = *filename++;
  /* then the length, which we don't know yet.
   * and at last 2 'ID' words.
   */
  header[14] = 0xb7; header[15] = 0x84; header[16] = 0xa7; header[17] = 0x06;
  /* checksum, don't know yet. */
  header[18] = 0x00;
  if (fwrite(header, sizeof(unsigned char), 19, fp) != 19) {
    fprintf(stderr, "error in write header!\n");
    fclose(fp);
		exit(1);
  }
  /* calculate checksum for header so far. */
  header_xor = header[0];
  for (i = 1; i < 18; header_xor = header_xor ^ header[i++]);
  
  /* length and block ID for next part. */
  fput_16bit(0x0000); /* don't know length yet. */
  fputc(0xff, fp);    /* datablock */
  block_xor = 0xff;
}

void prepare_xzx(char *filename)
{
  int i;
  
  memcpy(header, XZXID, 4);
  /* next comes the filetype which is 3 and then 
   * the filename which is space filled.
   */
  header[4] = 0x03; /* 3 = CODE */
  for (i = 5; i < 15; header[i++] = 0x20); /* fill with spaces */
  i = 0;
  while(*filename != 0 && i < 10)
    header[5+i++] = *filename++;
  /* then the length, which we don't know yet.
   * and at last 2 'ID' words. 
   */
  header[17] = 0xb7; header[18] = 0x84; header[19] = 0xa7; header[20] = 0x06;
  
  if (fwrite(header, sizeof(unsigned char), 21, fp) != 21) {
    fprintf(stderr, "error in write header!\n");
    fclose(fp);
    exit(1);
  }
}

/* close the file, and do the rest of the work with the header. */
void close_plus3(int len)
{
  unsigned char chksum;
  int n;
  
  len = len + 128; /* total length includes the header */
  fseek(fp, 11, SEEK_SET);
  fput_16bit(len); /* write the total length */
  header[11] = len % 256;
  header[12] = len / 256;
  chksum = 0;
  for (n = 0; n < 127; chksum = chksum + header[n++]);
  header[127] = chksum % 256;
  fseek(fp, 127, SEEK_SET); fputc(header[127], fp);
}

void close_xzx(int len)
{
  fseek(fp, 15, SEEK_SET);
  fput_16bit(len);
}

void close_z80(int len)
{
  fputc(block_xor, fp);   /* checksum for datablock */
  fseek(fp, 21, SEEK_SET);
  fput_16bit(len + 0x02); /* finished the datablock */
  
  fseek(fp, 14, SEEK_SET);
  fput_16bit(len);        /* for the header */
  header_xor = header_xor ^ (len % 256) ^ (len / 256);
  fseek(fp, 20, SEEK_SET);
  fputc(header_xor, fp);
}

/* read_line: read a line, of max lim characters, into the buffer,
 * return length.
 */
unsigned int read_line(int lim)
{
  int c, i;
  
  for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; ++i)
    line[i] = c;
  if (c == '\n') {
    line[i] = c;
    ++i;
  }
  line[i] = '\0';
  return i;
}

/* write_line: writes a line of DEVPAC code, from a length len ascii line,
 * with line number lineno return bytes written.
 */
unsigned int write_line(int lineno, int len)
{
  int i, n;
  char in_string;
  
  fput_16bit(lineno);
  i = 2; /* already written 2 bytes. */
  
  /* beacuse of Z80 every writes to the output file must be xor'ed */
  block_xor = block_xor ^ (lineno % 256) ^ (lineno / 256);
  
  n = 0;
  in_string = 0;
  while (n < len - 1) {
    switch(line[n]) {
    case '\t' : case ' ' :
      if (!in_string) { /* replace space and tab with 1 tab */
	while(line[n] == '\t' || line[n] == ' ' && n < len - 1) n++;
	fputc(0x09, fp); i++; block_xor = block_xor ^ 0x09;
	fprintf(stderr, "\t");
      }
      else {
	fputc(line[n++], fp); i++; block_xor = block_xor ^ line[n-1];
	fputc(line[n-1], stderr);
      }
      break;
    case ';' :
      if (!in_string) { /* just 'ignore' rest of the line */
	while(n < len - 1) {
	  n++; /* if you want comments, comment this line */
	  /* and uncomment the next 2 lines */  
	  /*
	     fputc(line[n++], fp); i++; block_xor = block_xor ^ line[n-1];
	     fputc(line[n-1], stderr);
	   */
	}
      }
      else {
	fputc(line[n++], fp); i++; block_xor = block_xor ^ line[n-1];
	fputc(line[n-1], stderr);
      }
      break;
    case '"' :
      in_string = !in_string;
      fputc(line[n++], fp); i++; block_xor = block_xor ^ line[n-1];
      fputc(line[n-1], stderr);
      break;
    default :
      fputc(line[n++], fp); i++; block_xor = block_xor ^ line[n-1];
      fputc(line[n-1], stderr);
      break;
    }
  }
  
  fputc(0x0d, fp); i++; block_xor = block_xor ^ 0x0d;
  fprintf(stderr, "\n");
  return i;
}
