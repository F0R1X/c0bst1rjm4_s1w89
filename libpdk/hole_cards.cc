#include "hole_cards.h"
#include "card.h"
#include "hole_cards.h"
#include "community_cards.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_find.h"

HoleCards::HoleCards()
{

}

void HoleCards::add_card(Card card)
{
	cards[card.value] = card;
}

int HoleCards::findCard(int value)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		if (value==it->second.value)
		{
			return 1;
		}
	}
	return 0;
}

void HoleCards::copy_cards(std::vector<Card> *v)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		v->push_back(it->second);
	}
}

void HoleCards::copy_cards(std::vector<int> *v)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++)
	{
		Card &card = it->second;
		v->push_back(card.value);
	}
}

void HoleCards::get_one_biggest_card(Card &c, int bomb_is_split)
{
	//将map牌堆转存到vector中
	std::vector<Card> v;
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++){
		v.push_back(it->second);
	}

	//根据炸弹是否可拆，找到
	CardFind cf;		
	cf.bomb_split = bomb_is_split;	

	//将牌归类分堆
	CardStatistics cs;
	cs.statistics(v);

	//从大到小遍历单牌，根据炸弹是否可拆排除，选出最大的单牌放入第一个参数c中
	std::vector<Card>::reverse_iterator it_v = cs.line1.rbegin();
	while (it_v != cs.line1.rend()) {
		if (!cf.bomb_split && cf.check_is_bomb_card((*it_v).face, cs)) {
			it_v--;
			continue;
		}
		c = *it_v;
		break;
	}
}


int HoleCards::robot(std::vector<int> &v)
{
	//map转存到cards_int中
	std::vector<int> cards_int;
	copy_cards(&cards_int);

	//找到cards_int中最长的顺子，放入v中，如果找到，即返回
	// to find straight
	CardFind::find_straight(cards_int, v); // find the longest
	if (v.size() > 0)
	{
		//remove(v);
		return 0;
	}

	//如果找不到顺子    
	// can not find straight
	vector<Card> cards_obj;
	copy_cards(&cards_obj);
	CardStatistics card_stat;
	card_stat.statistics(cards_obj);


	//找不到顺子，就出3带2，或者3带1,3带0
	// to find three with two;if no more cards,can be three with one or zero
	if (card_stat.card3.size() > 0)
	{
		v.push_back(card_stat.card3[0].value);
		v.push_back(card_stat.card3[1].value);
		v.push_back(card_stat.card3[2].value);

		//3带2 推对子
		if (card_stat.card2.size() > 0)
		{
			v.push_back(card_stat.card2[0].value);
			v.push_back(card_stat.card2[1].value);
			//remove(v);
			return 0;
		}

		//3带2张单牌
		if (card_stat.card1.size() > 1 )
		{
			if (card_stat.card1[0].face != 16 && card_stat.card1[0].face != 17)
			{
				v.push_back(card_stat.card1[0].value);
				v.push_back(card_stat.card1[1].value);
				//remove(v);
				return 0;
			}
		}
		//3带2 拆副炸弹
		else if (card_stat.card4.size()>0)
		{
			v.push_back(card_stat.card4[0].value);
			v.push_back(card_stat.card4[1].value);
			return 0;
		}
		//3带1 带个单牌
		else  if(card_stat.card1.size() > 0)
		{
			v.push_back(card_stat.card1[0].value);
			return 0;
		}

		//remove(v);
		return 0;
	}

	//顺子和3张都没有，就出个最小的单牌
	//the smallest single card
	if (card_stat.card1.size() > 0)
	{
		v.push_back(card_stat.card1[0].value);
		//remove(v);
		return 0;
	}

	//上述都没有，直接扔炸弹
	//the boom
	if (card_stat.card4.size() > 0)
	{
		v.push_back(card_stat.card4[0].value);
		v.push_back(card_stat.card4[1].value);
		v.push_back(card_stat.card4[2].value);
		v.push_back(card_stat.card4[3].value);
		//remove(v);
		return 0;
	}

	return -1;
}

int HoleCards::robot(std::vector<Card> &v)//the first to output cards
{
	std::vector<Card> cards_int;
	copy_cards(&cards_int);

	// to find straight
	// CardFind::find_straight(cards_int, v);
	// if (v.size() > 0)
	// {
	// 	//remove(v);
	// 	//cout<<"find the straight"<<endl;
	// 	return 0;
	// }

	//can not find the straight
	vector<Card> cards_obj;
	copy_cards(&cards_obj);
	CardStatistics card_stat;
	card_stat.statistics(cards_obj);

	// to find three or with one or with two
	if (card_stat.card3.size() > 0)
	{
		//3带2单
		if (card_stat.card1.size() > 1)
		{
			//if (card_stat.card1[0].face != 16 && card_stat.card1[0].face != 17)
			//{
				v.push_back(card_stat.card3[0]);
				v.push_back(card_stat.card3[1]);
				v.push_back(card_stat.card3[2]);
				v.push_back(card_stat.card1[0]);
				v.push_back(card_stat.card1[1]);

				return 0;
			//}
		}
		//3带1对
		if (card_stat.card2.size() > 0)
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card2[0]);
			v.push_back(card_stat.card2[1]);

			return 0;
		}
		//3带1对，拆3张
		if (card_stat.card3.size() > 3)
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card3[3]);
			v.push_back(card_stat.card3[4]);

			return 0;
		}
		//4带3
		if (card_stat.card4.size() > 0)//four with three
		{
			v.push_back(card_stat.card3[0]);
			v.push_back(card_stat.card3[1]);
			v.push_back(card_stat.card3[2]);
			v.push_back(card_stat.card4[0]);
			v.push_back(card_stat.card4[1]);
			v.push_back(card_stat.card4[2]);
			v.push_back(card_stat.card4[3]);

			return 0;
		}
		// if (card_stat.card1.size() > 0)
		// {
		// 	v.push_back(card_stat.card3[0]);
		// 	v.push_back(card_stat.card3[1]);
		// 	v.push_back(card_stat.card3[2]);
		// 	v.push_back(card_stat.card1[0]);
		//
		// 	return 0;
		// }

	}

	//3张没有出1对
	//can not find three with one or two
	if (card_stat.card2.size() > 0)
	{
		v.push_back(card_stat.card2[0]);
		v.push_back(card_stat.card2[1]);
		//remove(v);

		return 0;
	}

	//1对没有出单
	if (card_stat.card1.size() > 0)
	{
		v.push_back(card_stat.card1.back());
		//remove(v);
		return 0;
	}

	//啥都没有扔炸弹
	if (card_stat.card4.size() > 0)
	{
		v.push_back(card_stat.card4[0]);
		v.push_back(card_stat.card4[1]);
		v.push_back(card_stat.card4[2]);
		v.push_back(card_stat.card4[3]);
		//remove(v);

		return 0;
	}

	//出顺子的最大单
	if (card_stat.line1.size() > 0)
	{
		v.push_back(card_stat.line1.back());
		//remove(v);

		return 0;
	}
	return -1;
}


void HoleCards::remove(std::vector<Card> &v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		cards.erase(v[i].value);
	}
}

void HoleCards::remove(std::vector<int> &v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		cards.erase(v[i]);
	}
}

int HoleCards::size()
{
	return (int)(cards.size());
}

void HoleCards::debug()
{
	Card::dump_cards(cards);
}
