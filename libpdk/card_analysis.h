#ifndef _CARD_ANALYSIS_H_
#define _CARD_ANALYSIS_H_

#include <vector>
#include <string>

#include "card_statistics.h"

using namespace std;

/*
	该类对主要是对玩家出的牌进行归类（依靠card_statistics）后再分析（本类中的analysis函数），
	并提供了获取牌类型get_card_type 和 比较两个玩家的牌大小（是否胜出）功能 isGreater

	可优化的空间：
	1、analysis 返回的是牌的类型，此处用int类型返回，有可能会引起误会，最好直接返回参数为枚举类型
	2、判断是否连排的check只在本类调用，应隐藏其实现，改为private类型，以及其它可以选择私有的函数
	3、某些函数的形参并未在函数中改变，针对这些函数，可以明确指定为const& 类型

	by f0rix 2017年8月8日
*/
class CardAnalysis
{
public:
	unsigned int len;			//
	int type;					//牌类型
	int face;					//牌值
	bool three_k_is_bomb;		//三K是否为炸弹
	bool three_a_is_bomb;		//三A是否为炸弹
	//int line_count; // x=line_count;x���ԣ�x��˳
	CardAnalysis();

	/*****************************************
		！该功能未实现
	******************************************/
	void clear();

	/*****************************************
		功能：对打出的牌分析，返回出牌类型
	******************************************/
	int analysis(CardStatistics &card_stat);
	
	/*****************************************
		！该功能未实现
	******************************************/
	int analysis(vector<int> &cards,unsigned int handsno);

	/*****************************************
		功能：检查是否为连排，顺子，连对，飞机，根据第二个形参判断
	******************************************/
	bool check_is_line(CardStatistics &card_stat, int line_type);
	bool check_arr_is_line(std::vector<Card> &line, int line_type);
	bool check_arr_is_line(std::vector<Card> &line, int line_type, unsigned int begin, unsigned int end);

	/*****************************************
		功能：比较两个牌型的大小，*this大则为true，其余false
	******************************************/
	bool compare(CardAnalysis &card_analysis, int last_hand);

	/*****************************************
		功能：输出当前牌型和大小
				格式：type: CARD_TYPE_THREEWITHONE  face: 3
	******************************************/
	void debug();

	/*****************************************
		功能：格式化牌型，输出牌值花色向量 炸弹，3张，双，单 由大到小序列
	******************************************/
	static void format(CardStatistics &stat, vector<int> &cur);

	/*****************************************
		功能：格式化牌型，输出牌对象向量 炸弹，3张，双，单 由大到小序列
	******************************************/
	static void format(CardStatistics &stat, vector<Card> &cur);

	/*****************************************
		功能：比较last和cur的牌值大小
				last错误返回-1，cur错误返回-2，
				cur胜出则对cur大到小排序，返回1，
				否则不对cur排序，返回0
	******************************************/
	static int isGreater(vector<int> &last, vector<int> &cur, int *card_type, int handsno);

	/*****************************************
		功能：比较last和cur的牌值大小
				last错误返回-1，cur错误返回-2，
				cur胜出则对cur大到小排序，返回1，
				否则不对cur排序，返回0
			注意！与上面函数对比，除了形参的不同，还增加了3K，3A是否炸弹这一玩法的判断
	******************************************/
	static int isGreater(vector<Card> &last, vector<Card> &cur, int *card_type, int handsno, bool k_is_bomb = false, bool a_is_bomb = false);

	/*****************************************
		功能：判断牌类型并返回
	******************************************/
	static int get_card_type(vector<int> &input);

	/*****************************************
		功能：判断牌类型并返回
			注意！与上面函数对比，除了形参的不同，还增加了3K，3A是否炸弹这一玩法的判断
	******************************************/
	static int get_card_type(vector<Card> &input, bool k_is_bomb = false, bool a_is_bomb = false);

	/*****************************************
		！该功能未实现
	******************************************/
	static void test(int input[], int len);

	/*****************************************
		！该功能未实现
	******************************************/
	static void test(int input0[], int len0, int input1[], int len1);
};


#endif /* _CARD_STATISTICS_H_ */
