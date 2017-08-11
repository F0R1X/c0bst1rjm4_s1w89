#include <algorithm>
#include <set>
#include <ctime>
#include "deck.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_type.h"
#include "card_find.h"


//suit  0  Diamond  1  Club   2 Heart     3 Spade
//remove jokers and three of '2'(keep the spade '2') and spade 'A',48 cards in all.
static int card_arr[] = {
            0x21,    		//A 14
				  0x32,		//2 15
0x03, 0x13, 0x23, 0x33,		//3 3
0x04, 0x14, 0x24, 0x34,		//4 4
0x05, 0x15, 0x25, 0x35,		//5 5
0x06, 0x16, 0x26, 0x36,		//6 6
0x07, 0x17, 0x27, 0x37,		//7 7
0x08, 0x18, 0x28, 0x38,		//8 8
0x09, 0x19, 0x29, 0x39,		//9 9
0x0A, 0x1A, 0x2A, 0x3A,		//10 10
0x0B, 0x1B, 0x2B, 0x3B,		//J 11
0x0C, 0x1C, 0x2C, 0x3C,		//Q 12
0x0D, 0x1D, 0x2D,     		//K 13
};

void Deck::fill()
{
	cards.clear();
	//这种方法效率比较低，可用下一种方法
	//cards.assign(card_arr, card_arr + sizeof(card_arr)/sizeof(card_arr[0]));
	for (int i = 0; i < 45; i++){
		Card c(card_arr[i]);
		push(c);
	}


	//16表示的 这应该是一种玩法
	if (type == 16) {	
		Card a(0x3D);	//黑桃K
		Card b(0x01);	//方片A
		Card c(0x11);	//梅花A
		push(a);
		push(b);
		push(c);
	}
}

void Deck::empty()
{
	cards.clear();
}

int Deck::count() const
{
	return cards.size();
}

bool Deck::push(Card card)
{
	cards.push_back(card);
	return true;
}

bool Deck::pop(Card &card)
{
	if (!count())
		return false;

	card = cards.back();
	cards.pop_back();
	return true;
}

bool Deck::remove_d(Card &card)
{
	for(vector<Card>::iterator it=cards.begin(); it!=cards.end();)
	{
		Card temp = *it;
		if (temp.value==card.value)
		{
			it = cards.erase(it);
		}
		else
		{
			++it;
		}
	}
	return true;
}

bool Deck::shuffle(int seed)
{
	srand(time(NULL) + seed);
	random_shuffle(cards.begin(), cards.end());
	return true;
}

bool Deck::shuffle()
{
	srand(time(NULL));
	random_shuffle(cards.begin(), cards.end());
	return true;
}

int Deck::random_num(int start, int end, int seed)
{
	std::vector<int> myvector;
	for (int i = start; i <= end; ++i) {
		myvector.push_back(i);
	}
	std::srand(unsigned(std::time(NULL)) + seed);
	std::random_shuffle(myvector.begin(), myvector.end());
	return myvector[0];
}

/*srand((unsigned) time(NULL)); //为了提高不重复的概率
rand()%(n - m + 1) + m; */


void Deck::make_boom(HoleCards &holecards){
	//将cards归类
	CardStatistics stat;
	stat.statistics(cards);

	//得到炸弹牌数
	unsigned int len_c = stat.card4.size();
	unsigned int i;

	//牌数大于5，两幅或以上炸弹
	if (len_c>5)
	{
		//随机选出 [1, len_c - 1] 的数字
		i = random_num(1,len_c-1,time(NULL));
	}

	//如果至少有一个炸弹
	if (len_c>0)
	{
		//混牌
		srand(time(NULL) +1);
		random_shuffle(stat.card4.begin(),stat.card4.end());
		Card card;

		//推出混牌后最后的一个牌
		Card random_card;
		random_card = stat.card4.back();
		int count = 0;
		vector<Card>::iterator it = cards.begin();
		for (; it != cards.end();){
			if(random_card == *it){
				card = *it;

				//牌堆中去掉此牌，出牌加入此牌
				it = cards.erase(it);
				holecards.add_card(card);

				//手牌大于13张，退出
				if (holecards.size()>13)
				{
					return;
				}

				count++;
				if(4 == count){
					count = 0;
					return;
				}
			}else{
				++it;
			}
		}
	}
}

void Deck::make_one_line(HoleCards &holecards){

	//将cards归类
	CardStatistics stat;
	stat.statistics(cards);
	vector<Card> cards_line;
	vector<Card> cards_stra;
	cards_line = stat.line1;
	//找到最大的单顺并返回给cards_stra
	CardFind::find_straight(cards_line,cards_stra);
	int len_line = cards_stra.size();
	if (len_line>0)
	{
		Card card;
		Card random_card;
		int count = 0;
		int random_face;
		int first_face;
		int last_face;
		int temp_face;
		//打乱顺子的排序，从中随机返回一张牌的大小
		srand(time(NULL) + 1);
		random_shuffle(cards_stra.begin(),cards_stra.end());
		random_card = cards_stra.back();

		//按从小到大的顺序排序
		Card::sort_by_ascending(cards_stra);
		first_face = cards_stra.front().face;	//顺子的最小牌值
		last_face = cards_stra.back().face;		//顺子的最大牌值
		random_face = random_card.face;			//顺子的随机牌值
		temp_face = random_face - 4;			//取下标
		vector<Card>::iterator it = cards.begin();

		//如果刚好5张（最小长度顺子）
		if(5 == len_line){
			for(int i = 0; i < len_line;i++){
				for(;it != cards.end();){
					if (cards_stra[i] == *it){
						card = *it;
						it = cards.erase(it);
						holecards.add_card(card);
						if (holecards.size()>13)
						{
							return;
						}
						break;
					}else{
						++it;
					}
				}
			}
		}
		//随机出5张牌的顺子
		else if(( last_face - random_face) >= 4){
			for(;it != cards.end();){
				if(random_face == it->face){
					random_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
		//顺子至少9连顺以上，从随机牌（含）往前数5张推入牌堆，一旦满足5张即可渔鸥
		else if((random_face - first_face) >= 4 && (last_face - random_face) < 4){
			for(;it != cards.end();){
				if(temp_face == it->face){
					temp_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
		//顺子较小，取5张牌推入
		else if((random_face - first_face) < 4 && (last_face - random_face) < 4){
			for(;it != cards.end();){
				if(first_face == it->face){
					first_face++;
					card = *it;
					it = cards.erase(it);
					holecards.add_card(card);
					if (holecards.size()>13)
					{
						return;
					}
					count++;
					if(5 == count){
						count = 0;
						return ;
					}
				}else{
					++it;
				}
			}
		}
	}
}

void Deck::make_three_two(HoleCards &holecards){
	Card card;
	Card random_card;
	std::vector<Card> cards_three;
	std::vector<Card> cards_card3;
	//将cards牌归类
	CardStatistics card_stat;
	card_stat.statistics(cards);
	cards_card3 = card_stat.line3;
	//没有3张，退出
	if(cards_card3.size() == 0)
	{
		return ;
	}

	//排乱序
	srand(time(NULL) +1);
	random_shuffle(cards_card3.begin(),cards_card3.end());

	//随机取一张3张的牌
	random_card = cards_card3.back();


	int count = 0;
	vector<Card>::iterator it = cards.begin();
	//从牌堆里取三张
	for (; it != cards.end();){
		if(random_card == *it){
			card = *it;
			it = cards.erase(it);
			holecards.add_card(card);
			if (holecards.size()>13)
			{
				return;
			}
			count++;
			if(3 == count){
				count = 0;
				return ;
			}
		}else{
			++it;
		}
	}
}


void Deck::make_double(HoleCards &holecards){
	Card random_card;
	Card card;

	vector<Card> cards_card2;//用来获取是对子的所有的牌
	CardStatistics card_stat;//用来放置整理好了的牌

	int count = 0;
	srand(time(NULL) + 1);
	card_stat.statistics(cards);
	cards_card2 = card_stat.line2;
	int len = cards_card2.size()/2;
	int i  = random_num(1,len,time(NULL));;
	while (i--)
	{
		card_stat.statistics(cards);
		cards_card2 = card_stat.line2;
		srand(time(NULL) +1);
		random_shuffle(cards_card2.begin(),cards_card2.end());
		random_card = cards_card2.back();

		vector<Card>::iterator it = cards.begin();
		for (; it != cards.end();){
			if(random_card == *it){
				card = *it;
				it = cards.erase(it);
				holecards.add_card(card);
				if (holecards.size()>13)
				{
					return;
				}
				count++;
				if(2 == count){
					count = 0;
					break;
				}
			}else{
				++it;
			}
		}
	}

}

void Deck::make_one(HoleCards &holecards){
	Card random_card;
	Card card;

	vector<Card> cards_card1;
	CardStatistics card_stat;

	srand(time(NULL) + 1);
	card_stat.statistics(cards);
	cards_card1 = card_stat.line1;
	//int len = cards_card1.size();
	srand(time(NULL) +1);
	random_shuffle(cards_card1.begin(),cards_card1.end());
	random_card = cards_card1.back();
	vector<Card>::iterator it = cards.begin();
	for (; it != cards.end();){
		if(random_card == *it){
			card = *it;
			it = cards.erase(it);
			holecards.add_card(card);
			if (holecards.size()>13)
			{
				return;
			}
		}else{
			++it;
		}
	}
}


void Deck::make_card()
{


}

void Deck::get_hole_cards(HoleCards &holecards)
{
	Card card;
	srand(time(NULL) +1);
	random_shuffle(cards.begin(),cards.end());
    int len = holecards.size();
	for (int i = 0; i < (type - len); i++){
		pop(card);
		holecards.add_card(card);
	}
}


void Deck::makecards()
{
	CardStatistics card_stat;
	card_stat.statistics(cards);
	card_stat.debug();
}

void Deck::debug()
{
	cout<<"size "<<cards.size()<<endl;
	Card::dump_cards(cards);
	//Card::dump_cards(goodcards);
}


/*void Deck::make_cards(HoleCards &holecards, int type)
{
	holecards.clear();
	switch (type) {
	case 10:
		make_plane(holecards);
		break;
	case 15:
		make_boom(holecards);
		break;
	case 2:
		make_one_line(holecards);
		break;
	case 4:
		make_two_line(holecards);
		break;
	case 6:
		make_three_line(holecards);
		break;
	case 8:
		make_three_two(holecards);
		break;
	default:
		//get_hole_cards(holecards);
		break;
	}
}*/
