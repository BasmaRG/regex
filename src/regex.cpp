#include <algorithm>
#include "regex.h"


#define EPSILON       -1 
#define CLOSURE       '*'
#define PLUS_CLOSURE  '+'
#define OPTIONAL      '?'
#define OPEN_PAREN    '('
#define CLOSE_PAREN   ')'
#define OR            '|'
#define FALSE_CONCAT  '&'



RegEx::RegEx() {
}
RegEx::~RegEx(){
	CleanUp();
}
bool RegEx::IsMetaChar( char inputCh ) {
	return(( inputCh == CLOSURE )      || 
		   ( inputCh == OR )           || 
		   ( inputCh == OPEN_PAREN )   || 
		   ( inputCh == CLOSE_PAREN )  || 
		   ( inputCh == PLUS_CLOSURE ) ||
		   ( inputCh == FALSE_CONCAT ) ||
		   ( inputCh == OPTIONAL ));
}
bool RegEx::IsInput(char inputCh)      { return(!IsMetaChar(inputCh));   }
bool RegEx::IsLeftParan(char inputCh)  { return(inputCh == OPEN_PAREN);  }
bool RegEx::IsRightParan(char inputCh) { return(inputCh == CLOSE_PAREN); }

void RegEx::CleanUp() {
	for(int i=0; i < (int)m_NFATable.size(); ++i)
		delete m_NFATable[i];
	m_NFATable.clear();

	for(int i=0; i < (int)m_DFATable.size(); ++i)
		delete m_DFATable[i];
	m_DFATable.clear();

	m_nNextStateID = 0;

	while(!m_CharacterClassStack.empty())
		m_CharacterClassStack.pop();

	while(!m_ExpressionStack.empty())
		m_ExpressionStack.pop();

	m_InputSet.clear();

	std::list<RegExPattern*>::iterator iter;
	for(iter = m_PatternList.begin(); iter != m_PatternList.end(); ++iter)
		delete *iter;
	m_PatternList.clear();
}

bool RegEx::PopTable(Table  &NFATable) {
	if(m_CharacterClassStack.size()>0) {
		NFATable = m_CharacterClassStack.top();
		m_CharacterClassStack.pop();
		return true;
	}
	return false;
}

bool RegEx::Concatenate() {

	Table  LeftTable, RightTable;
	if(!PopTable(RightTable) || !PopTable(LeftTable))
		return false;

	(*(LeftTable.rbegin()))->AddTransition(EPSILON, (*(RightTable.begin())));
	LeftTable.insert(LeftTable.end(), RightTable.begin(), RightTable.end());
	m_CharacterClassStack.push(LeftTable);

	return true;
}

void RegEx::PushOnCharacterStack(char chInput) {

	RegExState *s0 = new RegExState(++m_nNextStateID);
	RegExState *s1 = new RegExState(++m_nNextStateID);
	s0->AddTransition(chInput, s1);

	Table NewSubTable;
	NewSubTable.push_back(s0);
	NewSubTable.push_back(s1);
	m_CharacterClassStack.push(NewSubTable);

	m_InputSet.insert(chInput);
}

bool RegEx::ClosureOptional() {

	Table PrevTable;
	if(!PopTable(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(++m_nNextStateID);
	RegExState *RightTable = new RegExState(++m_nNextStateID);

	LeftTable->AddTransition(EPSILON, RightTable);
	LeftTable->AddTransition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, RightTable);

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	m_CharacterClassStack.push(PrevTable);

	return true;
}

bool RegEx::ClosurePlus() {
	Table PrevTable;
	if(!PopTable(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(++m_nNextStateID);
	RegExState *RightTable = new RegExState(++m_nNextStateID);

	LeftTable->AddTransition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, RightTable);
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, ((*(PrevTable.begin()))));

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	m_CharacterClassStack.push(PrevTable);

	return true;
}
bool RegEx::Closure() {

	Table PrevTable;
	if(!PopTable(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(++m_nNextStateID);
	RegExState *RightTable = new RegExState(++m_nNextStateID);

	LeftTable->AddTransition(EPSILON, RightTable);
	LeftTable->AddTransition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, RightTable);
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, ((*(PrevTable.begin()))));

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	m_CharacterClassStack.push(PrevTable);

	return true;
}

bool RegEx::Or() {

	Table  UpperTable, LowerTable;

	if(!PopTable(LowerTable) || !PopTable(UpperTable))
		return false;

	RegExState *LeftTable	= new RegExState(++m_nNextStateID);
	RegExState *RightTable	= new RegExState(++m_nNextStateID);

	LeftTable->AddTransition(EPSILON, ((*(UpperTable.begin()))));
	LeftTable->AddTransition(EPSILON, ((*(LowerTable.begin()))));
	((*(UpperTable.rbegin())))->AddTransition(EPSILON, RightTable);
	((*(LowerTable.rbegin())))->AddTransition(EPSILON, RightTable);

	LowerTable.push_back(RightTable);
	UpperTable.insert(UpperTable.begin(),LeftTable);
	UpperTable.insert(UpperTable.end(), LowerTable.begin(), LowerTable.end());

	m_CharacterClassStack.push(UpperTable);

	return true;
}
void RegEx::EpsilonClosure(std::set<RegExState*> startSet, std::set<RegExState*> &result) {
	std::stack<RegExState*> UnTestedStates;	

	result.clear();	
	result = startSet; // Each State is epsilon closure to itself.

	StateIterator iter;
	for(iter = startSet.begin(); iter != startSet.end(); ++iter) {
		UnTestedStates.push(*iter);
	}

	while(!UnTestedStates.empty()) {
		RegExState* curState = UnTestedStates.top();
		UnTestedStates.pop();

		Table epsilonStates;
		curState->GetTransition(EPSILON, epsilonStates);

		for(int i = 0; i < (int)epsilonStates.size(); ++i) {
			RegExState* epsilonState = epsilonStates[i];
			if(result.find(epsilonState) == result.end()) {
				epsilonState->m_Marked = false;
				result.insert(epsilonState);
				UnTestedStates.push(epsilonState);
			}
		}
	}
}

void RegEx::Move(char chInput, std::set<RegExState*> NFAState, std::set<RegExState*> &Result) {

	Result.clear();
	StateIterator iter;
	for(iter = NFAState.begin(); iter != NFAState.end(); ++iter) {
		Table States;
		(*iter)->GetTransition(chInput, States);
		for(int index = 0; index < (int)States.size(); ++index){
			Result.insert(States[index]);
		}
	}
}

void RegEx::ConvertNFAtoDFA() {
	for(int i = 0; i < (int)m_DFATable.size(); ++i)
		delete m_DFATable[i];
	m_DFATable.clear();

	if(m_NFATable.size() == 0)
		return;

	m_nNextStateID = 0;

	Table unmarkedStates;
	std::set<RegExState*> DFAStartStateSet;
	std::set<RegExState*> NFAStartStateSet;

	NFAStartStateSet.insert(m_NFATable[0]);

	EpsilonClosure(NFAStartStateSet, DFAStartStateSet);
	RegExState *DFAStartState = new RegExState(DFAStartStateSet, ++m_nNextStateID);

	m_DFATable.push_back(DFAStartState);

	unmarkedStates.push_back(DFAStartState);
	while(!unmarkedStates.empty()) {

		RegExState* CurDFAState = unmarkedStates[unmarkedStates.size()-1];
		unmarkedStates.pop_back();

		std::set<char>::iterator iter;
		for(iter=m_InputSet.begin(); iter!=m_InputSet.end(); ++iter) {
			std::set<RegExState*> MoveRes, EpsilonClosureRes;
			Move(*iter, CurDFAState->GetNFAState(), MoveRes);

			EpsilonClosure(MoveRes, EpsilonClosureRes);

			StateIterator MoveResItr;
			StateIterator EpsilonClosureResItr;

			bool bFound = false;
			RegExState *s   = NULL;
			for(int i = 0; i < (int)m_DFATable.size(); ++i) {
				s = m_DFATable[i];
				if(s->GetNFAState() == EpsilonClosureRes) {
					bFound = true;
					break;
				}
			}
			if(!bFound) {
				RegExState* U = new RegExState(EpsilonClosureRes, ++m_nNextStateID);
				unmarkedStates.push_back(U);
				m_DFATable.push_back(U);                                
				CurDFAState->AddTransition(*iter, U);
			} else {
				CurDFAState->AddTransition(*iter, s);
			}
		}
	}
	ReduceDFA();
}
void RegEx::ReduceDFA() {
	std::set<RegExState*> DeadEndSet;
	for(int i = 0; i < (int)m_DFATable.size(); ++i) {
		if(m_DFATable[i]->IsDeadEnd()) {
			DeadEndSet.insert(m_DFATable[i]);
		}
	}
	if(DeadEndSet.empty())
		return;
	StateIterator iter;
	for(iter = DeadEndSet.begin(); iter != DeadEndSet.end(); ++iter) {
		for(int i = 0; i < (int)m_DFATable.size(); ++i)
			m_DFATable[i]->RemoveTransition(*iter);

		TableIterator pos;
		for(pos = m_DFATable.begin(); pos != m_DFATable.end(); ++pos) {
			if(*pos == *iter) {
				break;
			}
		}
		m_DFATable.erase(pos);
		delete *iter;
	}
}

void RegEx::PartitionStates (Table &NonAcceptTable, Table &AcceptTable) {
	Table::const_iterator iter = m_DFATable.begin();
	for ( iter = m_DFATable.begin(); iter != m_DFATable.end(); ++iter) {
		if ((*iter)->m_bAcceptingState) {
			if (std::find (AcceptTable.begin (), AcceptTable.end (), *iter) == AcceptTable.end ()) {
				AcceptTable.push_back (*iter);
			}
		} else {
			if (std::find (NonAcceptTable.begin (), NonAcceptTable.end (), *iter) == NonAcceptTable.end ()){
				NonAcceptTable.push_back (*iter);
			}
		}
	}
}
void RegEx::MinimizeDFA () {
	m_nNextStateID = 0;
	Table NonAcceptTable,AcceptTable;
	PartitionStates( NonAcceptTable, AcceptTable);
	if ( m_DFATable.size() != NonAcceptTable.size() + AcceptTable.size()) {
		if ((*(m_DFATable.begin()))->m_bAcceptingState){
			NonAcceptTable.insert (NonAcceptTable.begin (), AcceptTable.begin (),	AcceptTable.end ());
		} else	{
			NonAcceptTable.insert (NonAcceptTable.end (), AcceptTable.begin (),	AcceptTable.end ());
		}
		TableIterator DfaIter = NonAcceptTable.begin();
		while (DfaIter != NonAcceptTable.end ()) {
			RegExState *NewState = new RegExState(++m_nNextStateID);
			NewState->m_bAcceptingState = (*(DfaIter))->m_bAcceptingState;
			std::multimap<char, RegExState*>::iterator StateIter = (*(DfaIter))->m_Transition.begin();
			while (StateIter != (*(DfaIter))->m_Transition.end()) {
				bool bFound = false;
				TableIterator DfaIter2 = NonAcceptTable.begin();
			}
		}
	}
}
void RegEx::PrintTable(Table &table) {
	TableIterator itr;
	for ( itr = table.begin(); itr != table.end(); ++itr){
		int stateid = (*itr)->m_nStateID;
		int flagNoTransition = true;
		std::multimap<char, RegExState*>::iterator A;
		for ( A = (*itr)->m_Transition.begin(); A != (*itr)->m_Transition.end(); ++A) {
			flagNoTransition = false;
			char ch = A->first;
			RegExState *S = A->second;
			printf ( "(%d) ----- (%c) -----> ",stateid, ch);
			if ( S->m_bAcceptingState == true ) {
				printf ( " ((%d))\n", S->m_nStateID);
			} else {
				printf ( " (%d)\n", S->m_nStateID);
			}
		}	
		if ( flagNoTransition ) {
			printf ( "(%d) -----     ----->\n", stateid);
		}
	}
	printf ( "--------------------------------------------------------------\n");
}

bool RegEx::SetRegEx(std::string strRegEx) {
	m_InfixRegEx = const_cast<char*>(strRegEx.c_str());
	CleanUp();
	if(!ConstructThompsonNFA(strRegEx))
		return false;	

	PopTable( m_NFATable);
	m_NFATable[m_NFATable.size() - 1 ]->m_bAcceptingState = true;

	ConvertNFAtoDFA();
	PrintTable(m_DFATable);
	MinimizeDFA();
	return true;
}

bool RegEx::FindFirst(std::string strText, int &nPos, std::string &strPattern) {
	std::list<RegExPattern*>::iterator iter;
	for(iter = m_PatternList.begin(); iter != m_PatternList.end(); ++iter)
		delete *iter;
	m_PatternList.clear();

	m_strText = strText;

	if(Find()) {
		nPos			= m_vecPos[0];
		strPattern		= m_vecPattern[0];
		m_nPatternIndex	= 0;
		return true;
	}
	return false;
}

bool RegEx::FindNext(int &nPos, std::string &strPattern) {
	++m_nPatternIndex;
	if(m_nPatternIndex < (int)m_vecPos.size())	{
		nPos			= m_vecPos[m_nPatternIndex];
		strPattern		= m_vecPattern[m_nPatternIndex];
		return true;
	}
	return false;
}

bool RegEx::Find() {
	bool bRes = false;
	m_vecPos.clear();
	m_vecPattern.clear();

	if(m_DFATable.empty())
		return false;

	RegExState *pState = m_DFATable[0];
	for(int i = 0; i < (int)m_strText.size(); ++i) {

		char c = m_strText[i]; // Take the first character

		Table Transition;
		pState->GetTransition(c, Transition);

		if(!Transition.empty()) {
			RegExPattern *pPatternState = new RegExPattern();
			pPatternState->m_nStartIndex	= i;
			pPatternState->m_pState			= Transition[0];
			m_PatternList.push_back(pPatternState);

			if(Transition[0]->m_bAcceptingState){
				m_vecPos.push_back(i);
				std::string strTemp;
				strTemp += c;
				m_vecPattern.push_back(strTemp);
			}
		} else {
			if(pState->m_bAcceptingState) {
				m_vecPos.push_back(i);
				std::string strTemp;
				strTemp += c;
				m_vecPattern.push_back(strTemp);
			}
		}
	}
	return(m_vecPos.size()>0);
}

int RegEx::PreProcessLiterals() {
  if (m_CurPreProcChar == '\0') 
	  return -1;
  if (m_CurPreProcChar == '\\') {
    m_CurPreProcChar = *(m_InfixRegEx++);
    if (m_CurPreProcChar == '\0') 
		return -1;
    m_PostStrRegEx += m_CurPreProcChar;
    m_CurPreProcChar = *(m_InfixRegEx++);
  } else if (strchr("()|*+?", m_CurPreProcChar) == NULL) { 
    if (m_CurPreProcChar == '&')
		m_PostStrRegEx += "\\&";
    else 
		m_PostStrRegEx += m_CurPreProcChar;

    m_CurPreProcChar = *(m_InfixRegEx++);
  } else if (m_CurPreProcChar == '(') {
    m_CurPreProcChar = *(m_InfixRegEx++);
    PreProcessOr();
    if (m_CurPreProcChar != ')') 
		return -1;
    m_CurPreProcChar = *(m_InfixRegEx++);
  } else
		return -1;
  return 0;
}
int RegEx::PreProcessClosure() {
	PreProcessLiterals();
	while (m_CurPreProcChar != '\0' && strchr("*+?",m_CurPreProcChar) != NULL) {
		m_PostStrRegEx += m_CurPreProcChar;
		m_CurPreProcChar = *(m_InfixRegEx++);
	}
	return 0;
}
int RegEx::PrePreprocessConcatenation(void) {
	PreProcessClosure();
	while (strchr(")|*+?", m_CurPreProcChar) == NULL) { 
		PreProcessClosure();
		m_PostStrRegEx += "&";
	}
	return 0;
}
int RegEx::PreProcessOr() {
	PrePreprocessConcatenation();
	while (m_CurPreProcChar == '|') {
		m_CurPreProcChar = *(m_InfixRegEx++);
		PrePreprocessConcatenation();
		m_PostStrRegEx += "|";
	}
	return 0;
}

int RegEx::CovertToPostfix() {
	m_CurPreProcChar = *(m_InfixRegEx++);
	PreProcessOr();
	return 0;
}
bool RegEx::ConstructThompsonNFA(std::string strRegEx) {
	CovertToPostfix();
	for(int i = 0; i < (int)m_PostStrRegEx.size(); ++i) {
		char curChar = m_PostStrRegEx[i];
		if ( IsInput( curChar )) {
			PushOnCharacterStack( curChar );
		} else if ( IsMetaChar( curChar ) ) {
			switch ( curChar ) {
				case CLOSURE:
					Closure();
					break;
				case PLUS_CLOSURE:
					ClosurePlus();
					break;
				case OPTIONAL:
					ClosureOptional();
					break;
				case OR:
					Or();
					break;
				case FALSE_CONCAT:
					Concatenate();
					break;
			}
		}		
	}
	return true;
}


