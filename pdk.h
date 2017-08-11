#ifndef _LANDLORD_H_
#define _LANDLORD_H_

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include "card_lib.h"

class Game;

class PDK
{
public:
	std::string         conf_file;
	int                 is_daemonize;
	Json::Value         conf;
	Game                *game;
	struct ev_loop      *loop;
	std::map<std::string, int> room_conf;

private:
};

#endif    /* _LANDLORD_H_ */
