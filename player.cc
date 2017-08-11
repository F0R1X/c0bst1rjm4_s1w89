#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "pdk.h"
#include "log.h"
#include "proto.h"
#include "table.h"
#include "player.h"
#include "client.h"
#include "game.h"

extern PDK pdk;
extern Log log;

Player::Player() :
	_offline_timeout(60),
	_del_player_tstamp(3*60)
{
	_offline_timer.data = this;
	ev_timer_init(&_offline_timer, Player::offline_timeout, _offline_timeout, 0);
	_del_player_timer.data = this;
	ev_timer_init(&_del_player_timer, Player::del_player_timer_cb, _del_player_tstamp, 0);
	vid = 0;
	zid = 0;
	seatid = -1;
	is_offline = 0;
	uid = 0;
	rmb = 0;
	sex = 0;
	money = 0;
	client = NULL;
	login_type = 0;
	logout_type = 0;
	robot = 0;
	ready = 0;
	cur_money =0;
	cur_coin = 0;
	down_tag = 0;
	daemonize_flag = false;
	usertype = 0;
	dissolve_cnt = 0;
}

Player::~Player()
{
	ev_timer_stop(pdk.loop, &_offline_timer);
	ev_timer_stop(pdk.loop, &_del_player_timer);
	if (client)
	  client->player = NULL;
}

void Player::set_client(Client *c)
{
	client = c;
	vid = c->vid;
	zid = c->zid;
	client->player = this;
}

int Player::init( Json::Value &val )
{
	clear();
	settlement.clear();
	uid = val["uid"].asInt();
	skey = val["skey"].asString();
	name = val["name"].asString();
	sex = val["sex"].asInt();
	avatar = val["avatar"].asString();
	rmb = val["rmb"].asInt();
	money = val["money"].asDouble();
	usertype = val["usertype"].asInt();
	if( money < 0 ){
		log.error( "player init error uid[%d] money[%d] less than < 0 .\n", uid, money );
		return -1;
	}
	return 0;
}

void Player::reset()
{
	robot = 0;
	ready = 0;
	cur_coin = 0;
	cur_money = 0;
	ratio = 1;
	dissolve_cnt = 0;
	//dissovle_state = 2;
	com_opt[QIANGGUAN_STATE] = 0;
}

void Player::clear()
{
	login_type = 0;
	robot = 0;
	is_offline = 0;
	daemonize_flag = false;
	logout_type = 0;
	down_tag = 0;
	seatid = -1;
	ready = 0;
	ratio = 1;
	dissolve_cnt = 0;
	dissovle_state = 2;
	com_opt[QIANGGUAN_STATE] = 0;
}

void Player::start_offline_timer()
{
	is_offline = 1;
	ev_timer_start(pdk.loop, &_offline_timer);
}

void Player::stop_offline_timer()
{
	is_offline = 0;
	ev_timer_stop(pdk.loop, &_offline_timer);
}

void Player::offline_timeout(struct ev_loop *loop, ev_timer *w, int revents)
{
	/* player logout
	 * remove from offline table */
	Player* self = (Player*)w->data;
	ev_timer_stop(pdk.loop, &self->_offline_timer);
	if (pdk.game->all_tables.find(self->tid) != pdk.game->all_tables.end()) {
		pdk.game->all_tables[self->tid]->handler_offline(self);
	}
	else {
		pdk.game->del_player(self);
	}
}

void Player::start_del_player_timer()
{
	ev_timer_start(pdk.loop, &_del_player_timer);
}

void Player::stop_del_player_timer()
{
	ev_timer_stop(pdk.loop, &_del_player_timer);
}

void Player::del_player_timer_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	Player* self = (Player*)w->data;
	ev_timer_stop(pdk.loop, &self->_del_player_timer);
	pdk.game->del_player(self);
}


void Player::update_info( long alter_money, int alter_total_win)
{
	long old_money = money;
    money += alter_money;
    if( money < 0 ){
        money = 0;
    }
	//settlement.update_max_score(alter_money);
	settlement.update_win_lose(alter_total_win);
	settlement.update_total_score(alter_money);
	log.info("alter money uid[%d] old_money[%ld] alter_money[%d] money[%ld].\n", uid, old_money, alter_money, money );
}



int Player::recover_data(const PGame::UserInfo &pUserInfo)
{
	tid						= pUserInfo.tid();
	seatid 					= pUserInfo.seatid();
	is_offline 				= pUserInfo.offline();
	uid 					= pUserInfo.uid();
	skey 					= pUserInfo.skey();
	name 					= pUserInfo.name();
	sex	 					= pUserInfo.sex();
	avatar 					= pUserInfo.avatar();
	rmb 					= pUserInfo.rmb();
	money 					= pUserInfo.money();
	usertype 				= pUserInfo.usertype();
	login_type 				= pUserInfo.login_type();
	ready 					= pUserInfo.ready();
	logout_type 			= pUserInfo.logout_type();
	down_tag 				= pUserInfo.down_tag();
	robot 					= pUserInfo.robot();
	ratio 					= pUserInfo.ratio();
	dissolve_cnt 			= pUserInfo.dissolve_cnt();
	dissovle_state 			= pUserInfo.dissovle_state();
	settlement.max_score 	= pUserInfo.max_score();
	settlement.bomb_cnt 	= pUserInfo.bomb_cnt();
	settlement.total_win 	= pUserInfo.total_win();
	settlement.total_lose 	= pUserInfo.total_lose();
	settlement.total_score 	= pUserInfo.total_score();
	log.info("player[%d] recover succ\n", uid);
	return 0;
}
