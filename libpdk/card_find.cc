#include "card.h"
#include "card_analysis.h"
#include "card_statistics.h"
#include "card_find.h"

CardFind::CardFind()
{
	m_ntipType=0;
}

void CardFind::clear()
{
	results.clear();
}

int CardFind::find(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &target_card_stat)
{
	clear();

	//用switch时间效率更高，该枚举变量范围取值不大，生成跳转表较小，占用空间小 f0rix 2017年8月8日
	if (card_ana.type != CARD_TYPE_ERROR){
		if (card_ana.type == CARD_TYPE_ONE){
			find_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_TWO){
			find_two(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREE){
			find_three(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_ONELINE){
			find_one_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_TWOLINE){
			find_two_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREELINE){
			find_three_line(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREEWITHONE){
			find_three_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_THREEWITHTWO){
			find_three_with_two(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_PLANEWITHONE){
			find_plane_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_PLANEWITHWING){
			find_plane_with_wing(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHONE){
			find_four_with_one(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHTWO){
			find_four_with_two(card_ana, card_stat, target_card_stat);
		}
		else if (card_ana.type == CARD_TYPE_FOURWITHTHREE){
			find_four_with_three(card_ana, card_stat, target_card_stat);
		}
		find_bomb(card_ana, card_stat, target_card_stat);
	}
	return 0;
}

/*
	找到比card_ana大的单牌集合 并推入 result
*/
void CardFind::find_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_ONE) {
		for (unsigned int i = 0; i < my_card_stat.line1.size(); i++){
			if (my_card_stat.line1[i].face > card_ana.face){
				//如果炸弹不可拆并且该牌为炸弹牌，跳过
				if (!bomb_split && check_is_bomb_card(my_card_stat.line1[i].face, my_card_stat)){
					continue;
				}
				vector<Card> cards;
				cards.push_back(my_card_stat.line1[i]);
				results.push_back(cards);
			}
		}
	}
}


/*
	找到比card_ana大的对子的集合 并推入 result
*/
void CardFind::find_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_TWO){
		for (unsigned int i = 0; i < my_card_stat.line2.size(); i += 2){
			if (my_card_stat.line2[i].face > card_ana.face){
				if (!bomb_split && check_is_bomb_card(my_card_stat.line2[i].face, my_card_stat)){
					continue;
				}
				vector<Card> cards;
				cards.push_back(my_card_stat.line2[i]);
				cards.push_back(my_card_stat.line2[i + 1]);
				results.push_back(cards);
			}
		}
	}
}


/*
	找到所有比card_ana大的三张 并推入 result
*/
void CardFind::find_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_THREE){
		for (unsigned int i = 0; i < my_card_stat.line3.size(); i += 3){
			if (my_card_stat.line3[i].face > card_ana.face){
				if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
					continue;
				}
				vector<Card> cards;
				cards.push_back(my_card_stat.line3[i]);
				cards.push_back(my_card_stat.line3[i + 1]);
				cards.push_back(my_card_stat.line3[i + 2]);
				results.push_back(cards);
			}
		}
	}
}

/*
	检查c_face 是否为炸弹的牌
*/
int CardFind::check_is_bomb_card(int c_face, CardStatistics &card_stat)
{
	for (unsigned int i = 0; i < card_stat.card4.size(); i += 4) {
		if (c_face == card_stat.card4[i].face) {
			return 1;
		}
	}
	return 0;
}


/*
	找到所有比card_ana大的顺子 并推入 result
*/
void CardFind::find_one_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line1.size() - card_stat.line1.size();
	for (int i = 0; i <= count; i++){
		if (my_card_stat.line1[i].face > card_ana.face){
			if (my_card_stat.line1[i].face != 15){
				if (!bomb_split && check_is_bomb_card(my_card_stat.line1[i].face, my_card_stat)){
					continue;
				}
				int end = i + card_ana.len;
				//以i开始，end结束 寻找顺子（type == 1）
				if (card_ana.check_arr_is_line(my_card_stat.line1, 1, i, end)){
					vector<Card> cards;
					for (int j = i; j < end; j++){
						cards.push_back(my_card_stat.line1[j]);
					}
					results.push_back(cards);
				}

			}
		}
	}
}

/*
	找到所有比card_ana大的连对 并推入 result
*/
void CardFind::find_two_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line2.size() - card_stat.line2.size();
	for (int i = 0; i <= count; i += 2){
		if (my_card_stat.line2[i].face > card_ana.face){
			if (!bomb_split && check_is_bomb_card(my_card_stat.line2[i].face, my_card_stat)){
				continue;
			}
			int end = i + card_ana.len;
			if (card_ana.check_arr_is_line(my_card_stat.line2, 2, i, end)){
				vector<Card> cards;
				for (int j = i; j < end; j++){
					cards.push_back(my_card_stat.line2[j]);
				}
				results.push_back(cards);
			}
		}
	}
}

/*
	找到所有比card_ana大的3张顺子 并推入 result
*/
void CardFind::find_three_line(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line3.size() - card_stat.line3.size();
	for (int i = 0; i <= count; i += 3){
		if (my_card_stat.line3[i].face > card_ana.face){
			if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
				continue;
			}
			int end = i + card_ana.len;
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end)){
				vector<Card> cards;
				for (int j = i; j < end; j++){
					cards.push_back(my_card_stat.line3[j]);
				}
				results.push_back(cards);
			}
		}
	}
}

/*
	找到所有比card_ana大的3带1 并推入 result
*/
void CardFind::find_three_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 4){
		return;
	}
	for (unsigned int i = 0; i < my_card_stat.line3.size(); i += 3){
		if (my_card_stat.line3[i].face > card_ana.face){
			if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
				continue;
			}
			vector<Card> cards;
			cards.push_back(my_card_stat.line3[i]);
			cards.push_back(my_card_stat.line3[i + 1]);
			cards.push_back(my_card_stat.line3[i + 2]);
			for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j++) {
				if (my_card_stat.line3[i].face != my_card_stat.all_cards[j].face) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					break;
				}
			}
			results.push_back(cards);
		}
	}
}

/*
	找到所有比card_ana大的3带2 并推入 result
*/
void CardFind::find_three_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 3){
		return;
	}
	for (unsigned int i = 0; i < my_card_stat.line3.size(); i += 3){
		if (my_card_stat.line3[i].face > card_ana.face){
			if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
				continue;
			}
			vector<Card> cards;
			cards.push_back(my_card_stat.line3[i]);
			cards.push_back(my_card_stat.line3[i + 1]);
			cards.push_back(my_card_stat.line3[i + 2]);
			int k = 0;
			for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j++) {
				if (my_card_stat.line3[i].face != my_card_stat.all_cards[j].face) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					k++;
					if (k == 2) {
						break;
					}
				}
			}
			results.push_back(cards);
		}
	}
}

/*
	找到所有比card_ana大的飞机（3带1的飞机） 并推入 result
*/
void CardFind::find_plane_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line3.size() - card_stat.len / 4 * 3;
	for (int i = 0; i <= count; i += 3){
		if (my_card_stat.line3[i].face > card_ana.face){
			int end = i + card_stat.card3.size();
			if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
				continue;
			}
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end)){
				vector<Card> cards;
				for (int j = i; j < end; j++){
					cards.push_back(my_card_stat.line3[j]);
				}
				unsigned int card_size = cards.size();
				unsigned int k = 0;
				for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j++) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					bool flag = false;
					for (unsigned int m = 0; m < card_size; m+=3){
						if (cards[m].face == my_card_stat.all_cards[j].face){
							flag = true;
							break;
						}
					}
					if (flag) {
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					k++;
					if (k == card_size / 3) {
						break;
					}
				}
				results.push_back(cards);
			}
		}
	}
}

/*
	找到所有比card_ana大的飞机（3带对子的飞机） 并推入 result
*/
void CardFind::find_plane_with_wing(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	int count = my_card_stat.line3.size() - card_stat.len / 5 * 3;

	for (int i = 0; i <= count; i += 3){
		if (my_card_stat.line3[i].face > card_ana.face){
			int end = i + card_stat.len / 5 * 3;
			if (!bomb_split && check_is_bomb_card(my_card_stat.line3[i].face, my_card_stat)){
				continue;
			}
			if (card_ana.check_arr_is_line(my_card_stat.line3, 3, i, end)){
				vector<Card> cards;
				for (int j = i; j < end; j++){
					cards.push_back(my_card_stat.line3[j]);
				}
				unsigned int card_size = cards.size();
				unsigned int k = 0;
				for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j++) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					bool flag = false;
					for (unsigned int m = 0; m < card_size; m+=3){
						if (cards[m].face == my_card_stat.all_cards[j].face){
							flag = true;
							break;
						}
					}
					if (flag) {
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					k++;
					if (k == card_size / 3 * 2) {
						break;
					}
				}
				results.push_back(cards);
			}
		}
	}
}

/*
	找到所有比card_ana大的飞机（4带1的牌） 并推入 result
*/
void CardFind::find_four_with_one(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 5){
		return;
	}
	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4){
		if (my_card_stat.card4[i].face > card_ana.face){
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j ++){
				if (my_card_stat.all_cards[j].face != my_card_stat.card4[i].face) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					break;
				}
			}
			results.push_back(cards);
		}
	}
}

/*
	找到所有比card_ana大的飞机（4带2的牌） 并推入 result
*/
void CardFind::find_four_with_two(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 6){
		return;
	}
	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4){
		if (my_card_stat.card4[i].face > card_ana.face){
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			int k = 0;
			for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j ++){
				if (my_card_stat.all_cards[j].face != my_card_stat.card4[i].face) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					k++;
					if (k == 2) {
						break;
					}
				}
			}
			results.push_back(cards);
		}
	}
}

/*
	找到所有比card_ana大的飞机（4带3的牌） 并推入 result
*/
void CardFind::find_four_with_three(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (my_card_stat.len < 7){
		return;
	}
	for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4){
		if (my_card_stat.card4[i].face > card_ana.face){
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			int k = 0;
			for (unsigned int j = 0; j < my_card_stat.all_cards.size(); j ++){
				if (my_card_stat.all_cards[j].face != my_card_stat.card4[i].face) {
					if (!bomb_split && check_is_bomb_card(my_card_stat.all_cards[j].face, my_card_stat)){
						continue;
					}
					cards.push_back(my_card_stat.all_cards[j]);
					k++;
					if (k == 3) {
						break;
					}
				}
			}
			results.push_back(cards);
		}
	}
}

/*
	找到所有比card_ana大的炸弹 并推入 result
*/
void CardFind::find_bomb(CardAnalysis &card_ana, CardStatistics &card_stat, CardStatistics &my_card_stat)
{
	if (card_ana.type == CARD_TYPE_BOMB){
		for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4){
			if (my_card_stat.card4[i].face > card_ana.face){
				vector<Card> cards;
				cards.push_back(my_card_stat.card4[i]);
				cards.push_back(my_card_stat.card4[i + 1]);
				cards.push_back(my_card_stat.card4[i + 2]);
				cards.push_back(my_card_stat.card4[i + 3]);
				results.push_back(cards);
			}
		}
	}
	else{
		for (unsigned int i = 0; i < my_card_stat.card3.size(); i += 3){
			if ((my_card_stat.card3[i].face == Ace && card_ana.three_a_is_bomb) || 
			(my_card_stat.card3[i].face == King && card_ana.three_k_is_bomb)){
				vector<Card> cards;
				cards.push_back(my_card_stat.card3[i]);
				cards.push_back(my_card_stat.card3[i + 1]);
				cards.push_back(my_card_stat.card3[i + 2]);
				results.push_back(cards);
			}
		}
		for (unsigned int i = 0; i < my_card_stat.card4.size(); i += 4){
			vector<Card> cards;
			cards.push_back(my_card_stat.card4[i]);
			cards.push_back(my_card_stat.card4[i + 1]);
			cards.push_back(my_card_stat.card4[i + 2]);
			cards.push_back(my_card_stat.card4[i + 3]);
			results.push_back(cards);
		}
	}
}

/*
	找到输入牌堆中的最长顺子 并输出
*/
int CardFind::find_straight(vector<int> &input, vector<int> &output) // find the longest
{
	if (input.size() == 0)
	{
		return -1;
	}

	vector<Card> input_format; //
	vector<Card> longest_straight; //

	//将输入的牌值转换成牌堆
	for (unsigned int i = 0; i < input.size(); i++) // input format
	{
		Card card(input[i]);
		input_format.push_back(card);
	}

	//将牌堆归类
	CardStatistics input_statistics;
	input_statistics.statistics(input_format);

	//获取归类后牌堆中的最长顺子
	CardFind ::get_straight (input_statistics, longest_straight); // find the longest

	output.clear();

	//转换成牌堆结果
	for (unsigned int i = 0; i < longest_straight.size(); i++)  //output
	{
		output.push_back(longest_straight[i].value);
	}

	return 0;
}


/*
	找到输入牌堆中的最长顺子 并输出
*/
int CardFind::find_straight(vector<Card> &input, vector<Card> &output) // find the longest
{
	if (input.size() == 0)
	{
		return -1;
	}
	output.clear();
	CardStatistics input_statistics;
	input_statistics.statistics(input);
	CardFind::get_straight(input_statistics, output);
	return 0;
}

int CardFind::get_straight(CardStatistics &card_stat, vector<Card> &output) // find the longest
{
	vector<Card> straight_one_longest;
	vector<Card> straight_two_longest;
	vector<Card> straight_three_longest;
	get_longest_straight(card_stat.line1, 1, straight_one_longest); //
	output.clear();
	int cnt = get_max(straight_one_longest.size(), straight_two_longest.size(), straight_three_longest.size()); // the longest
	if (cnt == 1){
		output = straight_one_longest;
	}
	else if (cnt == 2){
		output = straight_two_longest;
	}
	else if (cnt == 3){
		output = straight_three_longest;
	}
	return 0;
}

int CardFind::get_max(unsigned int a, unsigned int b, unsigned int c)
{
	if (c >= a && c >= b && c >= 6){
		return 3;
	}
	if (b >= a && b >= c && b >= 6){
		return 2;
	}
	if (a >= b && a >= c && a >= 5){
		return 1;
	}
	return 0;
}

/*
	获取输入牌堆中的最长顺子，如果一个input中有两个等长的顺子，只会导出第一个结果
*/
void CardFind::get_longest_straight(vector<Card> &input, int type, vector<Card> &output)
{
	unsigned int cnt = 0; // continuous length
	unsigned int last_cnt = 0; // last continuous length
	unsigned int index = 0; // last anti_continuous length
	unsigned int i = 0; // cursor
	Card temp;
	int flag = 0; // 0 continuous,1 anti_continuous
	for (i = 0; i < input.size(); i += type)
	{
		//printf("[%d][%d][%d][%d][%d][%d]\n", type, input[i].face, temp.face, cnt,last_cnt, index);
		if ((input[i].face - temp.face) != 1)
		{
			if (cnt > last_cnt)
			{
				index = i;
				last_cnt = cnt;
			}
			flag = 1;
			cnt = 0;
		}
		else
		{
			if (input[i].face!=15)
			{
				flag = 0;
			}
		}
		cnt += type;
		temp = input[i];
	}

	if (flag == 0)
	{
		if (cnt > last_cnt)		//该处控制了当input中有两个等长的顺子，只会导出第一个结果
		{
			index = i;
			last_cnt = cnt;
		}
	}
	output.clear();
	//printf("copy[%d][%u][%u]\n", type, index - last_cnt, index);
	for (unsigned int i = (index - last_cnt); i < index; i++)
	{
		output.push_back(input[i]);
	}
}


void CardFind::debug()
{
	for (unsigned int i = 0; i < results.size(); i++)
	{
		Card::dump_cards(results[i], "tip");
	}
}


int CardFind::tip(vector<Card> &last, vector<Card> &cur, bool k_is_bomb, bool a_is_bomb)
{
	if (last.size() == 0){
		return -1;
	}
	if (cur.size() == 0){
		return -2;
	}
	clear();

	//将last牌堆归类
	CardStatistics card_stat0;
	card_stat0.statistics(last);

	//分析归类好的牌堆，返回出牌类型
	CardAnalysis card_ana0;
	card_ana0.three_k_is_bomb = k_is_bomb;
	card_ana0.three_a_is_bomb = a_is_bomb;
	card_ana0.analysis(card_stat0);
	if (card_ana0.type == 0){
		return -1;
	}

	//将cur牌堆归类
	CardStatistics card_stat1;
	card_stat1.statistics(cur);
	find(card_ana0, card_stat0, card_stat1);                              
	return 0;
}
