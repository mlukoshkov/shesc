/*
 * SHell ESCape, Copyright 2019 Vitaly Sinilin
 *
 * This little program tries to make arbitrary string safe for using with
 * shell eval command. It inserts backslash in front of any character that
 * has special meaning for shell and is not yet escaped, so that evaluation
 * of the string will result in the literal meaning of any special expression
 * and will not have any side-effects like calling other commands, changing
 * variables, writing to files etc.
 *
 * Unfortunately, the eval command, which is unsafe by its nature, is the
 * only way shell could do a token recognition in an external string. So this
 * program is meant to be used to work around this situation by making the
 * string safer before passing it to eval.
 *
 * If you need to programmatically generate input that, having been processed
 * by this program, needs to be taken verbatim by shell, the easiest way
 * would be to follow these two rules for each potentially unsafe argument:
 * 1. Insert backslash in front of any backslash and double quote;
 * 2. Put the argument into double quotes.
 */
#include <stdio.h>
#include <string.h>

enum state {
	SPACE, BARE, BARE_ESCAPE, SQUOTED, DQUOTED, DQUOTED_ESCAPE
};

void put_buf(const char *data, size_t len)
{
	fwrite(data, 1, len, stdout);
}

void put_escaped(char ch)
{
	fputc('\\', stdout);
	fputc(ch, stdout);
}

/* Since this program makes no sense beyond shell environment, it is highly
 * unlikely there will be no way to redirect input to its stdin. Hence,
 * there is no need to provide an option to take filenames as parameters. */
int main(void)
{
	enum state state = SPACE;
	char buf[BUFSIZ];

	while (!feof(stdin)) {
		char *ptr = buf;
		size_t len = fread(buf, 1, sizeof buf, stdin);

#define advance(sz) do { ptr += (sz); len -= (sz); } while (0)
		while (len) {
			size_t nchars;
			switch (state) {
			case SPACE:
				/* Skip any unquoted white space characters. */
				nchars = strspn(ptr, " \t");
				advance(nchars);
				if (!len)
					break;
				if (*ptr == '\0') {
					advance(1);
					break;
				}
				state = BARE;
				/* fallthru */
			case BARE:
				nchars = strcspn(ptr, "$<>{}()!&|=;*~#%'\"`\n\\\t ");
				put_buf(ptr, nchars);
				advance(nchars);
				if (!len)
					break;
				switch (*ptr) {
				case '\0': break;
				case '\n':
				case '\t':
				case ' ':  putchar(*ptr);
				           state = SPACE;
				           break;
				case '\\': state = BARE_ESCAPE;
				           break;
				case '\'': putchar(*ptr);
				           state = SQUOTED;
				           break;
				case '"':  putchar(*ptr);
				           state = DQUOTED;
				           break;
				default:   put_escaped(*ptr);
				}
				advance(1);
				break;
			case BARE_ESCAPE:
				switch (*ptr) {
				case '\n': break;
				default:   put_escaped(*ptr);
				           /* fallthru */
				case '\0': advance(1);
				           break;
				}
				state = BARE;
				break;
			case SQUOTED:
				nchars = strcspn(ptr, "'\n");
				put_buf(ptr, nchars);
				advance(nchars);
				if (!len)
					break;
				switch (*ptr) {
				case '\n': putchar('\'');
				           state = BARE;
				           break;
				case '\'': putchar('\'');
				           state = BARE;
				           /* fallthru */
				case '\0': advance(1);
				           break;
				}
				break;
			case DQUOTED:
				nchars = strcspn(ptr, "$\\`\"\n");
				put_buf(ptr, nchars);
				advance(nchars);
				if (!len)
					break;
				switch (*ptr) {
				case '\n': putchar('"');
				           state = BARE;
				           break;
				case '"':  putchar('"');
				           advance(1);
				           state = BARE;
				           break;
				case '\\': advance(1);
				           state = DQUOTED_ESCAPE;
				           break;
				case '$':
				case '`':  put_escaped(*ptr);
				           /* fallthru */
				case '\0': advance(1);
				           break;
				}
				break;
			case DQUOTED_ESCAPE:
				switch (*ptr) {
				case '\n': break;
				case '"':
				case '\\':
				case '$':
				case '`':  put_escaped(*ptr);
				           /* fallthru */
				case '\0': advance(1);
				           break;
				default:   put_escaped('\\');
				           break;
				}
				state = DQUOTED;
				break;
			}
		}
	}

	/* Close anything unclosed. We do not allow tokens to span between
	 * lines. */
	switch (state) {
	case SPACE:
	case BARE:
	case BARE_ESCAPE:    break; /* Look, no compilation warning here! */
	case SQUOTED:        putchar('\''); break;
	case DQUOTED_ESCAPE:
	case DQUOTED:        putchar('"'); break;
	}

	return 0;
}
