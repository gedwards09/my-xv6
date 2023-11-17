#include "types.h"
#include "user.h"

int ffork(char*);
int vawrite(char*);

int main(void)
{
	// base of page 1, start of .text
	char* p = (char*) 0x1000;
	// can overwrite own code 
	vawrite(p);
	// child inherits write privledges
	ffork(p);

	if (mprotect(p, 1) != 0)
	{
		printf(1, "mprotect: failed to protect %p\n");
		exit();
	}

	printf(1, "assert error device not available:\n");
	//write privledges revoked for child
	ffork(p);

	// restore protection
	if (munprotect(p, 1) != 0)
	{
		printf(1, "munprotect: failed to unprotect %p\n", p);
		exit();
	}
	// write priviledges restored for child:
	ffork(p);

	p = (char*) 0x1100; // not page aligned
	if (mprotect(p, 1) != -1)
	{
		printf(1, "mprotect: reject unaligned address fail\n");
		exit();
	}

	p = (char*) 0x10000; // unallocated page
	if (mprotect(p, 1) != -1)
	{
		printf(1, "mprotect: cannot protect unallocated page fail\n");
		exit();
	}

	p = (char*) 0x1000;
	if (mprotect(p, 1) != 0)
	{
		printf(1, "mprotect: failed to protect %p\n", p);
	}
	// kills process before exit
	printf(1, "assert error device not available\n");
	vawrite(p);
	// error if reachead
	printf(1, "mprotect: failed to enforce page protection  %p\n", p);
	exit();
}

int ffork(char* p)
{
	int pid = fork();
	if (pid == 0)
	{
		vawrite(p);
		exit();
	}
	wait();
	return 0;
}

int vawrite(char* p)
{
	*p += 1; // if protected segfault, kills child
	printf(1, "pid %d: didn't die!\n", getpid());
	return 0;
}


