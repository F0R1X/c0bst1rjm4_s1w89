#include <ctime>
#include "card.h"
#include "deck.h"
#include "hole_cards.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_type.h"
#include "card_find.h"

using namespace std;
int main(int argc, char *argv[])
{
	vector<Card> put_cards, cur_cards;
	int a0[8] = {0x03,0x13,0x23,0x04,0x14,0x24,0x05,0x06};
	int a1[15] = {0x15,0x25,0x35,0x16,0x26,0x36,0x33,0x34,0x2a,0x3a,0x0d,0x1d,0x2d,0x0c,0x1c};
	for (int i = 0; i < 8; i++) {
		Card cc(a0[i]);
		put_cards.push_back(cc);
	}
	for (int i = 0; i < 15; i++) {
		Card cc(a1[i]);
		cur_cards.push_back(cc);
	}
	CardFind cf;
	cf.tip(put_cards, cur_cards);
	cf.debug();
	return 0;
}
