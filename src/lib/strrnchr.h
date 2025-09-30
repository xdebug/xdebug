/*
ISC License

© 2017, 2018, 2021, 2022, 2023, 2024 Mattias Andrée <m@maandree.se>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/* From: https://github.com/maandree/libsimple/blob/9b483673c0c8a52d81127a23788fa4c976f2b10f/strrnchr.c */
static char *strrnchr(const char *s_, int c_, size_t n)
{
	char *s = *(char **)(void *)&s_, c = (char)c_;
	char *end = &s[n], *r = NULL;

	for (; s != end; s++) {
		if (*s == c) {
			r = s;
		}
		if (!*s) {
			break;
		}
	}
	return r;
}

