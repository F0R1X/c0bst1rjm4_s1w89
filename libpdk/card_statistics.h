#ifndef _CARD_STATISTICS_H_
#define _CARD_STATISTICS_H_

#include <vector>
#include <string>

using namespace std;

/*
	该类将不同类型的牌进行了存取
	可优化的空间：
	1、统计牌型处是否有更好的算法

	by f0rix 2017年8月8日
*/

class CardStatistics
{
public:
	vector<Card> all_cards;
	//将牌分类
	vector<Card> card1;//存放所有单牌
	vector<Card> card2;//对子
	vector<Card> card3;//三张
	vector<Card> card4;//炸弹

	//所有可能组合 比如三张可以拆成对子
	vector<Card> line1;//存放所有单张组合
	vector<Card> line2;//对子组合
	vector<Card> line3;//三张组合

	unsigned int len;

	CardStatistics();

	/*****************************************
		功能：清空所有的牌堆
	******************************************/
	void clear();

	/*****************************************
		功能：将输入的牌进行统计拆分，
				输入牌堆，按照单双三张，炸弹形式进行拆分，并储存
	******************************************/
	int statistics(std::vector<Card> &cards);

	//功能：打印输出所有的内部成员牌堆
	void debug();
};


#endif /* _CARD_STATISTICS_H_ */
