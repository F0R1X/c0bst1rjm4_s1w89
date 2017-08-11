#ifndef _HOLE_CARDS_H_
#define _HOLE_CARDS_H_

#include <map>
#include <vector>
#include <algorithm>

#include "card.h"

using namespace std;

/*
	该类提供了加牌，选牌，自动选牌，选好牌后从手牌中移除已选的牌
	
	by f0rix 2017年8月10日
*/
class HoleCards
{
public:
	HoleCards();

	/*****************************************
		功能：添加一张牌到map映射中（map<牌值花色， 牌>）
	******************************************/
	void add_card(Card c);

	/*****************************************
		功能：清空map牌堆
	******************************************/
	void clear() { cards.clear(); };

	/*****************************************
		功能：遍历map映射中（map<牌值花色， 牌>）的牌，并添加到参数v中，函数对v不进行清理，只推入
	******************************************/
	void copy_cards(std::vector<Card> *v);
	void copy_cards(std::vector<int> *v);

	/*****************************************
		功能：在map牌堆中进行处理，根据参数bomb_is_split炸弹是否可拆，选出map中最大的牌，并放入第一个参数c中
	******************************************/
	void get_one_biggest_card(Card &c, int bomb_is_split);


	/*****************************************
		功能：自动选牌（<int>好像已废弃），3带2,3带1,4带3，对子，最小单，炸弹，顺子中的最大单，按上述顺序出牌
	******************************************/
	int robot(std::vector<int> &v);
	int robot(std::vector<Card> &v);

	/*****************************************
		功能：移除类成员变量cards中已经被选走的牌
	******************************************/
	void remove(std::vector<Card> &v);
	void remove(std::vector<int> &v);

	/*****************************************
		功能：找到类中map容器里是否有value的牌，有返回1，没有返回0
			该算法可优化，通过map find
	******************************************/
	int findCard(int value);

	//int discard(int L,CardType T,std::vector<Card> &v);

	//int trustship(std::vector<Card> &in,std::vector<Card> &out);

	/*****************************************
		功能：返回手牌中的张数
	******************************************/
	int size();

	void debug();

	std::map<int, Card> cards;

	//std::vector<Card> cards;
};

#endif /* _HOLE_CARDS_H_ */
