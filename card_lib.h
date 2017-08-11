#ifndef __CARD_LIB_H__
#define __CARD_LIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <list>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

class CardLib {
public:
	std::map<int,vector<vector<int> > > cards;
	std::map<int,int> cards_index;
	
public:
	CardLib();
	virtual ~CardLib();
	
	void clear() { cards.clear(); };
	
private:
	
};

#endif
