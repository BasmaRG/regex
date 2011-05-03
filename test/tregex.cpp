#include <stdio.h>
#include <string.h>

#include "regex.h"
int main( int argc, char **argv ) {
	RegEx myRegEx;
	//myRegEx.SetRegEx("ab*c|d");
	//myRegEx.SetRegEx("a|b");
	//myRegEx.SetRegEx("((a|b)|cd)");	
	myRegEx.SetRegEx("D*\\.D|D\\.D*");	
	std::string pattern;
	int position;
	myRegEx.FindFirst("Hello How are you aaaaaab", position, pattern);
	myRegEx.FindFirst("Hello How are you ab", position, pattern);
	myRegEx.FindFirst("Hello How are you cd", position, pattern);
	return 0;
}