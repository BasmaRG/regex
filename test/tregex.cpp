#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "regex.h"
int main( int argc, char **argv ) {
	RegEx re;
	re.Compile("abcd");
	assert ( true == re.Match("abcd"));
	assert ( true == re.Match("abcde"));
	assert ( false == re.Match("acde"));

	re.Compile("ab*");
	assert ( true == re.Match("ac"));
	assert ( true == re.Match("abc"));
	assert ( true == re.Match("abbc"));
	assert ( true == re.Match("acbb"));
	assert ( false == re.Match("xcbb"));

	re.Compile("ab+");
	assert ( true == re.Match("abc"));
	assert ( true == re.Match("abbc"));
	assert ( false == re.Match ("ac"));

	re.Compile("ab?");
	assert ( true == re.Match("ac"));
	assert ( true == re.Match("abc"));
	assert ( true == re.Match ("abbc"));

	re.Compile("a|b*");
	assert ( true == re.Match("a"));
	assert ( true == re.Match("b"));
	assert ( true == re.Match ("bbb"));
	assert ( true == re.Match ("bb"));
		
	re.Compile("(a|b)*");
	assert ( true == re.Match("a"));
	assert ( true == re.Match("b"));
	assert ( true == re.Match ("aa"));
	assert ( true == re.Match ("ab"));
	assert ( true == re.Match ("ba"));
	assert ( true == re.Match ("bb"));
	assert ( true == re.Match ("aaa"));

	re.Compile("a*b");
	assert ( false == re.Match("a"));
	assert ( true == re.Match("ab"));
	assert ( true == re.Match("aaaabbbcccb"));
	assert ( false == re.Match("a12344b"));
	assert ( false == re.Match("a@#$@#$b"));
	assert ( true == re.Match("b"));

	re.Compile("(a|b)*abb");
	assert ( true == re.Match("abb"));
	assert ( true == re.Match("aabb"));
	assert ( true == re.Match("babb"));
	assert ( true == re.Match("aaabb"));
	assert ( true == re.Match("bbabb"));
	assert ( true == re.Match("ababb"));
	assert ( true == re.Match("aababb"));

	return 0;
}
