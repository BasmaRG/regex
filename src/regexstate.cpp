#include "regexstate.h"

RegExState::RegExState(std::set<RegExState*> NFAState, int nID) {
	m_NFAStates = NFAState;
	m_nStateID = nID;
	m_bAcceptingState = false;
	std::set<RegExState*>::iterator iter;
	for(iter=NFAState.begin(); iter!=NFAState.end(); ++iter)
		if((*iter)->m_bAcceptingState)
			m_bAcceptingState = true;
}
RegExState::RegExState(const RegExState &other){ 
	*this = other;
}

void RegExState::AddTransition(char inputCh, RegExState *pState){
	m_Transition.insert(std::make_pair(inputCh, pState));
}

void RegExState::RemoveTransition(RegExState* pState) {
	std::multimap<char, RegExState*>::iterator iter;
	for(iter = m_Transition.begin(); iter != m_Transition.end();) {
		RegExState *toState = iter->second;
		if(toState == pState)
			m_Transition.erase(iter++);
		else 
			++iter;
	}
}

void RegExState::GetTransition(char inputCh, std::vector<RegExState*> &States) {
	States.clear();
	std::multimap<char, RegExState*>::iterator iter;
	for(iter = m_Transition.lower_bound(inputCh); iter != m_Transition.upper_bound(inputCh); ++iter) {
		RegExState *pState = iter->second;
		States.push_back(pState);
	}
}

std::set<RegExState*>& RegExState::GetNFAState() { 
	return m_NFAStates; 
}

bool RegExState::IsDeadEnd() {
	if(m_bAcceptingState)
		return false;
	if(m_Transition.empty())
		return true;
	std::multimap<char, RegExState*>::iterator iter;
	for(iter=m_Transition.begin(); iter!=m_Transition.end(); ++iter){
		RegExState *toState = iter->second;
		if(toState != this)
			return false;
	}
	return true;
}	

RegExState& RegExState::operator=(const RegExState& other) { 
	this->m_Transition	= other.m_Transition; 
	this->m_nStateID		= other.m_nStateID;
	this->m_NFAStates	= other.m_NFAStates;
	return *this;
}

bool RegExState::operator==(const RegExState& other) {
	if(m_NFAStates.empty())
		return(m_nStateID == other.m_nStateID);
	else 
		return(m_NFAStates == other.m_NFAStates);
}





