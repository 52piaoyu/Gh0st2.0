#include "StdAfx.h"
#include <Windows.h>

int myiswctype(wint_t c, wctype_t type)
{
	WORD ret;
	GetStringTypeW(CT_CTYPE1, (LPCWSTR)&c, 1, &ret);
	if ((ret & type) != 0)
		return 1;
	return 0;
}

int myiswdigit(wint_t c)
{
	return myiswctype(c, _DIGIT);
}

int myiswspace(wint_t c)
{
	return myiswctype(c, _BLANK);
}

long mywtol(const wchar_t *str)
{
	while (myiswspace(*str))			// skip whitespace
		++str;

	wint_t cur = *str++;
	wint_t neg = cur;					// Save the negative sign, if it exists

	if (cur == L'-' || cur == L'+')
		cur = *str++;

	// While we have digits, add 'em up.

	long total = 0;
	while (myiswdigit(cur))
	{
		total = 10 * total + (cur - L'0');			// Add this digit to the total.
		cur = *str++;							// Do the next character.
	}

	// If we have a negative sign, convert the value.
	if (neg == L'-')
		return -total;
	else
		return total;
}

int mywtoi(const wchar_t *str)
{
	return (int)mywtol(str);
}

void *malloc(size_t size)
{
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void free(void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
}


void *memmove(void *dst, const void *src, size_t count)
{
	void *ret = dst;
	if (dst <= src || (char *)dst >= ((char *)src + count))
	{
		// Non-Overlapping Buffers
		// copy from lower addresses to higher addresses
		while (count--)
		{
			*(char*)dst = *(char*)src;
			dst = (char*)dst + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		// Overlapping Buffers
		// copy from higher addresses to lower addresses
		dst = (char*)dst + count - 1;
		src = (char*)src + count - 1;

		while (count--)
		{
			*(char*)dst = *(char*)src;
			dst = (char*)dst - 1;
			src = (char*)src - 1;
		}
	}

	return ret;
}

void *realloc(void *p, size_t size)
{
	if (p)
		return HeapReAlloc(GetProcessHeap(), 0, p, size);
	else
		return HeapAlloc(GetProcessHeap(), 0, size);
}


void *operator new(unsigned int s)
{
	return HeapAlloc(GetProcessHeap(), 0, s);
}

void operator delete(void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
}

void operator delete[](void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
}

void *operator new[](unsigned int s)
{
	return HeapAlloc(GetProcessHeap(), 0, s);
}
//////////////////////////////

