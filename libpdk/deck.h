#ifndef _DECK_H_
#define _DECK_H_

#include <vector>

#include "card.h"
#include "hole_cards.h"
//#include "community_cards.h"


using namespace std;

class Deck
{
public:

	/*****************************************
		功能：填充45张牌到cards（函数并未对cards进行清理工作），如果type==16，则再填充3张牌
	******************************************/
	void fill();

	/*****************************************
		功能：清空cards
	******************************************/
	void empty();

	/*****************************************
		功能：返回cards大小
	******************************************/
	int count() const;
	
	/*****************************************
		功能：将card压入cards
	******************************************/
	bool push(Card card);

	/*****************************************
		功能：弹出cards的尾部牌到card，有可能失败，失败不改变card值
	******************************************/
	bool pop(Card &card);

	/*****************************************
		功能：弹出cards的尾部牌到card，有可能失败，失败不改变card值
	******************************************/
	bool remove_d(Card &card);

	/*****************************************
		功能：洗牌，对cards的牌进行洗牌，一种加seed，一种不加
	******************************************/
	bool shuffle(int seed);
	bool shuffle();

	/*****************************************
		功能：在[start, end]的连续整型区间随机取一个数字并返回
	******************************************/
	int random_num(int start, int end, int seed);

	/*****************************************
		功能：先计算holecards的张数，根据玩法（type规定了holecards最多有几张），将cards打乱，往holecards推入牌，直到满足holecards的牌数==type
	******************************************/
	void get_hole_cards(HoleCards &holecards);
	//void make_cards(HoleCards &holecards, int type);

	/*****************************************
		功能：输出cards所有牌
	******************************************/
	void debug();

	/*****************************************
		功能：debug功能，归类cards，并调用debug()
	******************************************/
	void makecards();

	/*****************************************
		！功能：该功能未实现
	******************************************/
	void make_card();
	//void make_plane(HoleCards &holecards);

	/*****************************************
		功能：在cards中取出炸弹（有可能取到一半holecards就满14张了，所以会取到一半就退出）
	******************************************/
	void make_boom(HoleCards &holecards);

	/*****************************************
		功能：在cards中取出5位的顺子（有可能取到一半holecards就满14张了，所以会取到一半就退出）
	******************************************/
	void make_one_line(HoleCards &holecards);
	//void make_two_line(HoleCards &holecards);
	//void make_three_line(HoleCards &holecards);

	/*****************************************
		功能：在cards中取出3张（有可能取到一半holecards就满14张了，所以会取到一半就退出）
			此函数名为取3带2，其实只是取了3张牌，等于3带0
	******************************************/
	void make_three_two(HoleCards &holecards);

	/*****************************************
		功能：在cards中取出一系列对子（有可能取到一半holecards就满14张了，所以会取到一半就退出）
			此函数名为取对子，但是其实取了不止一个对子，一直取对子，知道holecards满了14张为止
	******************************************/
	void make_double(HoleCards &holecards);

	/*****************************************
		功能：在cards中随机取出一张单牌
	******************************************/
	void make_one(HoleCards &holecards);

public:
	int type;		//最大可持有牌数
	vector<Card> cards;
	vector<Card> goodcards;

private:
	void make_plane();
	void make_boom();
	void make_one_line();
	void make_two_line();
	void make_three_line();
	void make_three_two();
	void make_double();
};

#endif /* _DECK_H_ */
