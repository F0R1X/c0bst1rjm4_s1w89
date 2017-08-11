#ifndef _CARD_H_
#define _CARD_H_

#include <string>
#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

/*
	该类定义了纸牌的基本格式
	可优化的空间：
	1、纸牌可以用 _uint8 表示，通过掩码 &0x0f，&0xf0 或 >>4 来获取牌的花色和面值，取消face和suit，更节约空间

	by f0rix 2017年8月8日
*/
/**
 * suit  0 Diamonds 方片  1 Clubs 梅花  2 Hearts 红桃  3 Spades 黑桃
 * 	
0x01, 0x11, 0x21, 0x31,		//A 14
0x02, 0x12, 0x22, 0x32,		//2 15
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
0x0D, 0x1D, 0x2D, 0x3D,		//K 13
0x0E,						//red joker
0x0F						//black joker
 * @author luochuanting
 */
typedef enum {
	Three = 3,		//3		♠3 ♥3 ♦3 ♣3
	Four,			//4		♠4 ♥4 ♦4 ♣4
	Five,			//5		♠5 ♥5 ♦5 ♣5
	Six,			//6		♠6 ♥6 ♦6 ♣6
	Seven,			//7		♠7 ♥7 ♦7 ♣7
	Eight,			//8		♠8 ♥8 ♦8 ♣8
	Nine,			//9		♠9 ♥9 ♦9 ♣9
	Ten,			//10	♠10 ♥10 ♦10 ♣10
	Jack,			//11	♠J ♥J ♦J ♣J
	Queen,			//12	♠Q ♥Q ♦Q ♣Q
	King,			//13	♠K ♥K ♦K ♣K
	Ace,			//14	♠A ♥A ♦A ♣A
	Two,			//15	♠2 ♥2 ♦2 ♣2

	FirstFace = Three,
	LastFace = Two
} Face;

typedef enum {
	Diamonds = 0, //方片
	Clubs,		  //梅花
	Hearts,		  //红心
	Spades,		  //黑桃

	FirstSuit = Diamonds,
	LastSuit = Spades
} Suit;

class Card
{
	/*remove the pair of jokers;					//去掉大小王
	 *remove three '2'(remain the Spades '2');		//移除3个2， 保留黑桃2
	 *remove the Spades 'A';						//移除黑桃A
	 *48 cards in all								//合计48张牌
	 */
  public:
	int face; //牌值
	int suit; //花色

	int value;

	Card();
	Card(int val);

	/*****************************************
		功能：返回牌的值和花色
				格式：3d   方片3
					 4c   梅花4
	******************************************/
	std::string get_card();

	bool operator<(const Card &c) const { return (face < c.face); };
	bool operator>(const Card &c) const { return (face > c.face); };
	bool operator==(const Card &c) const { return (face == c.face); };

	/*****************************************
		功能：比较两个牌型的大小，
				大于返回1，小于返回-1，相等返回0，
				同一牌值下，黑桃>红桃>梅花>方片
	******************************************/
	static int compare(const Card &a, const Card &b)
	{
		if (a.face > b.face)
		{
			return 1;
		}
		else if (a.face < b.face)
		{
			return -1;
		}
		else if (a.face == b.face)
		{
			if (a.suit > b.suit)
			{
				return 1;
			}
			else if (a.suit < b.suit)
			{
				return -1;
			}
		}

		return 0;
	}

	/***********************************************
		功能：判断 a < b 
	************************************************/
	static bool lesser_callback(const Card &a, const Card &b)
	{
		if (Card::compare(a, b) == -1)
			return true;
		else
			return false;
	}

	/***********************************************
		功能：判断 a > b
	************************************************/
	static bool greater_callback(const Card &a, const Card &b)
	{
		if (Card::compare(a, b) == 1)
			return true;
		else
			return false;
	}

	/***********************************************
		功能：排序
				对输入的向量中的牌按从小到大的顺序进行排序
	************************************************/
	static void sort_by_ascending(std::vector<Card> &v)
	{
		sort(v.begin(), v.end(), Card::lesser_callback);
	}


	/***********************************************
		功能：排序
				对输入的向量中的牌按从大到小的顺序进行排序
	************************************************/
	static void sort_by_descending(std::vector<Card> &v)
	{
		sort(v.begin(), v.end(), Card::greater_callback);
	}


	/***********************************************
		功能：输出牌堆中(vector表示)所有的牌型到缓冲区
				格式　［str]: [[3d 4c 5s 6h ...]]

	************************************************/
	static void dump_cards(std::vector<Card> &v, string str = "cards")
	{
		fprintf(stdout, "[%s]: [[ ", str.c_str());
		for (std::vector<Card>::iterator it = v.begin(); it != v.end(); it++)
			fprintf(stdout, "%s ", it->get_card().c_str());

		fprintf(stdout, "]]\n");
	}

	/***********************************************
		功能：输出牌堆中（map表示）所有的牌型到缓冲区
				格式　［str]: [[3d 4c 5s 6h ...]]
				
	************************************************/
	static void dump_cards(std::map<int, Card> &m, string str = "cards")
	{
		fprintf(stdout, "[%s]: [[ ", str.c_str());
		for (std::map<int, Card>::iterator it = m.begin(); it != m.end(); it++)
			fprintf(stdout, "%s ", it->second.get_card().c_str());

		fprintf(stdout, "]]\n");
	}
};

#endif /* _CARD_H_ */
