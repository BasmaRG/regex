#ifndef _REGEXSTATE_H_
#define _REGEXSTATE_H_

#include <map>
#include <set>
#include <vector>
#include <string>

class RegExState{
private:
	std::multimap<char, RegExState*> m_Transition;	
	std::set<RegExState*> m_NFAStates;
	
public:

	int m_nStateID;

	bool m_bAcceptingState;

	RegExState() : m_nStateID(-1), m_bAcceptingState(false) {};
	RegExState(int nID) : m_nStateID(nID), m_bAcceptingState(false) {};
	RegExState(std::set<RegExState*> NFAState, int nID);
	RegExState(const RegExState &other);

	virtual ~RegExState() {};

	void AddTransition(char inputCh, RegExState *pState);
	void RemoveTransition(RegExState* pState);
	void GetTransition(char inputCh, std::vector<RegExState*> &States);
	std::set<RegExState*>& RegExState::GetNFAState();

	bool IsDeadEnd();
	RegExState& operator=(const RegExState& other);
	bool operator==(const RegExState& other);
protected:
};
#endif