#include <stdio.h>



int main()
{
char* s1 = "string";
char s2[] = "string";
	//s1 = "w";
	*s1 = 'w';
	s2 = "wlllll";
	//*s2 = 'w';
}
/*c99
11:cannot convert from 'const char [7]' to 'char [7]'
        There is no context in which this conversion is possible
jichu2:10 running error  

*/