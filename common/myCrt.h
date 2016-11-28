#pragma once

int myiswctype(wint_t c, wctype_t type);

int myiswdigit(wint_t c);
int myiswspace(wint_t c);
	
long mywtol(const wchar_t *str);

int mywtoi(const wchar_t *str);


void *mymalloc(size_t size);

void myfree(void *p);

void *mymemmove(void *dst, const void *src, size_t count);

void *myrealloc(void *p, size_t size);



void *operator new(unsigned int s);

void operator delete(void *p);

void operator delete[](void *p);

void *operator new[](unsigned int s);

//////////////////////////////

