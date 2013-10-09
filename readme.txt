gensconv.zip - HiSoft Gens source converter.

Do you think DEVPAC is the best assembler for the Spectrum computer?
Well so do I. The only bad thing about DEVPAC is the editor, working with that
is worse than working with MS-DOS's 'edlin' or CP/M's 'ed'.

Would you like to be able to use your favorite editor, like e.g. emacs, for
writing your DEVPAC source file? Well here's the solution, this little package
contains two small programs which will convert DEVPAC source files to normal
text-files and back again. And that's not all, the programs can read and write
files in (for) the following formats:

	* Spectrum +3 files (use e.g. 22DISK for transfering to MS-DOS)
	* Z80 v2.01 .TAP files
	* XZX tape files

And the programs are:
txt2gens.c : the program which will convert ascii text-files into a file
	which DEVPAC will understand. Must be called with :
	txt2gens -[p3|z80|xzx] destfile

	And takes input from stdin. All comments will be ignored to save
	memory for the real source. All assembler statements must be in the 
	format as stated in the DEVPAC manual.

	NOTE! Z80 and XZX users must be aware of the used file name, since 
	there's no check for invalid file name. 

	Invoking txt2gens with wrong number of parametres, will display a
	little help page.

gens2txt.c : the program which will convert your DEVPAC file into pure ascii.
	Must be called with :
	gens2txt sourcefile

	And writes the result to stdout. The program can by itself determine
	which of the three types of supported filetypes, produced this source.

	Again a little help page will be displayed, if wrong number of 
	arguments are supplied.

The programs has been compiled and tested under MS-DOS with TURBO-C 1.0, and 
under Linux with gcc-2.5.8.
BTW, none of the programs will do any form of case conversion, so remember
to use CAPS when writing your source, at least for the MNEMONICS.

Included in this package is a little, but unfinished, font-editor for all
types of Speccy's. Mostly included for testing purpose, as only the basic
functions works. The font-editor will edit fonts of size 8x8, 8x16, 16x8 and 
the huge 16x16, so when it's finished it could be considered as a combined
font/sprite-editor.

TODO:
* Complete the Font-editor program.

* Hopefully in the near future, I'll include #defines so this little piece of
  code willl compiler under HiTech C for CP/M.

* Adding support for +D files, made by DEVPAC.


Sep-94 Thomas Kjaer
email: takjaer@daimi.aau.dk
