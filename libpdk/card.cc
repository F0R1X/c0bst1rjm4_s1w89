#include "card.h"

static char face_symbols[] = {
	'3', '4', '5', '6', '7', '8', '9',
	'T', 'J', 'Q', 'K', 'A', '2'
};

static char suit_symbols[] = {
	'd', 'c', 'h', 's'
};

Card::Card()
{
	face = suit = value = 0;
}

Card::Card(int val)
{
	value = val;

	face = value & 0xF; //  get the lower four bits -- face; amount to *ch - '0',higher efficiency
	suit = value >> 4; // get the higher four bits -- suit
	if (face < 3)
		face += 13; //14='A' 15='2'
	//printf("val[%d] Face[%d] Suit[%d]\n",val, face, suit);
}

string Card::get_card()
{
	string card;

	/*
	char buf[32];
	snprintf(buf, sizeof(buf), "%d-%d", face, suit);
	card.append(buf);
	*/

	card.append(1, face_symbols[face - 3]);
	card.append(1, suit_symbols[suit]);

	//cout<<"card:"<<card<<endl;
	return card;
}
