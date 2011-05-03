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

bool RegEx::PopNFA(std::deque<RegExState*>  &NFATable) {
	if(m_CharacterClassStack.size()>0) {
		NFATable = m_CharacterClassStack.top();
		m_CharacterClassStack.pop();
		return true;
	}
	return false;
}

bool RegEx::Concatenate() {
	std::deque<RegExState*>  LeftExpr, RightExpr;
	if(!PopNFA(RightExpr) || !PopNFA(LeftExpr))
		return false;

	std::deque<RegExState*>::reverse_iterator pLeftItr = LeftExpr.rbegin();
	std::deque<RegExState*>::iterator pRightItr = RightExpr.begin();        

	////printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*pLeftItr))->m_nStateID, ((*pRightItr))->m_nStateID);
	((*pLeftItr))->AddTransition(EPSILON, ((*pRightItr)));

	LeftExpr.insert(LeftExpr.end(), RightExpr.begin(), RightExpr.end());
	m_CharacterClassStack.push(LeftExpr);

	return true;
}

void RegEx::PushOnCharacterStack(char chInput) {

	RegExState *s0 = new RegExState(++m_nNextStateID);
	RegExState *s1 = new RegExState(++m_nNextStateID);

	////printf ( "Transition from [%d] to [%d] on [%c]\n", s0->m_nStateID, s1->m_nStateID, chInput);
	s0->AddTransition(chInput, s1);

	std::deque<RegExState*>  NewExpr;
	NewExpr.push_back(s0);
	NewExpr.push_back(s1);
	m_CharacterClassStack.push(NewExpr);

	m_InputSet.insert(chInput);
}

bool RegEx::ClosureOptional() {

	std::deque<RegExState*> PrevExpr;
	if(!PopNFA(PrevExpr))
		return false;

	RegExState *LeftExpr  = new RegExState(++m_nNextStateID);
	RegExState *RightExpr = new RegExState(++m_nNextStateID);

	std::deque<RegExState*>::reverse_iterator rItr = PrevExpr.rbegin();
	std::deque<RegExState*>::iterator Itr = PrevExpr.begin();

	////printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, RightExpr->m_nStateID);
	LeftExpr->AddTransition(EPSILON, RightExpr);

	////printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, ((*Itr))->m_nStateID);
	LeftExpr->AddTransition(EPSILON, ((*Itr)));

	////printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*rItr))->m_nStateID, RightExpr->m_nStateID);
	(*(rItr))->AddTransition(EPSILON, RightExpr);

	PrevExpr.push_front( LeftExpr );
	PrevExpr.push_back ( RightExpr);

	m_CharacterClassStack.push(PrevExpr);

	return true;
}

bool RegEx::ClosurePlus() {
	std::deque<RegExState*> PrevExpr;
	if(!PopNFA(PrevExpr))
		return false;

	RegExState *LeftExpr  = new RegExState(++m_nNextStateID);
	RegExState *RightExpr = new RegExState(++m_nNextStateID);

	std::deque<RegExState*>::reverse_iterator rItr = PrevExpr.rbegin();
	std::deque<RegExState*>::iterator Itr = PrevExpr.begin();

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, ((*Itr))->m_nStateID);
	LeftExpr->AddTransition(EPSILON, ((*Itr)));

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*rItr))->m_nStateID, RightExpr->m_nStateID);
	(*(rItr))->AddTransition(EPSILON, RightExpr);

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*rItr))->m_nStateID, ((*Itr))->m_nStateID);
	(*(rItr))->AddTransition(EPSILON, ((*Itr)));

	PrevExpr.push_front( LeftExpr );
	PrevExpr.push_back ( RightExpr);

	m_CharacterClassStack.push(PrevExpr);

	return true;
}
bool RegEx::Closure() {

	std::deque<RegExState*> PrevExpr;
	if(!PopNFA(PrevExpr))
		return false;

	RegExState *LeftExpr  = new RegExState(++m_nNextStateID);
	RegExState *RightExpr = new RegExState(++m_nNextStateID);

	std::deque<RegExState*>::reverse_iterator rItr = PrevExpr.rbegin();
	std::deque<RegExState*>::iterator Itr = PrevExpr.begin();

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, RightExpr->m_nStateID);
	LeftExpr->AddTransition(EPSILON, RightExpr);

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, ((*Itr))->m_nStateID);
	LeftExpr->AddTransition(EPSILON, ((*Itr)));

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*rItr))->m_nStateID, RightExpr->m_nStateID);
	(*(rItr))->AddTransition(EPSILON, RightExpr);

	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*rItr))->m_nStateID, ((*Itr))->m_nStateID);
	(*(rItr))->AddTransition(EPSILON, ((*Itr)));

	PrevExpr.push_front( LeftExpr );
	PrevExpr.push_back ( RightExpr);

	m_CharacterClassStack.push(PrevExpr);

	return true;
}

bool RegEx::Or() {

	std::deque<RegExState*>  UpperExpr, LowerExpr;

	//printf ( "Size of Stack = %d\n", m_CharacterClassStack.size());

	if(!PopNFA(LowerExpr) || !PopNFA(UpperExpr))
		return false;

	RegExState *LeftExpr	= new RegExState(++m_nNextStateID);
	RegExState *RightExpr	= new RegExState(++m_nNextStateID);

	std::deque<RegExState*>::iterator UBeginItr = UpperExpr.begin();
	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, ((*UBeginItr))->m_nStateID);
	LeftExpr->AddTransition(EPSILON, ((*UBeginItr)));

	std::deque<RegExState*>::iterator LBeginItr = LowerExpr.begin();
	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", LeftExpr->m_nStateID, ((*LBeginItr))->m_nStateID);
	LeftExpr->AddTransition(EPSILON, ((*LBeginItr)));

	std::deque<RegExState*>::reverse_iterator URIterator = UpperExpr.rbegin();
	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*URIterator))->m_nStateID, RightExpr->m_nStateID);
	((*URIterator))->AddTransition(EPSILON, RightExpr);

	std::deque<RegExState*>::reverse_iterator LRIterator = LowerExpr.rbegin();
	//printf ( "Transition from [%d] to [%d] on [EPSILON]\n", ((*LRIterator))->m_nStateID, RightExpr->m_nStateID);
	((*LRIterator))->AddTransition(EPSILON, RightExpr);

	LowerExpr.push_back(RightExpr);
	UpperExpr.push_front(LeftExpr);
	UpperExpr.insert(UpperExpr.end(), LowerExpr.begin(), LowerExpr.end());

	m_CharacterClassStack.push(UpperExpr);

	std::deque<RegExState*>::iterator itr;
	for ( itr = UpperExpr.begin(); itr != UpperExpr.end(); ++itr) {
		//printf ( "%d - ", (*itr)->m_nStateID);
	}
	//printf ( "\n" );
	return true;
}
void RegEx::EpsilonClosure(std::set<RegExState*> startSet, std::set<RegExState*> &result) {
	std::stack<RegExState*> UnTestedStates;	

	result.clear();	
	result = startSet; // Each State is epsilon closure to itself.

	std::set<RegExState*>::iterator iter;
	for(iter = startSet.begin(); iter != startSet.end(); ++iter) {
		UnTestedStates.push(*iter);
	}

	while(!UnTestedStates.empty()) {
		RegExState* curState = UnTestedStates.top();
		//printf ( "Current State = %d\n", curState->m_nStateID);
		UnTestedStates.pop();

		std::vector<RegExState*> epsilonStates;
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
	//printf ( "Current Input Symbol = %c\n", chInput );

	Result.clear();
	std::set<RegExState*>::iterator iter;
	for(iter = NFAState.begin(); iter != NFAState.end(); ++iter) {
		std::vector<RegExState*> States;
		//printf ( "Checking Transition from [%d] of [%c]\n", (*iter)->m_nStateID, chInput);
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

	std::vector<RegExState*> unmarkedStates;
	std::set<RegExState*> DFAStartStateSet;
	std::set<RegExState*> NFAStartStateSet;

	NFAStartStateSet.insert(m_NFATable[0]);

	EpsilonClosure(NFAStartStateSet, DFAStartStateSet);
	//printf ( "e-CLOSURE({ %d }) = { ", m_NFATable[0]->m_nStateID);
	//std::set<RegExState*>::iterator itr;
	//for ( itr = DFAStartStateSet.begin(); itr != DFAStartStateSet.end(); ++itr ) {
	//	printf ( " %d ", (*itr)->m_nStateID);
	//}
	//printf ( " }\n");

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

	//		printf ( "move( { ");
	//		std::set<RegExState*>::iterator A;
	//		for ( A = CurDFAState->GetNFAState().begin(); A != CurDFAState->GetNFAState().end(); ++A ) {
	//			printf ( "%d ", (*A)->m_nStateID );
	//		}
	//		printf ( "} , %c ) = {", *iter);
	//		std::set<RegExState*>::iterator B;
	//		for ( B = MoveRes.begin(); B != MoveRes.end(); ++B ) {
	//			printf ( "%d ", (*B)->m_nStateID);
	//		}
	//		printf ( " }\n");

			EpsilonClosure(MoveRes, EpsilonClosureRes);

			std::set<RegExState*>::iterator MoveResItr;
			std::set<RegExState*>::iterator EpsilonClosureResItr;
	//		printf ( "e-CLOSURE({");
	//		for (MoveResItr = MoveRes.begin(); MoveResItr != MoveRes.end(); ++MoveResItr) {
	//			printf ( " %d ",(*MoveResItr)->m_nStateID);
	//		}
	//		printf ( "}) = { ");
	//		for (EpsilonClosureResItr = EpsilonClosureRes.begin(); EpsilonClosureResItr != EpsilonClosureRes.end(); ++EpsilonClosureResItr) {
	//			printf ( " %d ",(*EpsilonClosureResItr)->m_nStateID);
	//		}
	//		printf ( "}\n");

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
	std::set<RegExState*>::iterator iter;
	for(iter = DeadEndSet.begin(); iter != DeadEndSet.end(); ++iter) {
		for(int i = 0; i < (int)m_DFATable.size(); ++i)
			m_DFATable[i]->RemoveTransition(*iter);

		std::deque<RegExState*>::iterator pos;
		for(pos=m_DFATable.begin(); pos!=m_DFATable.end(); ++pos) {
			if(*pos == *iter) {
				break;
			}
		}
		m_DFATable.erase(pos);
		delete *iter;
	}
}

bool RegEx::SetRegEx(std::string strRegEx) {
	m_InfixRegEx = const_cast<char*>(strRegEx.c_str());
	CleanUp();
	if(!ConstructThompsonNFA(strRegEx))
		return false;	

	PopNFA( m_NFATable);
	m_NFATable[m_NFATable.size() - 1 ]->m_bAcceptingState = true;

	ConvertNFAtoDFA();
	{
	std::deque<RegExState*>::iterator itr;
	for ( itr = m_DFATable.begin(); itr != m_DFATable.end(); ++itr){
		std::multimap<char, RegExState*>::iterator A;
		for ( A = (*itr)->m_Transition.begin(); A != (*itr)->m_Transition.end(); ++A) {
			char ch = A->first;
			RegExState *S = A->second;
			printf ( "[%d] --- %c -----> ", (*itr)->m_nStateID, ch);
			if ( S->m_bAcceptingState == true ) {
				printf ( " - [[%d]]\n", S->m_nStateID);
			} else {
				printf ( " - [%d]\n", S->m_nStateID);
			}

		}	
	}
	}
	printf ( "===========================\n");

	ReduceDFA();
	{
	std::deque<RegExState*>::iterator itr;
	for ( itr = m_DFATable.begin(); itr != m_DFATable.end(); ++itr){
		std::multimap<char, RegExState*>::iterator A;
		for ( A = (*itr)->m_Transition.begin(); A != (*itr)->m_Transition.end(); ++A) {
			char ch = A->first;
			RegExState *S = A->second;
			printf ( "[%d] --- %c -----> ", (*itr)->m_nStateID, ch);
			if ( S->m_bAcceptingState == true ) {
				printf ( " - [[%d]]\n", S->m_nStateID);
			} else {
				printf ( " - [%d]\n", S->m_nStateID);
			}

		}	
	}
	}
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

		std::vector<RegExState*> Transition;
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
					//printf ( "Size of Stack after Closure = %d\n", m_CharacterClassStack.size());
					break;
				case PLUS_CLOSURE:
					ClosurePlus();
					//printf ( "Size of Stack after ClosurePlus = %d\n", m_CharacterClassStack.size());
					break;
				case OPTIONAL:
					ClosureOptional();
					//printf ( "Size of Stack after ClosureOptional = %d\n", m_CharacterClassStack.size());
					break;
				case OR:
					Or();
					//printf ( "Size of Stack after Or = %d\n", m_CharacterClassStack.size());
					break;
				case FALSE_CONCAT:
					Concatenate();
					//printf ( "Size of Stack after Concatenate = %d\n", m_CharacterClassStack.size());
					break;
			}
		}		
	}
	return true;
}
