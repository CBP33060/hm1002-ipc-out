#ifndef __T41_CPM_H__
#define __T41_CPM_H__

#include <common.h>

/*
 * Clock reset and power controller module(CPM) address definition
 */
//#define	CPM_BASE	0xb0000000
#ifndef CPM_BASE
#define	CPM_BASE	0x10000000
#endif
/*
 * CPM registers offset address definition
 */
#define CPM_CLKGR0_OFFSET       (0x20)  /* T40, rw, 32, 0x1FFFFF80 */
/*
 * CPM registers address definition
 */
#ifndef CPM_CLKGR0
#define CPM_CLKGR0       	(CPM_BASE + CPM_CLKGR0_OFFSET)		/* T41 */
#endif

/* T40, Clock gate register 0(CLKGR0) */
#define CLKGR0_SC_HASH      BIT2 //t41
#define CLKGR0_RSA          BIT27 //t41
/********************************************/
unsigned int * cpm_vir_base;
#endif /* __T41_CPM_H__ */
