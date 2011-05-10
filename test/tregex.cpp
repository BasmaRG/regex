#include <stdio.h>
#include <string.h>

#include "regex.h"
int main( int argc, char **argv ) {
	RegEx myRegEx;
	/*myRegEx.SetRegEx("ab*c|d");
	myRegEx.SetRegEx("a|b");
	myRegEx.SetRegEx("((a|b)|cd)");	*/
	myRegEx.SetRegEx("D*\\.D|D\\.D*");/*
	myRegEx.SetRegEx("(a|b)c");	
	myRegEx.SetRegEx("ab*a");
	myRegEx.SetRegEx("(a|b)a*b");
	myRegEx.SetRegEx("a(bb)+a");
	myRegEx.SetRegEx("(a|b)|cd");
	myRegEx.SetRegEx("(a|b)*abbb");
	myRegEx.SetRegEx("(1|0)*1");
	myRegEx.SetRegEx("abcdefghi");
	myRegEx.SetRegEx("(a|b)*abb");*/
	std::string pattern;
	bool match = myRegEx.Match("abbbbc");
	match = myRegEx.Match("abbbbd");

/*	myRegEx.Match("Hello How are you ab", position, pattern);
	myRegEx.Match("Hello How are you cd", position, pattern);*/
	return 0;
}