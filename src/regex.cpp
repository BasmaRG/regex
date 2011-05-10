#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include "regex.h"


#define EPSILON       -1 
#define CLOSURE       '*'
#define PLUS_CLOSURE  '+'
#define OPTIONAL      '?'
#define OPEN_PAREN    '('
#define CLOSE_PAREN   ')'
#define OR            '|'
#define FALSE_CONCAT  '&'

#define PRINT_DEBUG 1

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
	#ifdef PRINT_DEBUG
		std::cout << (*(LeftTable.rbegin()))->m_nStateID << " --> " << (*(RightTable.begin()))->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
	#endif

	(*(LeftTable.rbegin()))->AddTransition(EPSILON, (*(RightTable.begin())));
	LeftTable.insert(LeftTable.end(), RightTable.begin(), RightTable.end());
	m_CharacterClassStack.push(LeftTable);

	#ifdef PRINT_DEBUG
		PrintTable( LeftTable);
	#endif

	return true;
}

void RegEx::PushOnCharacterStack(char chInput) {

	RegExState *s0 = new RegExState(m_nNextStateID++);
	RegExState *s1 = new RegExState(m_nNextStateID++);

	#ifdef PRINT_DEBUG
	 std::cout << s0->m_nStateID << " --> " << s1->m_nStateID << " [ " << chInput << " ] " << std::endl;
	#endif
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

	RegExState *LeftTable  = new RegExState(m_nNextStateID++);
	RegExState *RightTable = new RegExState(m_nNextStateID++);

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

	RegExState *LeftTable  = new RegExState(m_nNextStateID++);
	RegExState *RightTable = new RegExState(m_nNextStateID++);

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

	RegExState *LeftTable  = new RegExState(m_nNextStateID++);
	RegExState *RightTable = new RegExState(m_nNextStateID++);

	#ifdef PRINT_DEBUG
		std::cout << LeftTable->m_nStateID << " --> " << RightTable->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << LeftTable->m_nStateID << " --> " << ((*(PrevTable.begin())))->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << ((*(PrevTable.rbegin())))->m_nStateID << " --> " << RightTable->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << ((*(PrevTable.rbegin())))->m_nStateID << " --> " << ((*(PrevTable.begin())))->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
	#endif

	LeftTable->AddTransition(EPSILON, RightTable);
	LeftTable->AddTransition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, RightTable);
	((*(PrevTable.rbegin())))->AddTransition(EPSILON, ((*(PrevTable.begin()))));

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	#ifdef PRINT_DEBUG
		PrintTable( PrevTable);
	#endif

	m_CharacterClassStack.push(PrevTable);

	return true;
}

bool RegEx::Or() {

	Table  UpperTable, LowerTable;

	if(!PopTable(LowerTable) || !PopTable(UpperTable))
		return false;

	RegExState *LeftTable	= new RegExState(m_nNextStateID++);
	RegExState *RightTable	= new RegExState(m_nNextStateID++);
	
	#ifdef PRINT_DEBUG
		std::cout << LeftTable->m_nStateID << " --> " << ((*(UpperTable.begin())))->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << LeftTable->m_nStateID << " --> " << ((*(LowerTable.begin())))->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << ((*(UpperTable.rbegin())))->m_nStateID << " --> " << RightTable->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
		std::cout << ((*(LowerTable.rbegin())))->m_nStateID << " --> " << RightTable->m_nStateID << " [ " << "EPSILON" << " ] " << std::endl;
	#endif

	LeftTable->AddTransition(EPSILON, ((*(UpperTable.begin()))));
	LeftTable->AddTransition(EPSILON, ((*(LowerTable.begin()))));
	((*(UpperTable.rbegin())))->AddTransition(EPSILON, RightTable);
	((*(LowerTable.rbegin())))->AddTransition(EPSILON, RightTable);

	LowerTable.push_back(RightTable);
	UpperTable.insert(UpperTable.begin(),LeftTable);
	UpperTable.insert(UpperTable.end(), LowerTable.begin(), LowerTable.end());

	m_CharacterClassStack.push(UpperTable);

	#ifdef PRINT_DEBUG
		PrintTable ( UpperTable );
	#endif

	return true;
}
void RegEx::EpsilonClosure(std::set<RegExState*> startSet, std::set<RegExState*> &result) {
	std::stack<RegExState*> UnVisitedStates;	
	result.clear();	
	result = startSet; 
	StateIterator iter;
	for(iter = startSet.begin(); iter != startSet.end(); ++iter) {
		UnVisitedStates.push(*iter);
	}

	while(!UnVisitedStates.empty()) {
		RegExState* curState = UnVisitedStates.top();
		UnVisitedStates.pop();

		std::cout << "Current State " <<  curState->m_nStateID << std::endl;
		Table epsilonStates;
		curState->GetTransition(EPSILON, epsilonStates);

		TableIterator epsilonItr;

		for ( epsilonItr = epsilonStates.begin(); epsilonItr != epsilonStates.end(); ++epsilonItr ){
			std::cout << "State = " << (*epsilonItr)->m_nStateID << std::endl;
			if(result.find((*epsilonItr)) == result.end()) {
				result.insert((*epsilonItr));
				UnVisitedStates.push((*epsilonItr));
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

//	PrintTable( m_NFATable);
	if(m_NFATable.size() == 0)
		return;
	
	std::set<RegExState*> NFAStartStateSet;
	NFAStartStateSet.insert(m_NFATable[0]);

	std::set<RegExState*> DFAStartStateSet;
	EpsilonClosure(NFAStartStateSet, DFAStartStateSet);

	std::set<RegExState*>::iterator iter;
	for ( iter = DFAStartStateSet.begin(); iter != DFAStartStateSet.end(); ++iter )
		std::cout << (*iter)->m_nStateID << "\t" ;
	std::cout << std::endl;

	m_nNextStateID = 0;
	RegExState *DFAStartState = new RegExState(DFAStartStateSet, m_nNextStateID++);

	m_DFATable.push_back(DFAStartState);

	Table UnVisitedStates;
	UnVisitedStates.push_back(DFAStartState);
	while(!UnVisitedStates.empty()) {

		RegExState* CurDFAState = UnVisitedStates[UnVisitedStates.size()-1];
		UnVisitedStates.pop_back();

		std::set<char>::iterator iter;
		for(iter = m_InputSet.begin(); iter != m_InputSet.end(); ++iter) {
			std::set<RegExState*> MoveRes, EpsilonClosureRes;
			Move(*iter, CurDFAState->GetNFAState(), MoveRes);

			std::set<RegExState*>::iterator itr;
			for ( itr = MoveRes.begin(); itr != MoveRes.end(); ++itr) 
				std::cout << (*itr)->m_nStateID << "\t" ;
			std::cout << std::endl;


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
				RegExState* U = new RegExState(EpsilonClosureRes, m_nNextStateID++);
				UnVisitedStates.push_back(U);
				m_DFATable.push_back(U);                                
				CurDFAState->AddTransition(*iter, U);
			} else {
				CurDFAState->AddTransition(*iter, s);
			}
		}
	}	
	PrintTable(m_DFATable);
	ReduceDFA();
	PrintTable(m_DFATable);
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

void RegEx::MinimizeDFA () {
}

bool RegEx::SetRegEx(std::string strRegEx) {
	m_InfixRegEx = const_cast<char*>(strRegEx.c_str());
	CleanUp();
	if(!ConstructThompsonNFA())
		return false;	

	PopTable( m_NFATable);
	m_NFATable[m_NFATable.size() - 1 ]->m_bAcceptingState = true;
	
	PrintTable( m_NFATable);

	ConvertNFAtoDFA();
	PrintTable(m_DFATable);
	MinimizeDFA();
	return true;
}

bool RegEx::Match(std::string strText) {
	m_strText = strText;
	if(Find()) {
		return true;
	}
	return false;
}

bool RegEx::Find() {
	if(m_DFATable.empty())
		return false;

	RegExState *pState = m_DFATable[0];
	for(int i = 0; i < (int)m_strText.size(); ++i) {

		char c = m_strText[i]; // Take the first character

		Table Transition;
		pState->GetTransition(c, Transition);
		if(!Transition.empty()) {
			if(Transition[0]->m_bAcceptingState) {
				return true;
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
bool RegEx::ConstructThompsonNFA() {
	CovertToPostfix();
	for(int i = 0; i < (int)m_PostStrRegEx.size(); ++i) {
		char curChar = m_PostStrRegEx[i];
		std::cout << "Current Processing " << curChar << std::endl;
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


void RegEx::PrintTable(Table &table) {
	std::string TableString;
	for(int i = 0; i < (int)table.size(); ++i) {
		RegExState *pState = table[i];
		if(pState->m_bAcceptingState) {
			TableString += "\t" + pState->getStringID() + "\t\n";
		}
	}
	TableString += "\n";
	for( int i = 0; i < (int)table.size(); ++i) {
		RegExState *pState = table[i];
		std::vector<RegExState*> State;
		pState->GetTransition(EPSILON, State);
		for(int j = 0; j < (int)State.size(); ++j) {
			TableString += "\t" + pState->getStringID() + " -> " + State[j]->getStringID();
			TableString += "\t[label = \"epsilon \"]\n";
		}
		std::set<char>::iterator iter;
		for(iter = m_InputSet.begin(); iter != m_InputSet.end(); ++iter) {
			pState->GetTransition(*iter, State);
			for(int j = 0; j < (int)State.size(); ++j) {
				TableString += "\t" + pState->getStringID() + " -> " + State[j]->getStringID();
				std::stringstream out;
				out << *iter;
				TableString += "\t[label = \"" + out.str() + " \"]\n";
			}
		}
	}
	std::cout << TableString;
}
      
