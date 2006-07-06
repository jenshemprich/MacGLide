/* libzip mac specific */

#include <stdio.h>

int fseeko(FILE *stream, off_t offset, int whence)
{
	return fseek(stream, offset, whence);
}

int mkstemp(char*)
{
	/* Rough simulation of mkstemp */
	FILE* stream = tmpfile();
	return fileno(stream);
	/* Note that the FILE tmpfile is closed automaticaaly upon prg exit */
}
