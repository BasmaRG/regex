#ifndef _REG_EX_H_
#define _REG_EX_H_

//#include <deque>
#include <stack>
#include <set>
#include <list>
#include <string>
#include <vector>
#include <map>

#include "regexstate.h"
#include "regexpattern.h"


class RegEx {
public:
	RegEx();
	~RegEx();

	bool SetRegEx(std::string strRegEx);
	bool FindFirst(std::string strText, int &nPos, std::string &strPattern);
	bool FindNext(int &nPos, std::string &strPattern);

	typedef std::vector<RegExState*> Table;


private:
	Table m_NFATable;
	Table m_DFATable;

	std::stack<Table >       m_CharacterClassStack;
	std::stack<char>         m_ExpressionStack;
	std::set<char>           m_InputSet;
	std::list<RegExPattern*> m_PatternList;
	std::string              m_strText;
	std::vector<int>         m_vecPos;
	std::vector<std::string> m_vecPattern;
	std::vector<std::string> m_FoundPatterns;
	std::vector<int>         m_FoundPatternPositions;
	std::string              m_PostStrRegEx;
	char*                    m_InfixRegEx;
	char                     m_CurPreProcChar;
	int                      m_nNextStateID;
	int                      m_nPatternIndex;


	bool ConstructThompsonNFA(std::string strRegEx);
	void PushOnCharacterStack(char chInput);
	bool PopNFA(Table  &NFATable);
	bool EvalMetaChar(char);
	bool Concatenate();
	bool Closure();
	bool ClosureOptional();
	bool ClosurePlus();
	bool Or();
	bool IsMetaChar(char inputCh); 
	bool presedence (char LeftOperator, char RightOperator);
	bool IsInput(char inputCh);
	bool IsLeftParan(char inputCh); 
	bool IsRightParan(char inputCh); 
	void EpsilonClosure(std::set<RegExState*> T, std::set<RegExState*> &Res);
	void Move(char chInput, std::set<RegExState*> T, std::set<RegExState*> &Res);
	void ConvertNFAtoDFA();
	void ReduceDFA();
	void CleanUp();
	bool Find();

	int CovertToPostfix();
	int PreProcessLiterals();
	int PreProcessClosure();
	int PrePreprocessConcatenation();
	int PreProcessOr();
	int PreProcess();

	void printNFA();
	void printDFA();
	
protected:
};
#endif