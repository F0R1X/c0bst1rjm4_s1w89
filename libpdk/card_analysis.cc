#include <iostream>
#include "card.h"
#include "card_type.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_find.h"

using namespace std;

static char *card_type_str[] =
{
	(char*)"CARD_TYPE_ERROR",
	(char*)"CARD_TYPE_ONE", 	// 单牌
	(char*)"CARD_TYPE_ONELINE", // 顺子（单连，至少5张，范围3-A，不包括2）
	(char*)"CARD_TYPE_TWO", 	// 对子
	(char*)"CARD_TYPE_TWOLINE", // 连对（至少4张，3-A，不包括2）
	(char*)"CARD_TYPE_THREE", 	// 3张（只有没有其他牌的时候，仅只剩3张且3张相等的时候可用）only if no more cards can take '3+0'
	(char*)"CARD_TYPE_THREELINE",		// 飞机（至少6张，3-A，不包括2）
	(char*)"CARD_TYPE_THREEWITHONE",	// 3带1 only if no more cards can take '3+1'
	(char*)"CARD_TYPE_THREEWITHTWO",	// 3带2 only if no more cards can take '3+1' or '3+0'
	//(char*)"CARD_TYPE_PLANELINE",//plane(at least eight cards);can start form '3' to 'A'(not '2');can be '3n+n' or '3n+2n'
	(char*)"CARD_TYPE_PLANEWITHONE",	// 飞机带羿，多个三带一,牌不够时出
	(char*)"CARD_TYPE_PLANEWITHWING",	// 飞机带翅，多个三带二
	(char*)"CARD_TYPE_PLANEWHITHLACK",	// 飞机带少或不带
	(char*)"CARD_TYPE_FOURWITHONE",  	// 4带1（牌不够时可以少带，带1-2张）
	(char*)"CARD_TYPE_FOURWITHTWO", 	// 4带2（牌不够时可以少带，带1-2张）
	(char*)"CARD_TYPE_FOURWITHTHREE", 	// 4张牌也可以带3张其他牌，这时不算炸弹
	(char*)"CARD_TYPE_BOMB" 			// 炸弹
};

CardAnalysis::CardAnalysis()
{

}

int CardAnalysis::analysis(CardStatistics &card_stat)//analysize the card type
{
	type = CARD_TYPE_ERROR;

	len = card_stat.len;

	//牌堆没牌，返回错误类型
	if (len == 0)
	{
		return type;
	}

	//牌堆只有一张牌，牌值存放在类的face中后，返回单牌类型
	if (len == 1)
	{
		face = card_stat.card1[0].face;
		type = CARD_TYPE_ONE; // 3
		return type;
	}

	//牌堆有两张且为对子，牌值存放在类的face中后，返回对子类型
	if (len == 2)
	{
		if (card_stat.line1.size() == 1
			&& card_stat.card2.size() == 2)
		{
			face = card_stat.card2[1].face;
			type = CARD_TYPE_TWO; // 33
			return type;
		}
	}

	//牌堆有三张且为三张相等，如果是3个K或3个A，且模式3K,3A为炸弹，则返回炸弹，设定面值为0，否则返回3张
	//疑问？3K和3A是不区分大小的么？正常3A应该大于3K，除非只有一种情况，在选牌的时候3K和3A是炸弹只能2选1，不可能同时存在3K，3A都为炸弹这种情况
	if (len == 3)
	{
		if (card_stat.card3.size() == 3)
		{
			if((card_stat.card3[0].face == King && three_k_is_bomb)
			 ||	(card_stat.card3[0].face == Ace && three_a_is_bomb)){
				face = 0;
				len = 4;
				type = CARD_TYPE_BOMB; // KKK
				return type;
			}
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREE; // 333
			return type;
		}
	}
	// if(len > 3 && card_stat.card3.size() >= 3){
	// 	for(size_t i = 0; i < card_stat.card3.size(); i+=3){
	// 		if((card_stat.card3[i].face == Ace && three_a_is_bomb ) || (card_stat.card3[0].face == King && three_k_is_bomb)){
	// 			return type;
	// 		}
	// 	}
	// }

	//牌堆有四张且为四张相等，设定面值，返回炸弹，如果是3张相等，带1，判定3带1
	//疑问？如果是2个对子呢？或者4张单牌呢？除非这个类是在已经选好牌出牌阶段进行判定，否则此处有漏洞，另外如果是出连对呢？
	if (len == 4)
	{
		if (card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_BOMB; // 3333
			return type;
		}
		else if (card_stat.card1.size() == 1 && card_stat.card3.size() == 3){
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHONE; // 333 4
			return type;
		}

	}

	//牌堆有5张牌，3带2？ 3带2单或3带1对
	//另，该面值只是存储了3个的面值，应该该种玩法只有1副牌，
	//不可能存在2组同面值的3带，所以可以不需要对带的对子或单牌进行判断
	//屏蔽了4带1
	//疑问？没有判断顺子 （后面有判断）
	if (len == 5)
	{
		if (card_stat.card2.size() == 2
			&& card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHTWO; // 333 44
			return type;
		}
		//屏蔽四带一
		// else if (card_stat.card4.size() == 4
		// 		&& card_stat.card1.size() == 1)
		// {
		// 	face = card_stat.card4[3].face;
		// 	type = CARD_TYPE_FOURWITHONE; // 3333 4
		// 	return type;
		// }
		else if (card_stat.card1.size() == 2
				&& card_stat.card3.size() == 3)
		{
			face = card_stat.card3[2].face;
			type = CARD_TYPE_THREEWITHTWO; // 3333 4 5
			return type;
		}


	}

	//如果是6张，4带1对 ？ 4带2单？
	//疑问？有没有可能是顺子？345678？ 没有进行判断	（后面有进行判断）
	//疑问？对飞机也没有进行判断 444555
	//疑问？或者连对，也是没进行判断 334455			（后面有进行判断）
	if (len == 6)
	{
		if (card_stat.card1.size() == 2
			&& card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTWO; // 3333 4 5
			return type;
		}
		else if (card_stat.card2.size() == 2
			&& card_stat.card4.size() == 4)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTWO; // 3333 44
			return type;
		}

	}

	//如果是7张 4带3单牌？ 4带1单1对？ 4带3张？
	//疑问，是不是顺子？没进行判断（后面有判断顺子）
	if (len == 7)
	{
		if (card_stat.card4.size() == 4
			&& card_stat.card1.size() == 3)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTHREE; // 3333 4 5 6
			return type;
		}
		else if (card_stat.card4.size() == 4
				&& card_stat.card1.size() == 1
				&& card_stat.card2.size() == 2)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTHREE; // 3333 4 55
			return type;
		}
		else if (card_stat.card4.size() == 4
			&& card_stat.card3.size() == 3)
		{
			face = card_stat.card4[3].face;
			type = CARD_TYPE_FOURWITHTHREE; // 3333 444
			return type;
		}
	}

	//检测是不是顺子
	if (
		card_stat.card1.size() == card_stat.line1.size()
		&& card_stat.card2.size() == 0
		&& card_stat.card3.size() == 0
		&& card_stat.card4.size() ==0 ) 
	{
		if (check_is_line(card_stat, 1)) {
			face = card_stat.card1[0].face;
			type = CARD_TYPE_ONELINE;
			return type;
		}
	}

	//检测是不是连对
	if (len == card_stat.card2.size()
		&& card_stat.card2.size() == card_stat.line2.size()) {
		if (check_is_line(card_stat, 2)) {
			face = card_stat.card2[0].face;
			type = CARD_TYPE_TWOLINE;
			return type;
		}
	}

	//小于5张，返回牌型错误
	//疑问？在这里判断是否已经晚了，前面的顺子有没有判断长度？
	if (len < 5)
	{
		return type;
	}

	//5张以上，飞机判断
	unsigned int left_card_len;
	if (card_stat.card3.size() == card_stat.line3.size()
		&& card_stat.card4.size() == 0 && card_stat.card3.size() != 0)
	{
		if (check_is_line(card_stat, 3))
		{
			left_card_len = card_stat.card1.size() + card_stat.card2.size();
			//飞机不带任何牌
			if (left_card_len == 0)
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_THREELINE;
				return type;

			}
			//3带1的飞机
			else if (left_card_len * 3 == card_stat.card3.size())
			{
				face = card_stat.card3[0].face;
				type = CARD_TYPE_PLANEWITHONE;//555 666 8 9
				return type;
			}
			//3带2的飞机
			//else if (card_stat.card1.size() == 0 && left_card_len * 3 == card_stat.card3.size() * 2)
			else if (left_card_len * 3 == card_stat.card3.size() * 2)//20140819
			{
				cout<<"CARD_TYPE_PLANEWITHWING"<<endl;
				face = card_stat.card3[0].face;
				type = CARD_TYPE_PLANEWITHWING;//555 666 88 99
				return type;
			}
		}

	}

	if (card_stat.card3.size() != 0 && check_arr_is_line(card_stat.card3, 3))
	{
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card4.size();
		cout<<"left_card_len"<<left_card_len<<endl;
		if (left_card_len * 3 == card_stat.card3.size()) {
			cout<<"if (left_card_len * 3 == card_stat.card3.size()) {"<<endl;
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			type = CARD_TYPE_PLANEWITHONE; // 333 444 555 666 9 9 9 9
			return type;
		}
		//else if (card_stat.card1.size() == 0 && left_card_len * 3 == card_stat.card3.size() * 2)
		else if (left_card_len * 3 == card_stat.card3.size() * 2)//20140819
		{
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			type = CARD_TYPE_PLANEWITHWING; // 333 444 99 99
			return type;
		}
		//in this situation:the last hands has not enough cards
		else if (left_card_len * 3 < card_stat.card3.size() * 2)
		{
			face = card_stat.card3[card_stat.card3.size() - 1].face;
			type = CARD_TYPE_PLANEWHITHLACK; // 333 444 9;333 444 8 99;333444 7 8 9
			return type;
		}
	}
	if (card_stat.line3.size()>=6)//bug 444 555 666 888 739  can not recognize
	{
		vector<Card> straight_three_longest;
		unsigned int cnt = 0; // continues length
		unsigned int last_cnt = 0; //last time continues length
		unsigned int index = 0; // last time anti_continues length
		unsigned int i = 0; // cursor
		Card temp;
		int flag = 0; // 0 continues,1 anti_continues

		//求出连3的界限
		for (i = 0; i < card_stat.line3.size(); i += 3)
		{
			//printf("[3][%d][%d][%d][%d][%d]\n",  card_stat.line3[i].face, temp.face, cnt,last_cnt, index);
			if ((card_stat.line3[i].face - temp.face) != 1)
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
				if (card_stat.line3[i].face!=15)
				{
					flag = 0;
				}
			}
			cnt += 3;
			temp = card_stat.line3[i];
		}

		if (flag == 0)
		{
			if (cnt > last_cnt)
			{
				index = i;
				last_cnt = cnt;
			}
		}

		//将单独的连3推入组内
		straight_three_longest.clear();
		for (unsigned int i = (index - last_cnt); i < index; i++)
		{
			straight_three_longest.push_back(card_stat.line3[i]);
		}

		//记录连3牌数
		unsigned int len1=straight_three_longest.size();
		//记录所有牌数
		left_card_len = card_stat.card1.size() + card_stat.card2.size() + card_stat.card3.size() + card_stat.card4.size();
		//记录连3带牌数
		unsigned int len2 = left_card_len-len1;
		//飞机带2
		if ((len1*2)==(len2*3))
		{
			face = straight_three_longest[len1-1].face;
			type = CARD_TYPE_PLANEWITHWING; // 333 444 6 7 3
			return type;
		}
		//连3少带
		else if ((len1*2) > (len2*3)) {
			face = straight_three_longest[len1-1].face;
			type = CARD_TYPE_PLANEWHITHLACK; // 333 444 6 7 3
			return type;
		}
		//由于一共是48张牌，333 444 555 666 888 9 这种情况一旦出现，牌已经出完，所以无需判断3带1的情形
	}
	return type;
}

bool CardAnalysis::check_is_line(CardStatistics &card_stat, int line_type)
{
	if (line_type == 1)
	{
		return check_arr_is_line(card_stat.line1, line_type);
	}
	else if (line_type == 2)
	{
		return check_arr_is_line(card_stat.line2, line_type);
	}
	else if (line_type == 3)
	{
		return check_arr_is_line(card_stat.line3, line_type);
	}

	return false;
}

bool CardAnalysis::check_arr_is_line(std::vector<Card> &line, int line_type)
{
	return check_arr_is_line(line, line_type, 0, line.size());
}

bool CardAnalysis::check_arr_is_line(std::vector<Card> &line, int line_type, unsigned int begin, unsigned int end)
{
	int len = 1;
	Card card = line[begin];
	for (unsigned int i = (line_type + begin); i < end; i += line_type)
	{
		if ((card.face + 1) == line[i].face && line[i].face != 15) { // 2 is not straight (Line)
			len++;
			card = line[i];
		}
		else
		{
			return false;
		}
	}

	if (line_type == 1 && len > 4)
		return true;
	else if (line_type == 2 && len >= 1)
		return true;
	else if (line_type == 3 && len > 1)
		return true;

	return false;
}

bool CardAnalysis::compare(CardAnalysis &card_analysis, int last_hand) //  win : true
{
	printf("compare type[%s] len[%d] face[%d] vs type[%s] len[%d] face[%d]\n",
		   card_type_str[type], len, face, card_type_str[card_analysis.type], card_analysis.len, card_analysis.face);
	
	//如果牌型错误，返回错误
	if (card_analysis.type == CARD_TYPE_ERROR){
		return false;
	}

	//如果最后一手
	if (last_hand) {
		//如果是3带2
		if (card_analysis.type == CARD_TYPE_THREEWITHTWO) {
			if (type == CARD_TYPE_THREEWITHTWO) {
				if (face > card_analysis.face) {
					return true;
				}
			}
		}
		//如果多个3带2
		else if (card_analysis.type == CARD_TYPE_PLANEWITHWING) {
			if (type == CARD_TYPE_PLANEWITHONE || type == CARD_TYPE_PLANEWHITHLACK) {
				if (len >= (card_analysis.len / 5 * 3) && len <= card_analysis.len) {
					if (face > card_analysis.face) {
						return true;
					}
				}
			}
		}
	}

	if (type == card_analysis.type && len == card_analysis.len){
		if (face > card_analysis.face){
			return true;
		}
	}
	else{
		if (type == CARD_TYPE_BOMB){
			return true;
		}
	}
	return false;
}

void CardAnalysis::debug()
{
	cout << "type: " << card_type_str[type] << " face: " << face << endl;
}

void CardAnalysis::format(CardStatistics &stat, vector<int> &cur)
{
	int len;
	cur.clear();

	len = stat.card4.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card4[i].value);
	}

	len = stat.card3.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card3[i].value);
	}

	len = stat.card2.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card2[i].value);
	}

	len = stat.card1.size() - 1;
	for (int i = len; i >= 0; i--)
	{
		cur.push_back(stat.card1[i].value);
	}
}

void CardAnalysis::format(CardStatistics &stat, vector<Card> &cur)
{
	int len;
	cur.clear();
	len = stat.card4.size() - 1;
	for (int i = len; i >= 0; i--){
		Card card(stat.card4[i].value);
		cur.push_back(card);
	}
	len = stat.card3.size() - 1;
	for (int i = len; i >= 0; i--){
		Card card(stat.card3[i].value);
		cur.push_back(card);
	}
	len = stat.card2.size() - 1;
	for (int i = len; i >= 0; i--){
		Card card(stat.card2[i].value);
		cur.push_back(card);
	}
	len = stat.card1.size() - 1;
	for (int i = len; i >= 0; i--){
		Card card(stat.card1[i].value);
		cur.push_back(card);
	}
}

int CardAnalysis::isGreater(vector<int> &last, vector<int> &cur, int *card_type, int handsno)
{
	if (last.size() == 0)
	{
		return -1;
	}

	if (cur.size() == 0)
	{
		return -2;
	}

	//生成牌堆 last
	vector<Card> cards0;//copy the last cards
	for (unsigned int i = 0; i < last.size(); i++)
	{
		Card card(last[i]);
		cards0.push_back(card);
	}

	//解析牌堆 归类 last
	CardStatistics card_stat0;
	card_stat0.statistics(cards0);

	//判断出牌类型
	CardAnalysis card_ana0;
	card_ana0.analysis(card_stat0);

	//如果err，返回错误
	if (card_ana0.type == 0)
	{
		return -1;
	}

	//生成牌堆 cur
	vector<Card> cards1;//copy current cards
	for (unsigned int i = 0; i < cur.size(); i++)
	{
		Card card(cur[i]);
		cards1.push_back(card);
	}

	//解析牌堆 归类 cur
	CardStatistics card_stat1;
	card_stat1.statistics(cards1);

	//判断出牌类型 cur
	CardAnalysis card_ana1;
	card_ana1.analysis(card_stat1);

	//如果err，返回错误
	if (card_ana1.type == 0)
	{
		return -2;
	}

	//将cur牌分析结果（牌类型）传给形参card_type
	*card_type = card_ana1.type;


	bool res = false;

	//出牌数等于手牌数(最后一手)
	if (card_stat1.len == (unsigned int)handsno) {
		res = card_ana1.compare(card_ana0, 1);
	}
	//不是最后一手
	else{
		res = card_ana1.compare(card_ana0, 0);
	}

	//如果cur 胜出，格式化输出牌型，返回1，否则返回0
	if (res)
	{
		CardAnalysis::format(card_stat1, cur);
		return 1;
	}
	else
	{
		return 0;
	}
}

int CardAnalysis::isGreater(vector<Card> &last, vector<Card> &cur, int *card_type, int handsno, bool k_is_bomb, bool a_is_bomb)
{
	if (last.size() == 0){
		return -1;
	}
	if (cur.size() == 0){
		return -2;
	}

	//解析牌堆，归类 last
	CardStatistics card_stat0;
	card_stat0.statistics(last);

	//判断出牌类型
	CardAnalysis card_ana0;
	card_ana0.three_k_is_bomb = k_is_bomb;
	card_ana0.three_a_is_bomb = a_is_bomb;
	card_ana0.analysis(card_stat0);
	if (card_ana0.type == 0){
		return -1;
	}

	//解析牌堆，归类 cur
	CardStatistics card_stat1;
	card_stat1.statistics(cur);

	//判断出牌类型
	CardAnalysis card_ana1;
	card_ana1.three_k_is_bomb = k_is_bomb;
	card_ana1.three_a_is_bomb = a_is_bomb;
	card_ana1.analysis(card_stat1);
	if (card_ana1.type == 0){
		return -2;
	}

	//将cur牌分析结果（牌类型）传给形参card_type
	*card_type = card_ana1.type;
	bool res = false;

	//出牌数等于手牌数(最后一手)
	if (card_stat1.len == (unsigned int)handsno) {
		res = card_ana1.compare(card_ana0, 1);
	}
	//不是最后一手
	else{
		res = card_ana1.compare(card_ana0, 0);
	}

	//如果cur 胜出，格式化输出牌型，返回1，否则返回0
	if (res){
		CardAnalysis::format(card_stat1, cur);
		return 1;
	}
	else{
		return 0;
	}
}

int CardAnalysis::get_card_type(vector<int> &input)
{
	if (input.size() == 0)
	{
		return 0;
	}

	vector<Card> cards;
	for (unsigned int i = 0; i < input.size(); i++)
	{
		Card card(input[i]);
		cards.push_back(card);
	}
	CardStatistics card_stat;
	card_stat.statistics(cards);
	CardAnalysis card_ana;
	card_ana.analysis(card_stat);
	CardAnalysis::format(card_stat, input);

	return card_ana.type;
}

int CardAnalysis::get_card_type(vector<Card> &input, bool k_is_bomb, bool a_is_bomb)
{
	CardStatistics card_stat;
	card_stat.statistics(input);
	CardAnalysis card_ana;
	card_ana.three_k_is_bomb = k_is_bomb;
	card_ana.three_a_is_bomb = a_is_bomb;
	card_ana.analysis(card_stat);
	CardAnalysis::format(card_stat, input);

	return card_ana.type;
}
