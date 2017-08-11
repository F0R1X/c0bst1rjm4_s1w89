#ifndef _CARD_FIND_H_
#define _CARD_FIND_H_

#include <vector>
#include <string>

#include "card.h"
#include "card_type.h"
#include "card_analysis.h"
#include "card_statistics.h"


using namespace std;
/*
	该类对主要是对玩家已出的牌的类型从自己的牌堆中寻找更大牌的集合，并放入results中
	
	by f0rix 2017年8月9日
*/
class CardFind
{
public:
	vector<vector<Card> > results;
	CardFind();
	void clear();
	/*****************************************
		功能：根据已有的card_ana出牌类型，从第三个参数my_card_stat中找到比其更大的牌的集合，并推入results中
	******************************************/
	int find(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);

	/*****************************************
		功能：find系列子函数
	******************************************/
	void find_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_one_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_two_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_three_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_plane_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_plane_with_wing(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_four_with_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);
	void find_bomb(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat);

	void debug();

	/*****************************************
		功能：分析last出牌类型，从cur中找到更大的牌的集合，并将结果放入类的成员变量results中
	******************************************/
	int tip(vector<Card> &last, vector<Card> &cur, bool k_is_bomb = false, bool a_is_bomb = false);
	static int find_straight(vector<int> &input, vector<int> &output);
	static int find_straight(vector<Card> &input, vector<Card> &output);
	static int get_straight(CardStatistics &card_stat, vector<Card> &output);
	static int get_max(unsigned int a, unsigned int b, unsigned int c);
	static void get_longest_straight(vector<Card> &input, int type, vector<Card> &output);
	int check_is_bomb_card(int c_face, CardStatistics &card_stat);
	int m_ntipType;

	int bomb_split;//炸弹是否可拆 0--不可拆 1--可拆
private:

};


#endif /* _CARD_FIND_H_ */
