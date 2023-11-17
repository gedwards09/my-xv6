#include "types.h"
#include "user.h"
#define NULL (0)

int main(void)
{
	char* p = (char*)0x1001;
	printf(1, "%s\n", p);
	*p = 0x41; //'A'
	printf(1, "%s\n", p);
	exit();
}
