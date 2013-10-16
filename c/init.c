/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <di_calls.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/
extern void idleproc(void);
unsigned int idleproc_pid;
/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{
	kprintf( "\n\nCPSC 415, 2012W1\nA1 Solution Kernel\n32 Bit Xeros 1.1\nLocated at: %x to %x\n", &entry, &end ); 

        /* Your code goes here */

        kprintf("Max addr is %d %x\n", maxaddr, maxaddr);

        kmeminit();
        kprintf("memory inited\n");

        dispatchinit();
        kprintf("dispatcher inited\n");
  
        contextinit();
        kprintf("context inited\n");

		deviceinit();
		kprintf("device inited\n");

        create( root, PROC_STACK );
        kprintf("create inited\n");
  
		idleproc_pid = create (idleproc, PROC_STACK);
		
		kprintf("idleproc pid %d inited\n", idleproc_pid);
		
        dispatch();
  
        kprintf("Returned to init, you should never get here!\n");

        /* This code should never be reached after you are done */
	for(;;); /* loop forever */
}


