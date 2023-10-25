#ifndef _PSS_H__
#define _PSS_H__

struct  Pss_Info_t{
	char *mHash;
	int lenHash;
	char *em;
	int lenEm;
	unsigned char *(*sha256)(const unsigned char *d, int n,
                              unsigned char *md);
};

int pss_verify(struct Pss_Info_t *psInfo);

#endif
