#include <stdio.h>
#include <string.h>

#include "regex.h"
int main( int argc, char **argv ) {
	RegEx myRegEx;
	myRegEx.Compile("ab*c|d");
	bool match = myRegEx.Match("abc");
	match = myRegEx.Match("d");

	/*myRegEx.Compile("a|b");
	myRegEx.Compile("((a|b)|cd)");	
	myRegEx.Compile("D*\\.D|D\\.D*");
	myRegEx.Compile("(a|b)c");	
	myRegEx.Compile("ab*a");
	myRegEx.Compile("(a|b)a*b");
	myRegEx.Compile("a(bb)+a");
	myRegEx.Compile("(a|b)|cd");
	myRegEx.Compile("(a|b)*abbb");
	myRegEx.Compile("(1|0)*1");
	myRegEx.Compile("abcdefghi");
	myRegEx.Compile("(a|b)*abb");*/
	myRegEx.Compile("(a|b)*abb");
	std::string pattern;
	match = myRegEx.Match("abb");
	match = myRegEx.Match("abbbbd");
	match = myRegEx.Match("xyzxyz");
	match = myRegEx.Match("abx");

	myRegEx.Compile("ba?");
	match = myRegEx.Match("b");
	match = myRegEx.Match("ba");
	match = myRegEx.Match("bc");

	myRegEx.Compile("ba+");
	match = myRegEx.Match("b");
	match = myRegEx.Match("ba");
	match = myRegEx.Match("bc");

	myRegEx.Compile("(abc)|(def)");
	match = myRegEx.Match("abef");
	match = myRegEx.Match("abcef");
	match = myRegEx.Match("abdef");
	match = myRegEx.Match("abxyz");
	return 0;
}
