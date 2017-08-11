#ifndef _COMMUNITY_CARDS_H_
#define _COMMUNITY_CARDS_H_

#include <vector>
#include <algorithm>

#include "card.h"
#include "hole_cards.h"

using namespace std;

class CommunityCards
{
public:
	CommunityCards();
	
	/*****************************************
		功能：添加一张牌到类成员变量cards中
	******************************************/
	void add_card(Card c);

	/*****************************************
		功能：清空类成员变量cards
	******************************************/
	void clear() { cards.clear(); };
	
	/*****************************************
		功能：拷贝cards的牌输出到v中（向量）
	******************************************/
	void copy_cards(std::vector<Card> *v);

	/*****************************************
		功能：拷贝cards的牌输出到映射中（map）
	******************************************/
	void copy_to_hole_cards(HoleCards &holecards);
	void debug();

	std::vector<Card> cards;
};

#endif /* _COMMUNITY_CARDS_H_ */
