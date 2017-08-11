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
#include <algorithm>
#include <assert.h>
#include <sys/time.h>
#include <sstream>

#include "pdk.h"
#include "game.h"
#include "log.h"
#include "table.h"
#include "client.h"
#include "player.h"
#include "proto.h"
#include "card.h"
#include "card_find.h"
#include "card_statistics.h"
#include "card_analysis.h"
#include "card_type.h"
#include "card.h"
#include "base64.h"


extern PDK pdk;
extern Log log;

extern void xorfunc(std::string &nString);

using namespace std;

Table::Table():
preready_timer_stamp(1.5),
play_card_timer_tstamp(20),
robot_timer_tstamp(2),
pass_timer_tstamp(0.5),
pass_timer_more_tstamp(1),
call_timer_stamp(1.5),
dissolve_room_tstamp(180),
pre_pass_tstamp(0.5),
video_tstamp(0.5),
game_restart_timer_stamp(3)
{
	preready_timer.data = this;
	ev_timer_init(&preready_timer, Table::preready_timer_cb, preready_timer_stamp, preready_timer_stamp);

	play_card_timer.data = this;
	ev_timer_init(&play_card_timer, Table::play_card_timer_cb, play_card_timer_tstamp, play_card_timer_tstamp);

	robot_timer.data = this;
	ev_timer_init(&robot_timer, Table::robot_timer_cb, robot_timer_tstamp, robot_timer_tstamp);

	pass_timer.data = this;
	ev_timer_init(&pass_timer, Table::pass_timer_cb, pass_timer_tstamp, pass_timer_tstamp);

	call_timer.data = this;
	ev_timer_init(&call_timer, Table::call_timer_cb, call_timer_stamp, call_timer_stamp);

	dissolve_room_timer.data = this;
	ev_timer_init(&dissolve_room_timer, Table::dissolve_room_timer_cb, dissolve_room_tstamp, dissolve_room_tstamp);

	pre_pass_timer.data = this;
	ev_timer_init(&pre_pass_timer, Table::pre_pass_timer_cb, pre_pass_tstamp, pre_pass_tstamp);

	video_timer.data = this;
	ev_timer_init(&video_timer, Table::video_timer_cb, video_tstamp, video_tstamp);

	game_restart_timer.data = this;
	ev_timer_init(&game_restart_timer, Table::game_restart_timer_cb, game_restart_timer_stamp, game_restart_timer_stamp);

	init_member_variable();
}

Table::~Table()
{
	ev_timer_stop(pdk.loop, &preready_timer);
	ev_timer_stop(pdk.loop, &play_card_timer);
	ev_timer_stop(pdk.loop, &robot_timer);
	ev_timer_stop(pdk.loop, &pass_timer);
	ev_timer_stop(pdk.loop, &call_timer);
	ev_timer_stop(pdk.loop, &dissolve_room_timer);
	ev_timer_stop(pdk.loop, &pre_pass_timer);
	ev_timer_stop(pdk.loop, &video_timer);
	ev_timer_stop(pdk.loop, &game_restart_timer);
}

void Table::init_member_variable()
{
	MAX_PLAYERS = 3;
	tid = "";

	//json库table节点下key：vid value->asInt;
	vid = pdk.conf["tables"]["vid"].asInt();
	zid = pdk.conf["tables"]["zid"].asInt();
	lose_exp = pdk.conf["tables"]["lose_exp"].asInt();
	win_exp = pdk.conf["tables"]["win_exp"].asInt();
	players.clear();			//房间内所有玩家
	seat_players.clear();		//玩牌时所有在座的玩家
	voters_players.clear();		//解散房间投票者
	last_players.clear();		//上局玩家的uid 用于判断是否玩家不变(先出规则)
	dissovle_state = 0;			//是否处于解散房间 0--no 1--yes
	is_dissolved = 0;			//此房间已被解散
	game_nums = 1;
	qiangguan_ratio = 1;			//抢关倍数
	mingpai_start_ratio = 1;		//明牌发牌倍数
	mingpai_play_ratio = 1;			//明牌打牌倍数
	base_money_double_ratio = 1;	//底分加倍倍数
	win_seatid = -1;				//赢家座位号
	is_split = 0;					//0--没有切牌选项 1--有切牌选项
	k_is_bomb = false;
	a_is_bomb = false;
	reset();
}

int Table::init(Json::Value &val, Client *client)
{
	tid = client->roomid.empty() ? tid : client->roomid;
	base_money = val["baseGold"].asInt();
	stand_money = val["standMoney"].asInt();
	takein = val["roomGolden"].asInt();
	owner_uid = val["roomUserId"].asInt();
	room_state = val["roomStatus"].asInt();
	if (room_state >= 2) {
		return -1;
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1001"].asString(), table_type) < 0){
		table_type = pdk.conf["tables"]["table_type"].asInt();
	}
	if(table_type == 16){
		multi_conf(val["dataMap"]["innerWayList"]["1020"].asString(), player_option);
		a_is_bomb = player_option.find(THREE_A_IS_BOMB) != player_option.end();
	}
	else if(table_type == 15){
		multi_conf(val["dataMap"]["innerWayList"]["1019"].asString(), player_option);
		k_is_bomb = player_option.find(THREE_K_IS_BOMB) != player_option.end();
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1002"].asString(), first_rule) < 0){
		first_rule = pdk.conf["tables"]["first_rule"].asInt();
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1021"].asString(), settle_type) < 0){
		settle_type = pdk.conf["tables"]["settle_type"].asInt();
	}
	if(settle_type == 1){
		if(single_conf(val["dataMap"]["innerWayList"]["1003"].asString(), last_card_rule) < 0){
			last_card_rule = pdk.conf["tables"]["last_card_rule"].asInt();
		}
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1004"].asString(), single_rule) < 0){
		single_rule = pdk.conf["tables"]["single_rule"].asInt();
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1015"].asString(), spade3_first_out) < 0){
		spade3_first_out = pdk.conf["tables"]["spade3_first_out"].asInt();
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1016"].asString(), MAX_PLAYERS) < 0){
		MAX_PLAYERS = pdk.conf["tables"]["max_players"].asInt();
	}
	if(single_conf(val["dataMap"]["innerWayList"]["1017"].asString(), is_display_left) < 0){
		is_display_left = pdk.conf["tables"]["is_display_left"].asInt();
	}
	multi_conf(val["dataMap"]["innerWayList"]["1005"].asString(), player_option);
	multi_conf(val["dataMap"]["innerWayList"]["1018"].asString(), player_option);
	
	if (player_option.find(BOMB_CANNOT_SPLIT) != player_option.end()) {
		bomb_can_split = 0;
	}
	else {
		bomb_can_split = 1;
	}
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		seats[i].clear();
		seats[i].seatid = i;
	}
	heart10_owner_map.clear();
	return 0;
}

int Table::single_conf(string conf_id, int &conf_value, bool user_input)
{
	if (!conf_id.empty()) {
		if (user_input) {
			conf_value = atoi(conf_id.c_str());
		}
		else if (pdk.room_conf.find(conf_id) != pdk.room_conf.end()) {
			conf_value = pdk.room_conf[conf_id];
		}
		else{
			return -1;
		}
		return 0;
	}
	log.error("conf_id[%s] is empty\n", conf_id.c_str());
	return -1;
}

int Table::multi_conf(string conf_id, set<int> &conf_value)
{
	if (!conf_id.empty()) {
		char *temp;
		char tempstr[200];
		strcpy(tempstr, conf_id.c_str());
		temp = strtok(tempstr, ",");
		while (temp != NULL) {
			conf_id = temp;
			if (pdk.room_conf.find(conf_id) != pdk.room_conf.end()) {
				conf_value.insert(pdk.room_conf[conf_id]);
			}
			// else{
			// 	return -1;
			// }
			temp = strtok(NULL, ",");
		}
	}
	return 0;
}

void Table::reset()
{
	state = READY;		//enum Type
	can_logout = 1;
	start_seat = -1;	//每一轮首出牌玩家
	cur_seat = -1;		//当前出牌玩家
	//win_seatid = -1;
	play_cards.clear();	//当前出的牌
	hole_cards.clear();	//用于复制玩家手牌
	last_cards.clear();	//游戏中上一家出的牌
	ready_players = 0;
	//voters_players.clear();
	//dissovle_state = 0;
	let_slip_seat = -1;	//放走包赔玩家
	ratio = 1;			//比例
	split_ensure = 0;	//0--未选择是否切牌 1--不切牌 2--切牌 3--切牌中 4--确定切牌
	split_pos = 0;		//切牌位置
	split_seat = -1;	//切牌座位
	spade3_owner = -1;	//黑桃三拥有者
	m_videostr.clear();	//录像数据
	banker = -1;		//本局庄家
	out_card_index = 1;	//第多少手
	m_bAlreadyResp = false;
	heart10_owner = -1;	//红桃10玩家
}


int Table::broadcast(Player *p, const std::string &packet)
{
	Player *player;
	std::map<int, Player*>::iterator it;
	for (it = players.begin(); it != players.end(); it++){
		player = it->second;
		if (player == p || player->client == NULL){
			continue;
		}
		player->client->send(packet);
	}
	return 0;
}

int Table::unicast(Player *p, const std::string &packet)
{
	if (p->client){
		return p->client->send(packet);
	}
	return -1;
}

int Table::random(int start, int end, int seed)
{
	srand((unsigned)time(NULL) + seed);
	return start + rand() % (end - start + 1);
}

void Table::preready_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->preready_timer);
	table->handler_preready();
}

void Table::call_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->call_timer);
	table->start_play();
}

void Table::pass_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->pass_timer);
	table->cur_seat = table->next_seat(table->cur_seat);
	if (table->cur_seat == table->start_seat) {
		table->action = NEW_CARD;
		if (table->card_type == CARD_TYPE_BOMB) {
			table->bomb_balance(table->seats[table->cur_seat].player);
		}
	}
	else{
		table->action = FOLLOW_CARD;
	}
	table->start_next_player();
}

void Table::pre_pass_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->pre_pass_timer);
	table->handler_pass_card();
}

void Table::robot_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->robot_timer);
	Player *player = table->seats[table->cur_seat].player;
	if (table->state == PLAYING) {
		table->handler_robot_play(player);
		log.debug("robot_timer_cb\n");
	}
}


void Table::play_card_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->play_card_timer);
	Player *player = table->seats[table->cur_seat].player;
	table->handler_timeout(player);
}

int Table::handler_login(Player *player)
{
	if (players.find(player->uid) == players.end()) {
		players[player->uid] = player;
		player->tid = tid;
		//自动上桌
		handler_apply_uptable(player);
		handler_login_succ_uc(player);
		handler_table_info(player);
		return 0;
	}
	return -1;
}

int Table::handler_login_succ_uc(Player *player)
{
	PGame::UptableSuccAck pUptableSuccAck;
	PGame::UserInfo *pUserInfo = pUptableSuccAck.mutable_player();
	pUserInfo->set_uid(player->uid);
	pUserInfo->set_seatid(player->seatid);
	pUserInfo->set_name(player->name);
	pUserInfo->set_sex(player->sex);
	pUserInfo->set_avatar(player->avatar);
	pUserInfo->set_rmb(player->rmb);
	pUserInfo->set_money(player->money);
	Ppacket ppack;
	pUptableSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_LOGIN_SUCC_UC);
	unicast(player, ppack.data);
	log.info( "one player login uid[%d].\n", player->uid );
	player->logout_type = 0;
	return 0;
}


int Table::handler_login_succ_bc(Player *player)
{
	PGame::UptableSuccAck pUptableSuccAck;
	PGame::UserInfo *pUserInfo = pUptableSuccAck.mutable_player();
	pUserInfo->set_uid(player->uid);
	pUserInfo->set_seatid(player->seatid);
	pUserInfo->set_name(player->name);
	pUserInfo->set_sex(player->sex);
	pUserInfo->set_avatar(player->avatar);
	pUserInfo->set_rmb(player->rmb);
	pUserInfo->set_money(player->money);
	Ppacket ppack;
	pUptableSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_LOGIN_SUCC_BC);
	broadcast(player, ppack.data);
	player->logout_type = 0;
	return 0;
}


int Table::handler_table_info(Player *player)
{
	PGame::TableInfo pTableInfo;
	if(seat_players.find(player->uid) != seat_players.end()){
		pTableInfo.set_seatid(player->seatid);
	}else {
		pTableInfo.set_seatid(-1);
	}
	pTableInfo.set_state(state);
	pTableInfo.set_preready_timer_stamp(preready_timer_stamp);
	pTableInfo.set_play_card_timer_stamp(play_card_timer_tstamp);
	pTableInfo.set_pass_timer_tstamp(pass_timer_tstamp);
	pTableInfo.set_call_timer_stamp(call_timer_stamp);
	pTableInfo.set_dissolve_room_tstamp(dissolve_room_tstamp);
	pTableInfo.set_spade3_first_out(spade3_first_out);
	pTableInfo.set_table_type(table_type);
	pTableInfo.set_single_rule(single_rule);
	pTableInfo.set_bomb_can_split(bomb_can_split);
	pTableInfo.set_base_money(base_money);
	pTableInfo.set_stand_money(stand_money);
	pTableInfo.set_takein(takein);
	pTableInfo.set_ratio(ratio);
	pTableInfo.set_banker(banker);
	pTableInfo.set_max_players(MAX_PLAYERS);
	pTableInfo.set_left_nums(is_display_left);
	std::map<int, Player*>::iterator it;
	for (it = seat_players.begin(); it != seat_players.end(); it++) {
		Player *p;
		p = it->second;
		if (p == NULL)
		  continue;
		PGame::UserInfo *pUserInfo = pTableInfo.add_players();
		pUserInfo->set_uid(p->uid);
		pUserInfo->set_ready(p->ready);
		pUserInfo->set_seatid(p->seatid);
		pUserInfo->set_name(p->name);
		pUserInfo->set_sex(p->sex);
		pUserInfo->set_avatar(p->avatar);
		pUserInfo->set_rmb(p->rmb);
		pUserInfo->set_money(p->money);
		pUserInfo->set_card_nums(seats[p->seatid].hole_cards.size());
		pUserInfo->set_robot(p->robot);
		pUserInfo->set_usertype(p->usertype);
		pUserInfo->set_offline(p->is_offline);
	}
	if(seat_players.find(player->uid) != seat_players.end()){
		map<int, Card>::iterator it;
		map<int, Card> &cards = seats[player->seatid].hole_cards.cards;
		for (it = cards.begin(); it != cards.end(); it++){
			pTableInfo.add_holes(it->second.value);
		}
	}
	if (state == QIANGGUAN_STATE) {
		pTableInfo.set_cur_seat(cur_seat);
	}
	if (state == PLAYING) {
		pTableInfo.set_cur_seat(cur_seat);
		pTableInfo.set_action(action);
		if (spade3_first_out == 1) {
			pTableInfo.set_spade3_owner(spade3_owner);
		}
		for (unsigned int i = 0; i < play_cards.size(); i++) {
			pTableInfo.add_play_cards(play_cards[i].value);
		}
		if (action == FOLLOW_CARD || action == PASS_CARD) {
			for (unsigned int i = 0; i < last_cards.size(); i++) {
				pTableInfo.add_last_cards(last_cards[i].value);
			}
			pTableInfo.set_last_seatid(last_seatid);
		}
	}
	else if (state == END_GAME) {
		log.debug("handler_rebind END_GAME: %d\n", state);
	}
	for (std::set<int>::iterator it_s = player_option.begin(); it_s != player_option.end(); it_s++) {
		pTableInfo.add_player_option(*it_s);
	}
	PGame::DisRoomInfo *pDisRoomInfo = pTableInfo.mutable_dissolve_room_info();
	pDisRoomInfo->set_state(dissovle_state);
	if (dissovle_state == 1) {
		pDisRoomInfo->set_uid(dissolve_applyer);
		pDisRoomInfo->set_remain_time((int)ev_timer_remaining(pdk.loop, &dissolve_room_timer));
		for (std::map<int, Player*>::iterator it = voters_players.begin(); it != voters_players.end(); it++) {
			pDisRoomInfo->add_voters_uid(it->first);
			PGame::UserDisRoomInfo *pUserDisRoomInfo = pDisRoomInfo->add_players();
			pUserDisRoomInfo->set_uid(it->second->uid);
			pUserDisRoomInfo->set_action(it->second->dissovle_state);
		}
	}
	Ppacket ppack;
	pTableInfo.SerializeToString(&ppack.body);
	ppack.pack(SERVER_TABLE_INFO_UC);
	unicast(player, ppack.data);
	return 0;
}

int Table::handler_apply_uptable(Player *player)
{
	//has on the table
	if (seat_players.find(player->uid) != seat_players.end()) {
		log.error("handler apply uptable error uid[%d] already in seat.\n", player->uid);
		uptable_error_uc(player, PGame::USER_ALREADY_IN_SEAT_ERR);
		return -1;
	}
	//not enough money
	if (player->money < stand_money) {
		log.error("handler apply uptable error uid[%d] money[%d] too less stand_money[%d].\n", player->uid, player->money, stand_money);
		uptable_error_uc(player, PGame::USER_NO_MONEY_UPTABLE_ERR);
		return -1;
	}
	if ( seat_players.size() >= (unsigned int)MAX_PLAYERS) {
		log.error("handler apply uptable error uid[%d] more than MAX_PLAYERS[%d].\n", player->uid, MAX_PLAYERS);
		uptable_error_uc(player, PGame::NO_UPTABLE_SEAT_ERR);
		return -1;
	}
	handler_uptable(player);
	return 0;
}

int Table::handler_ready(Player *player)
{
	if (state != READY) {
		log.error("handler_ready state[%d]\n", state);
		return -1;
	}
	PGame::ReadyReq pReadyReq;
	if (!pReadyReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_ready parse pReadyReq error\n");
		return -1;
	}
	//兼容老版本不带ready字段
	if (!pReadyReq.has_ready()) {
		if (player->ready == 1) {
			return -1;
		}
		else {
			player->ready = 1;
		}
	}
	else {
		int is_ready = pReadyReq.ready();
		if (is_ready != 0 && is_ready != 1) {
			log.error("handler ready err uid[%d] ready[%d] tid[%d]\n", player->uid, is_ready, tid.c_str());
			return -1;
		}
		if (is_ready == 1 && player->ready == 1) {
			log.error("player uid[%d] has already[%d] tid[%d]\n", player->uid, is_ready, tid.c_str());
			return -1;
		}
		player->ready = is_ready;
	}
	PGame::ReadyAck pReadyAck;
	pReadyAck.set_uid(player->uid);
	pReadyAck.set_seatid(player->seatid);
	pReadyAck.set_ready(player->ready);
	Ppacket ppack;
	pReadyAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_READY_SUCC_BC);
	broadcast(NULL, ppack.data);
	log.info("handler_game_ready success uid[%d] \n", player->uid );
	ready_players = 0;
	for (map<int, Player*>::iterator it = seat_players.begin(); it != seat_players.end(); it++) {
		if (it->second->ready == 1) {
			ready_players++;
		}
	}
	log.info("check ready players[%d] tid[%s] game_nums[%d]\n", ready_players, tid.c_str(), game_nums);
	if(ready_players == MAX_PLAYERS){
		if (state < QIANGGUAN_STATE && player_option.find(QIANG_GUAN) != player_option.end()) {
			state = QIANGGUAN_STATE;
		}
		else if (state < BASE_DOUBLE_STATE && player_option.find(BASE_MONEY_DOUBLE) != player_option.end()) {
			state = BASE_DOUBLE_STATE;
		}
		else if (state < MINGPAI_PLAY_STATE && player_option.find(MINGPAI_PLAY) != player_option.end()) {
			state = MINGPAI_PLAY_STATE;
		}
		else{
			state = PLAYING;
		}
		log.debug("players all ready state[%d] tid[%s]\n", state, tid.c_str());
		ev_timer_stop(pdk.loop, &preready_timer);
		game_start_req();
		ev_timer_again(pdk.loop, &game_restart_timer);
	}
	return 0;
}

int Table::handler_uptable(Player *player)
{
	if (seat_players.find(player->uid) != seat_players.end()) {
		log.error("handler up table error uid[%d] already in seat.\n", player->uid);
		uptable_error_uc(player, PGame::USER_ALREADY_IN_SEAT_ERR);
		return -1;
	}
	if (player->money < stand_money) {
		log.error("handler up table error uid[%d] money[%d] too less stand_money[%d].\n", player->uid, player->money, stand_money);
		uptable_error_uc(player, PGame::USER_NO_MONEY_UPTABLE_ERR);
		return -1;
	}
	player->seatid = sit_down(player);
	Seat &seat = seats[player->seatid];
	seat.uid = player->uid;
	seat_players[player->uid] = player;
	PGame::UptableSuccAck pUptableSuccAck;
	PGame::UserInfo *pUserInfo = pUptableSuccAck.mutable_player();
	pUserInfo->set_uid(player->uid);
	pUserInfo->set_seatid(player->seatid);
	pUserInfo->set_name(player->name);
	pUserInfo->set_sex(player->sex);
	pUserInfo->set_avatar(player->avatar);
	pUserInfo->set_rmb(player->rmb);
	pUserInfo->set_money(player->money);
	Ppacket ppack;
	pUptableSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_UPTABLE_SUCC_BC);
	broadcast(NULL, ppack.data);
	return 0;
}

void Table::uptable_error_uc(Player *player, int code)
{
	PGame::UptableErrAck pUptableErrAck;
	pUptableErrAck.set_uid(player->uid);
	pUptableErrAck.set_code(code);
	pUptableErrAck.set_money(player->money);
	Ppacket ppack;
	pUptableErrAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_UPTABLE_ERR_UC);
	unicast(player, ppack.data);
}

int Table::sit_down(Player *player)
{
	std::vector<int> tmp;
	for (int i = 0; i < MAX_PLAYERS; i++){
		if (seats[i].occupied == 0){
			tmp.push_back(i);
		}
	}
	int len = tmp.size();
	if (len > 0){
		int index = random(0, len - 1, player->uid);
		int i = tmp[index];
		log.debug("len[%d] index[%d] i[%d]\n", len, index, i);
		if (player->login_type==0){
			seats[i].reset();
		}
		seats[i].occupied = 1;
		seats[i].player = player;
		return i;
	}
	log.info("sit_down uid[%d] seatid[%d] tid[%s].\n",player->uid, player->seatid,tid.c_str());
	return -1;
}


int Table::game_start_req()
{
	std::ostringstream oss;
	oss << tid << "_" << game_nums;
	Jpacket req_packet;
	req_packet.val["cmd"] = SERVER_DZ_GAME_START_REQ;
	req_packet.val["vid"] = vid;
	req_packet.val["roomid"] = tid;
	req_packet.end();
	pdk.game->send_to_datasvr( req_packet.tostring() );
	log.info( "game start req roomid[%s] .\n", tid.c_str() );
	room_state = 1;
	return 0;
}


int Table::handler_downtable(Player *player)
{
	// player in seat
	if (seat_players.find(player->uid) == seat_players.end()) {
		log.error("handler down table error uid[%d] not in seat tid[%s].\n", player->uid, tid.c_str());
		return -1;
	}
	if (!can_logout) {
		player->down_tag = (player->down_tag == 0) ? 1:player->down_tag;
		PGame::AheadDowntableAck pAheadDowntableAck;
		pAheadDowntableAck.set_uid(player->uid);
		pAheadDowntableAck.set_seatid(player->seatid);
		pAheadDowntableAck.set_down_tag(player->down_tag);
		Ppacket ppack;
		pAheadDowntableAck.SerializeToString(&ppack.body);
		ppack.pack(SERVER_AHEAD_DOWNTABLE_UC);
		unicast(player, ppack.data);
		log.info("handler down table next round down uid[%d] money[%d] state[%d] tid[%s].\n", player->uid, player->money, state, tid.c_str());
		return 0;
	}
	PGame::DowntableSuccAck pDowntableSuccAck;
	pDowntableSuccAck.set_uid(player->uid);
	pDowntableSuccAck.set_seatid(player->seatid);
	pDowntableSuccAck.set_money(player->money);
	pDowntableSuccAck.set_type(player->logout_type);
	Ppacket ppack;
	pDowntableSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_DOWNTABLE_SUCC_BC);
	broadcast(NULL, ppack.data);
	seat_players.erase(player->uid);
	stand_up(player);
	player->clear();
	return 0;
}

void Table::stand_up(Player *player)
{
	seats[player->seatid].clear();
}

int Table::game_start_res( Jpacket &packet )
{
	int can_start = packet.val["can_start"].asInt();
	if( 0 == can_start )
	{
		int code = packet.val["code"].asInt();
		PGame::GameStartAck pGameStartAck;
		pGameStartAck.set_can_start(can_start);
		pGameStartAck.set_code(code);
		pGameStartAck.set_ts((int)time( NULL ));
		pGameStartAck.set_status(packet.val["status"].asInt());
		Ppacket ppack;
		pGameStartAck.SerializeToString(&ppack.body);
		ppack.pack(SERVER_DZ_GAME_START_RES);
		broadcast( NULL, ppack.data );
		log.error( "game start res fail roomid[%s] can_start[%d] code[%d] .\n", tid.c_str(), can_start, code );
		advance_game_end();
		return -1;
	}
	ev_timer_stop(pdk.loop, &game_restart_timer);
	if( m_bAlreadyResp == true )
	{
		log.error("game start res tid[%s] m_bAlreadyResp[%d] .\n", tid.c_str(), m_bAlreadyResp );
		return -1;
	}
	m_bAlreadyResp = true;
	test_game_start();
	log.info( "game start res succ roomid[%s] can_start[%d] game_nums[%d].\n", tid.c_str(), can_start, game_nums);
	return 0;
}

int Table::advance_game_end()
{
	log.info("roomid[%s] game_num[%d] advance game_end\n", tid.c_str(), game_nums);
	state = END_GAME;
	ev_timer_stop(pdk.loop, &preready_timer);
	ev_timer_stop(pdk.loop, &play_card_timer);
	ev_timer_stop(pdk.loop, &robot_timer);
	ev_timer_stop(pdk.loop, &call_timer);
	ev_timer_stop(pdk.loop, &pass_timer);
	ev_timer_stop(pdk.loop, &dissolve_room_timer);
	ev_timer_stop(pdk.loop, &pre_pass_timer);
	for (std::map<int, Player*>::iterator it = seat_players.begin(); it != seat_players.end(); it++) {
		it->second->down_tag = 1;
	}
	//handler_preready();
	is_dissolved = 1;
	return 0;
}


int Table::check_cards_error(HoleCards &h, vector<Card> &c)
{
	if (c.empty()) {
		log.error("check cards empty error\n");
		return -1;
	}
	set<int> c_val;
	c_val.clear();
	vector<Card>::iterator it = c.begin();
	while (it != c.end()) {
		if (h.cards.find(it->value) == h.cards.end()) {
			log.error("check cards error HoleCards don't contain card[%d]\n", it->value);
			return -1;
		}
		if (c_val.find(it->value) == c_val.end()) {
			c_val.insert(it->value);
			it++;
		}
		else{
			log.error("check cards error recieve 2 same card[%d]\n", it->value);
			it = c.erase(it);
		}
	}
	return 0;
}
int Table::test_game_start()
{
	if (seat_players.size() != (unsigned int)MAX_PLAYERS) {
		log.error("test start bet seat_players size[%d] MAX_PLAYERS[%d].\n", seat_players.size(), MAX_PLAYERS);
		std::map<int, int> uid_win_lose;
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (seats[i].player) {
				uid_win_lose[seats[i].player->uid] = 0;
			}
		}
		update_user_info_to_datasvr( uid_win_lose, 1102 );
		return -1;
	}
	can_logout = 0;
	game_start();
	return 0;
}

void Table::safe_check_ratio()
{
	if(ratio > max_ratio){
		ratio = max_ratio;
	}else if(ratio <= 1){
		ratio = 1;
	}
}

void Table::handler_offline(Player *player)
{
	PGame::OfflineAck pOfflineAck;
	pOfflineAck.set_uid(player->uid);
	pOfflineAck.set_seatid(player->seatid);
	Ppacket ppack;
	pOfflineAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_PLAYER_OFFLINE_BC);
	broadcast(NULL, ppack.data);
}

void Table::handler_timeout(Player *player)
{
	PGame::TimeoutAck pTimeoutAck;
	pTimeoutAck.set_uid(player->uid);
	pTimeoutAck.set_seatid(player->seatid);
	pTimeoutAck.set_timeout_type(player->is_offline);
	Ppacket ppack;
	pTimeoutAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_PLAYER_TIMEOUT_BC);
	broadcast(NULL, ppack.data);
}

void Table::common_operation_err(Player *player, int error_code)
{
	PGame::UserComOptErrAck pUserComOptErrAck;
	pUserComOptErrAck.set_error_code(error_code);
	Ppacket ppack;
	pUserComOptErrAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_COMMON_OPT_ERR);
	unicast(player, ppack.data);
}

int Table::game_start()
{
	std::ostringstream oss;
	oss<<zid<<tid<<time(NULL);
	innings = oss.str();
	deck.type = table_type;
	log.info("game start .\n");
	last_cards.clear();
	play_cards.clear();
	for (int i = 0; i < MAX_PLAYERS; i++) {
		seats[i].hole_cards.clear();
		seats[i].player->reset();
		seats[i].player->ready = 1;
	}
	srand(time(NULL));
	int rnd = rand() % 10;
	int is_fit = 0;
	do {
		deck.fill();
		deck.shuffle(rand() % 100);
		for (int i = 0; i < MAX_PLAYERS; i++) {
			seats[i].cardno = table_type;
			seats[i].hole_cards.clear();
			deck.get_hole_cards(seats[i].hole_cards);
			if (seats[i].hole_cards.findCard(0x33)) {
				start_seat = i;
			}
			if(seats[i].hole_cards.findCard(0x2a) && player_option.find(HEART_TEN) != player_option.end()){
				heart10_owner = i;
			}
		}
		vector<Card> cards_filter;
		CardStatistics filter_stat;
		int boom_count = 0;
		for (int i = 0; i < MAX_PLAYERS; i++) {
			seats[i].hole_cards.copy_cards(&cards_filter);
			filter_stat.statistics(cards_filter);
			boom_count += filter_stat.card4.size() / 4;
			cards_filter.clear();
			filter_stat.clear();
		}
		if ((boom_count == 0 && rnd < 8) || (boom_count > 0 && boom_count < 3 && rnd >= 8)) {
			break;
		}
		else {
			is_fit++;
			continue;
		}
	} while(is_fit <= 50);
	// int a0[16] = {0x03,0x13,0x23,0x33,0x04,0x14,0x24,0x34,0x0d,0x1d,0x2d,0x3d,0x01,0x11,0x21,0x02};
	// int a1[16] = {0x06,0x16,0x26,0x36,0x07,0x17,0x27,0x37,0x08,0x18,0x28,0x38,0x09,0x19,0x29,0x39};
	// int a2[16] = {0x0a,0x1a,0x2a,0x3a,0x0b,0x1b,0x2b,0x3b,0x0c,0x1c,0x2c,0x3c,0x05,0x15,0x25,0x35};
	// seats[0].hole_cards.clear();
	// seats[1].hole_cards.clear();
	// seats[2].hole_cards.clear();
	// for (int i = 0; i < 16; i++) {
	// 	Card cc(a0[i]);
	// 	seats[0].hole_cards.add_card(cc);
	// 	start_seat = 0;
	// }
	// for (int i = 0; i < 16; i++) {
	// 	Card cc(a1[i]);
	// 	seats[1].hole_cards.add_card(cc);
	// }
	// for (int i = 0; i < 16; i++) {
	// 	Card cc(a2[i]);
	// 	seats[2].hole_cards.add_card(cc);
	// }
	if (MAX_PLAYERS == 2) {
		start_seat = rand() % 2;
	}
	heart10_owner_map[game_nums] = heart10_owner;
	bool same_players = true;
	for (std::map<int, Player*>::iterator it = seat_players.begin(); it != seat_players.end(); it++) {
		if (last_players.find(it->second->uid) == last_players.end()) {
			same_players = false;
			break;
		}
	}
	if (same_players) {
		if (first_rule == 0) {
			start_seat = win_seatid == -1 ? start_seat : win_seatid;
		}
	}
	Ppacket ppack;
	ppack.pack(SERVER_GAME_START_BC);
	broadcast(NULL, ppack.data);
	cur_seat = start_seat;
	banker = start_seat;
	action = NEW_CARD;
	save_game_start_video_data();
	send_card();
	ev_timer_again(pdk.loop, &call_timer);
	return 0;
}

void Table::send_card()
{
	for (int c = 0; c < MAX_PLAYERS; c++){
		Seat &seat = seats[c];
		Player *player = seat.player;
		if (player->com_opt.find(MINGPAI_START_STATE) != player->com_opt.end()
			&& player->com_opt[MINGPAI_START_STATE] > 1) {
			PGame::MingPaiSuccAck pMingPaiSuccAck;
			pMingPaiSuccAck.set_uid(player->uid);
			pMingPaiSuccAck.set_seatid(player->seatid);
			map<int, Card>::iterator it = seat.hole_cards.cards.begin();
			map<int, Card>::iterator it_end = seat.hole_cards.cards.end();
			for (; it != it_end; it++) {
				pMingPaiSuccAck.add_cards(it->first);
			}
			Ppacket ppack;
			pMingPaiSuccAck.SerializeToString(&ppack.body);
			ppack.pack(SERVER_MINGPAI_SUCC_BC);
			broadcast(NULL, ppack.data);
			save_video_data(ppack.header.cmd, ppack.body);
		}
	}
	for (int c = 0; c < MAX_PLAYERS; c++){
		Seat &seat = seats[c];
		Player *player = seat.player;
		PGame::GameStartUc pGameStartUc;
		pGameStartUc.set_uid(player->uid);
		pGameStartUc.set_seatid(player->seatid);
		map<int, Card>::iterator it = seat.hole_cards.cards.begin();
		map<int, Card>::iterator it_end = seat.hole_cards.cards.end();
		for (; it != it_end; it++) {
			pGameStartUc.add_cards(it->first);
		}
		Ppacket ppack;
		pGameStartUc.SerializeToString(&ppack.body);
		ppack.pack(SERVER_GAME_START_UC);
		log.info("tid[%s] player[%d] send card[%s]\n", tid.c_str(), player->uid, pGameStartUc.DebugString().c_str());
		unicast(player, ppack.data);
	}
	PGame::FirstPlayAck pFirstPlayAck;
	pFirstPlayAck.set_seatid(start_seat);
	if (first_rule == 1) {
		pFirstPlayAck.set_type(0);
		spade3_owner = start_seat;
	}
	else {
		if (win_seatid == -1) {
			pFirstPlayAck.set_type(0);
			spade3_owner = start_seat;
		}
		else{
			pFirstPlayAck.set_type(1);
		}
	}
	Ppacket ppack;
	pFirstPlayAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_FIRST_PLAY_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
}

void Table::start_play()
{
	state = PLAYING;
	cur_seat = start_seat;
	action = NEW_CARD;
	start_next_player();
}

int Table::check_card_type(int _card_type, bool _last_hand)
{
	if (_card_type == CARD_TYPE_FOURWITHTWO && player_option.find(ALLOW_FOUR_WITH_TWO) == player_option.end() ) {
		return -1;
	}
	if (_card_type == CARD_TYPE_FOURWITHTHREE && player_option.find(ALLOW_FOUR_WITH_THREE) == player_option.end()) {
		return -1;
	}
	if (!_last_hand) {
		if (_card_type == CARD_TYPE_THREE || _card_type == CARD_TYPE_THREEWITHONE || _card_type == CARD_TYPE_PLANEWITHONE || _card_type == CARD_TYPE_PLANEWHITHLACK) {
			return -1;
		}
		if (_card_type == CARD_TYPE_THREELINE && player_option.find(ALLOW_THREE_LINE) == player_option.end()) {
			return -1;
		}
	}
	else{
		if (_card_type == CARD_TYPE_THREE || _card_type == CARD_TYPE_THREEWITHONE){
			if(action == NEW_CARD && player_option.find(ALLOW_THREE_WITH_ONE_LAST) == player_option.end()){
				return -1;
			}
			if(action == FOLLOW_CARD && player_option.find(ALLOW_THREE_WITH_ONE_LAST_JIE) == player_option.end()){
				return -1;
			}
		}
		if (_card_type == CARD_TYPE_PLANEWHITHLACK || _card_type == CARD_TYPE_PLANEWITHONE){
			if(action == NEW_CARD && player_option.find(ALLOW_PLANE_WITH_LESS_LAST) == player_option.end()){
				return -1;
			}
			if(action == FOLLOW_CARD && player_option.find(ALLOW_PLANE_WITH_LESS_LAST_JIE) == player_option.end()){
				return -1;
			}
		}
	}
	return 0;
}

int Table::start_next_player()
{
	Player *player = seats[cur_seat].player;
	//优先判断要不起
	if (action == FOLLOW_CARD) {
		CardFind card_find;
		card_find.bomb_split = bomb_can_split;
		hole_cards.clear();
		seats[cur_seat].hole_cards.copy_cards(&hole_cards);
		card_find.tip(last_cards, hole_cards, k_is_bomb, a_is_bomb);
		if (card_find.results.size() == 0) {
			action = PASS_CARD;
		}
		else if (card_find.results.size() == 1 && card_find.results[0].size() ==  hole_cards.size()) {
			
			int _card_type = CardAnalysis::get_card_type(hole_cards, k_is_bomb, a_is_bomb);
			if (check_card_type(_card_type, true) >= 0) {
				play_cards = hole_cards;
				action = LAST_CARD;
			}
			else {
				action = PASS_CARD;
			}
		}
	}
	hole_cards.clear();
	seats[cur_seat].hole_cards.copy_cards(&hole_cards);
	CardStatistics cs;
	cs.statistics(hole_cards);
	CardAnalysis ca;
	ca.three_k_is_bomb = k_is_bomb;
	ca.three_a_is_bomb = a_is_bomb;
	ca.analysis(cs);
	//判断手牌是否可以全出
	if (action == NEW_CARD) {
		card_type = ca.type;
		if (card_type > 0 && cs.card4.size() == 0) {
			if ((card_type == CARD_TYPE_PLANEWHITHLACK || card_type == CARD_TYPE_PLANEWITHONE) && player_option.find(ALLOW_PLANE_WITH_LESS_LAST) == player_option.end()) {
				if (cs.len % 5 == 0 && (cs.card4.size() == 0 || bomb_can_split)) {
					card_type = CARD_TYPE_PLANEWITHWING;
				}
			}
			if (check_card_type(card_type, true) >= 0) {
				play_cards = hole_cards;
				action = LAST_CARD;
			}
		}
	}
	PGame::PlayCardAck pPlayCardAck;
	pPlayCardAck.set_uid(player->uid);
	pPlayCardAck.set_seatid(cur_seat);
	pPlayCardAck.set_action(action);
	Ppacket ppack;
	pPlayCardAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_PLAY_CARD_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	ev_timer_again(pdk.loop, &play_card_timer);
	if (action == PASS_CARD || action == LAST_CARD) {
		return 0;
	}
	if (player->robot == 1) {
		log.debug("server robot [%d]\n", player->uid);
		ev_timer_again(pdk.loop, &robot_timer);
	}
	Seat &seat_n = seats[(cur_seat + 1) % MAX_PLAYERS];
	if (seat_n.is_single) {
		Jpacket pac;
		PGame::SingleCardAck pSingleCardAck;
		pSingleCardAck.set_type(single_rule);
		Ppacket ppack1;
		pSingleCardAck.SerializeToString(&ppack1.body);
		ppack1.pack(SERVER_SINGLE_CARD_UC);
		unicast(player, ppack1.data);
	}
	return 0;
}

int Table::next_seat(int pos)
{
	for (int i = 0; i < MAX_PLAYERS; i++) {
		pos++;
		if (pos >= MAX_PLAYERS)
		  pos = 0;
		return pos;
	}

	return -1;
}

void Table::card_type_count(int type)
{
	Seat &seat = seats[cur_seat];
	switch (type) {
		case CARD_TYPE_BOMB:
			seat.player->settlement.update_bomb_cnt();
			break;
	}
}

int Table::handler_play_card(Player *player)
{
	if (state != PLAYING) {
		log.error("handler play card state[%d] error tid[%s].\n", state, tid.c_str());
		return -1;
	}
	log.debug("handler play card req uid[%d] real uid[%d] seatid[%d] .\n", player->uid, seats[cur_seat].player->uid, cur_seat);
	Player *p = seats[cur_seat].player;
	if (p != player) {
		log.error("handler play card player error p_uid[%d] but active uid[%d] tid[%s].\n", p->uid, player->uid, tid.c_str());
		return -1;
	}
	PGame::PlayCardReq pPlayCardReq;
	if (!pPlayCardReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_play_card parse pPlayCardReq error\n");
		return -1;
	}
	int player_action = pPlayCardReq.action();
	if (player_action != action) {
		log.error("handler play card player error real action[%d] but return action[%d] tid[%s].\n", action, player_action, tid.c_str());
		return -1;
	}
	ev_timer_stop(pdk.loop, &play_card_timer);
	if (player_action == PASS_CARD) {
		ev_timer_stop(pdk.loop, &pass_timer);
		ev_timer_again(pdk.loop, &pre_pass_timer);
		return 0;
	}
	if (player_action == LAST_CARD) {
		handler_put_card();
		return 0;
	}
	vector<Card> cur_cards;
	for (int i = 0; i < pPlayCardReq.cards_size(); i++){
		Card card(pPlayCardReq.cards(i));
		cur_cards.push_back(card);
	}
	if (check_cards_error(seats[cur_seat].hole_cards, cur_cards) < 0) {
		log.error("check player[%d] cards error\n", player->uid);
		common_operation_err(player, PGame::PLAY_CARD_TYPE_ERR);
		return -1;
	}
	int my_card_type = 0;
	int ret = 0;
	if (action == NEW_CARD){
		ret = CardAnalysis::get_card_type(cur_cards, k_is_bomb, a_is_bomb);
		my_card_type = ret;
	}
	else {
		ret = CardAnalysis::isGreater(last_cards, cur_cards, &my_card_type, seats[cur_seat].cardno, k_is_bomb, a_is_bomb);
	}
	card_type = my_card_type;
	//检测报单出牌
	Seat &seat_n = seats[(cur_seat + 1) % MAX_PLAYERS];
	if (seat_n.is_single && my_card_type == CARD_TYPE_ONE) {
		Card tm;
		seats[cur_seat].hole_cards.get_one_biggest_card(tm, bomb_can_split);
		if (cur_cards[0].face != tm.face) {
			if (single_rule == 1) {
				common_operation_err(player, PGame::PLAY_SINGLE_CARD_ERR);
				log.debug("error single card\n");
				return -1;
			}
			else{
				seats[cur_seat].is_max_single = 0;
			}
		}
		else {
			seats[cur_seat].is_max_single = 1;
		}
	}
	//检测炸弹不可拆
	if (my_card_type < CARD_TYPE_FOURWITHONE && !bomb_can_split) {
		std::set<int> bomb_card_val;
		CardStatistics cs;
		hole_cards.clear();
		seats[cur_seat].hole_cards.copy_cards(&hole_cards);
		cs.statistics(hole_cards);
		if (cs.card4.size() > 0) {
			for (unsigned int i = 0; i < cs.card4.size(); i += 4) {
				bomb_card_val.insert(cs.card4[i].face);
			}
			std::vector<Card>::iterator it_cur = cur_cards.begin();
			for (; it_cur != cur_cards.end(); it_cur++) {
				if (bomb_card_val.find(it_cur->face) != bomb_card_val.end()) {
					common_operation_err(player, PGame::PLAY_CARD_TYPE_ERR);
					log.debug("bomb can not split\n");
					return -1;
				}
			}
		}
	}
	//检测牌型错误
	if (check_card_type(my_card_type, cur_cards.size() == (unsigned int)seats[cur_seat].cardno) < 0) {
		common_operation_err(player, PGame::PLAY_CARD_TYPE_ERR);
		log.debug("check card type error\n");
		return -1;
	}
	//增加首出必出黑桃三判断
	if (spade3_owner != -1 && spade3_first_out == 1 && MAX_PLAYERS == 3) {
		bool no_spade3 = true;
		if (spade3_owner == player->seatid) {
			for (vector<Card>::iterator it = cur_cards.begin(); it != cur_cards.end(); it++) {
				if (it->value == 0x33) {
					no_spade3 = false;
				}
			}
			if (no_spade3) {
				common_operation_err(player, PGame::PLAY_SPADE3_ERR);
				return -1;
			}
			else {
				spade3_owner = -1;
			}
		}
	}
	if (ret > 0 && my_card_type == card_type) {
		play_cards = cur_cards;
		handler_put_card();
	}
	return 0;
}

void Table::handler_put_card(int _type)
{
	start_seat = cur_seat;
	last_seatid = cur_seat;
	last_cards = play_cards;
	Seat &seat = seats[cur_seat];
	CardAnalysis type_analysis;
	card_type  = type_analysis.get_card_type(play_cards, k_is_bomb, a_is_bomb);
	if (_type != 0) {
		card_type = _type;
	}
	Player *player = seat.player;
	seat.hole_cards.remove(play_cards);
	seat.last_play_cards = play_cards;
	seat.cardno = seat.hole_cards.size();
	PGame::PlayCardSuccAck pPlayCardSuccAck;
	pPlayCardSuccAck.set_uid(player->uid);
	pPlayCardSuccAck.set_seatid(cur_seat);
	pPlayCardSuccAck.set_action(action);
	pPlayCardSuccAck.set_card_type(card_type);
	pPlayCardSuccAck.set_card_nums(seat.cardno);
	pPlayCardSuccAck.set_base_money(base_money);
	pPlayCardSuccAck.set_ratio(ratio);
	for (unsigned int i = 0; i < play_cards.size(); i++) {
		pPlayCardSuccAck.add_cards(play_cards[i].value);
	}
	Ppacket ppack;
	pPlayCardSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_PLAY_CARD_SUCC_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	card_type_count(card_type);
	//统计手牌详情
	if (player->card_list.find(game_nums) == player->card_list.end()) {
		vector<CardList> cl;
		player->card_list[game_nums] = cl;
	}
	CardList card_l;
	card_l.index = out_card_index;
	for (unsigned int i = 0; i < play_cards.size(); i++) {
		card_l.cards.push_back(play_cards[i].value);
	}
	player->card_list[game_nums].push_back(card_l);
	out_card_index++;
	//抢关失败
	if (player_option.find(QIANG_GUAN) != player_option.end()) {
		if (seats[(cur_seat + 2) % 3].player->com_opt[QIANGGUAN_STATE] == 2) {
			qiangguan_fail();
			return;
		}
	}
	//游戏结算
	if (seat.cardno == 0) {
		Seat &seat_l = seats[(cur_seat + 2) % 3];
		if (card_type == CARD_TYPE_ONE && seat_l.last_play_cards.size() == 1) {
			if (!seat_l.is_max_single) {
				let_slip_seat = seat_l.seatid;
			}
		}
		if (card_type == CARD_TYPE_BOMB) {
			bomb_balance(player);
		}
		game_end();
		return;
	}
	else if (seat.cardno == 1) {
		seat.is_single = 1;
	}
	cur_seat = next_seat(cur_seat);
	if (cur_seat == start_seat) {
		action = NEW_CARD;
	}
	else{
		action = FOLLOW_CARD;
	}
	start_next_player();
}


void Table::qiangguan_fail()
{
	log.debug("qiangguan fail game end...\n");
	ev_timer_stop(pdk.loop, &play_card_timer);
	state = END_GAME;
	win_seatid = -1;
	last_players.clear();
	int fail_seat = -1;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (seats[i].player->com_opt[QIANGGUAN_STATE] == 2) {
			fail_seat = i;
		}
	}
	int total_win = 0;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (i != fail_seat) {
			seats[i].cur_money = table_type * spring_ratio * base_money * ratio;
			seats[i].player->update_info(seats[i].cur_money, 1);
			total_win += seats[i].cur_money;
			//seats[i].player->update_info(seats[i].cur_money, 1, 0, lose_exp);
			log.debug("seat[%d] cardno[%d] spring[%d] cur_money[%d]...\n",i, seats[i].cardno, seats[i].spring, seats[i].cur_money);
		}
	}
	seats[fail_seat].cur_money = -total_win;
	seats[fail_seat].player->update_info(seats[fail_seat].cur_money, 0);
	std::map<int, int> uid_win_lose;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		uid_win_lose[seats[i].player->uid] = seats[i].cur_money + seats[i].bomb_money;
	}
	update_user_info_to_datasvr( uid_win_lose, 1102 );
	PGame::GameEndAck pGameEndAck;
	pGameEndAck.set_seatid(fail_seat);
	pGameEndAck.set_fail(1);
	pGameEndAck.set_base_money(base_money);
	pGameEndAck.set_ratio(ratio);
	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = seats[i].player;
		PGame::CountMoney *pCountMoney = pGameEndAck.add_players();
		pCountMoney->set_seatid(seats[i].seatid);
		pCountMoney->set_uid(player->uid);
		pCountMoney->set_rmb(player->rmb);
		pCountMoney->set_money(player->money);
		pCountMoney->set_cur_money(seats[i].cur_money);
		pCountMoney->set_spring(seats[i].spring);
		map<int, Card>::iterator it = seats[i].hole_cards.cards.begin();
		map<int, Card>::iterator it_end = seats[i].hole_cards.cards.end();
		for (; it != it_end; it++) {
			pCountMoney->add_holes(it->first);
		}
		last_players.insert(player->uid);
	}
	Ppacket ppack;
	pGameEndAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_GAME_END_BC);
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	ev_timer_again(pdk.loop, &video_timer);
	ev_timer_again(pdk.loop, &preready_timer);
}

void Table::handler_pass_card()
{
	Player *player = seats[cur_seat].player;
	seats[cur_seat].last_play_cards.clear();
	PGame::PassCardAck pPassCardAck;
	pPassCardAck.set_uid(player->uid);
	pPassCardAck.set_seatid(cur_seat);
	Ppacket ppack;
	pPassCardAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_PASS_CARD_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	ev_timer_stop(pdk.loop, &pass_timer);
	unsigned int last_cards_nums = last_cards.size();
	if (last_cards_nums < 5) {
		ev_timer_set(&pass_timer, pass_timer_tstamp, pass_timer_tstamp);
	}
	else {
		ev_timer_set(&pass_timer, pass_timer_more_tstamp, pass_timer_more_tstamp);
	}
	ev_timer_again(pdk.loop, &pass_timer);
}


int Table::handler_robot(Player *player)
{
	//屏蔽托管功能
#if 1
	if (state != PLAYING) {
		log.error("handler robot error state[%d]\n", state);
		return -1;
	}
	PGame::RobotReq pRobotReq;
	if (!pRobotReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_robot parse pRobotReq error\n");
		return -1;
	}
	player->robot = pRobotReq.robot();
	PGame::RobotAck pRobotAck;
	pRobotAck.set_uid(player->uid);
	pRobotAck.set_seatid(player->seatid);
	pRobotAck.set_robot(pRobotReq.robot());
	Ppacket ppack;
	pRobotAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_ROBOT_BC);
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	if (cur_seat == player->seatid){
		ev_timer_stop(pdk.loop, &robot_timer);
		if (action == NEW_CARD || action == FOLLOW_CARD){
			if (player->robot == 1)	{
				log.debug("player[%d] robot running right row.\n", player->uid);
				handler_robot_play(player);
			}
			else{
				PGame::PlayCardAck pPlayCardAck;
				pPlayCardAck.set_uid(player->uid);
				pPlayCardAck.set_seatid(cur_seat);
				pPlayCardAck.set_action(action);
				Ppacket ppack;
				pPlayCardAck.SerializeToString(&ppack.body);
				ppack.pack(SERVER_PLAY_CARD_BC, atoi(tid.c_str()));
				unicast(player, ppack.data);
				save_video_data(ppack.header.cmd, ppack.body);
			}
		}
	}
	// if (player->robot != 0) {
	// 	player->robot = 1;
	// }
	// else{
	// 	if(cur_seat == player->seatid){
	// 		ev_timer_stop(pdk.loop, &robot_timer);
	// 		if (action == NEW_CARD || action == FOLLOW_CARD){
	// 			PGame::PlayCardAck pPlayCardAck;
	// 			pPlayCardAck.set_uid(player->uid);
	// 			pPlayCardAck.set_seatid(cur_seat);
	// 			pPlayCardAck.set_action(action);
	// 			Ppacket ppack;
	// 			pPlayCardAck.SerializeToString(&ppack.body);
	// 			ppack.pack(SERVER_PLAY_CARD_BC, atoi(tid.c_str()));
	// 			unicast(player, ppack.data);
	// 			save_video_data(ppack.header.cmd, ppack.body);
	// 		}
	// 	}
	// }
	// if (player->robot == 1 && cur_seat == player->seatid
	// 	&& action == ) {
	// 	log.debug("player[%d] robot running right row.\n", player->uid);
	// 	handler_robot_play(player);
	// }
#endif
	return 0;
}

//机器人只处理newcard和followcard的逻辑 pass和last系统处理
void Table::handler_robot_play(Player *player)
{
	ev_timer_stop(pdk.loop, &play_card_timer);
	Seat &seat = seats[cur_seat];
	CardFind card_find;
	card_find.bomb_split = bomb_can_split;
	if (action == NEW_CARD) {
		start_seat = cur_seat;
		last_cards.clear();
		play_cards.clear();
		if (spade3_first_out == 1 && spade3_owner == player->seatid && MAX_PLAYERS == 3) {
			play_cards.clear();
			if (seat.hole_cards.findCard(0x03) == 1
				&& seat.hole_cards.findCard(0x13) == 1
				&& seat.hole_cards.findCard(0x23) == 1
				&& seat.hole_cards.findCard(0x33) == 1) {
				play_cards.push_back(0x03);
				play_cards.push_back(0x13);
				play_cards.push_back(0x23);
				play_cards.push_back(0x33);
			}
			else {
				play_cards.push_back(0x33);
			}
			handler_put_card();
			spade3_owner = -1;
			return;
		}
		seat.hole_cards.robot(play_cards);
		CardStatistics playcard_stat;
		playcard_stat.statistics(play_cards);
		CardAnalysis playcard_ana;
		playcard_ana.three_k_is_bomb = k_is_bomb;
		playcard_ana.three_a_is_bomb = a_is_bomb;
		playcard_ana.analysis(playcard_stat);
		int play_type = playcard_ana.type;
		Seat &seat_n = seats[(cur_seat + 1) % MAX_PLAYERS];
		//next seat is single
		if (play_type == CARD_TYPE_ONE && seat_n.is_single) {
			Card tmp_card;
			seat.hole_cards.get_one_biggest_card(tmp_card, bomb_can_split);
			play_cards.clear();
			play_cards.push_back(tmp_card);
		}
		handler_put_card();
		return;
	}
	else if (action == FOLLOW_CARD){
		play_cards.clear();
		hole_cards.clear();
		seat.hole_cards.copy_cards(&hole_cards);
		Seat &seat_n = seats[(cur_seat + 1) % MAX_PLAYERS];
		CardStatistics tmp_stat;
		tmp_stat.statistics(hole_cards);
		if (last_cards.size() == 1 && seat_n.is_single) {
			if (tmp_stat.card4.size() > 0) {
				for (int i = 0; i < 4; i++) {
					play_cards.push_back(tmp_stat.card4[i]);
				}
				handler_put_card();
				return;
			}
			else{
				Card tmp_card;
				seat.hole_cards.get_one_biggest_card(tmp_card, bomb_can_split);
				if (tmp_card.face > last_cards[0].face) {
					play_cards.push_back(tmp_card);
					handler_put_card();
					return;
				}
			}
		}
		else{
			card_find.tip(last_cards,hole_cards, k_is_bomb, a_is_bomb);
			if(card_find.results.size() > 0){
				vector< vector<Card> >::iterator it = card_find.results.begin();
				play_cards = *it;
				handler_put_card();
				return;
			}
		}
	}
	// else if (action == PASS_CARD){
	// 	ev_timer_again(pdk.loop, &pre_pass_timer);
	// 	return;
	// }
	// else if (action == LAST_CARD){
	// 	handler_put_card();
	// 	return;
	// }
}

void Table::bomb_balance(Player *player)
{
	PGame::BombAck pBombAck;
	pBombAck.set_uid(player->uid);
	pBombAck.set_seatid(cur_seat);
	for (int i = 0; i < MAX_PLAYERS; i++) {
		int bomb_m = 0;
		if (seats[i].player == player) {
			if(settle_type == 1){
				bomb_m =  10 * (MAX_PLAYERS - 1);
			}
			else if(settle_type == 2){
				bomb_m = 2 * (MAX_PLAYERS - 1);
			}
			seats[i].player->update_info(bomb_m, 2);
			 
		}else{
			if(settle_type == 1){
				bomb_m = -10;
			}
			else if(settle_type == 2){
				bomb_m = -2;
			}
			seats[i].player->update_info(bomb_m, 2);
		}
		seats[i].bomb_money += bomb_m;
		PGame::CountMoney *pCountMoney = pBombAck.add_players();
		pCountMoney->set_uid(seats[i].player->uid);
		pCountMoney->set_seatid(seats[i].seatid);
		pCountMoney->set_money(seats[i].player->money);
		pCountMoney->set_cur_money(bomb_m);
	}
	Ppacket ppack;
	pBombAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_BOMB_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
}

void Table::count_money(Player *player)
{
	int total_win = 0;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (seats[i].player != player){
			seats[i].cur_money = -1;
			if(table_type == seats[i].cardno) {
				seats[i].spring = 1;
				seats[i].cur_money = -2;
			}
		}
		if(seats[i].player->seatid == heart10_owner || player->seatid == heart10_owner){
			seats[i].cur_money *= 2;
		}
	}
	//按剩余牌多少结算
	if(settle_type == 2){
		int lose_seatid1 = (player->seatid + 1) % MAX_PLAYERS;
		int lose_seatid2 = (player->seatid + 2) % MAX_PLAYERS;
	 	int left_num1 = seats[lose_seatid1].cardno;
		int left_num2 = seats[lose_seatid2].cardno;
		if(MAX_PLAYERS == 2){
			seats[lose_seatid1].cur_money *= 2;
			total_win = -seats[lose_seatid1].cur_money;
		}
		else if(MAX_PLAYERS == 3){
			if(left_num1 == left_num2){
				seats[lose_seatid1].cur_money *= 2;
				seats[lose_seatid2].cur_money *= 2;
			}
			else if(left_num1 > left_num2){
				seats[lose_seatid1].cur_money *= 2;
				seats[lose_seatid2].cur_money *= 1;
			}
			else if(left_num2 > left_num1){
				seats[lose_seatid2].cur_money *= 2;
				seats[lose_seatid1].cur_money *= 1;
			}
			total_win = -(seats[lose_seatid1].cur_money + seats[lose_seatid2].cur_money);
		}
	}
	else if(settle_type == 1){
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (seats[i].player != player) {
				if(seats[i].cardno == 1 && last_card_rule == 0){
					seats[i].cur_money = 0;
				}
				else{
					seats[i].cur_money *= seats[i].cardno;
				}
				total_win += -seats[i].cur_money;
				log.debug("seat[%d] cardno[%d] spring[%d] cur_money[%d]...\n",i, seats[i].cardno, seats[i].spring, seats[i].cur_money);
			}
		}
	}
	
	//包赔
	if (single_rule == 0 && let_slip_seat >= 0 && let_slip_seat < MAX_PLAYERS) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (i == let_slip_seat) {
				seats[i].cur_money = -total_win;
			}
			else if (i != win_seatid) {
				seats[i].cur_money = 0;
			}
		}
	}
	seats[win_seatid].cur_money = total_win;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (i == win_seatid) {
			seats[i].player->update_info(seats[i].cur_money, 1);
		}
		else {
			seats[i].player->update_info(seats[i].cur_money, 0);
		}
	}
	std::map<int, int> uid_win_lose;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		uid_win_lose[seats[i].player->uid] = seats[i].cur_money + seats[i].bomb_money;
		seats[i].player->settlement.update_max_score(seats[i].cur_money + seats[i].bomb_money);
		SettleList settle_l;
		settle_l.cur_money = seats[i].cur_money + seats[i].bomb_money;
		settle_l.bomb_money = seats[i].bomb_money;
		seats[i].player->settle_list[game_nums] = settle_l;
	}
	update_user_info_to_datasvr( uid_win_lose, 1102 );
}

void Table::update_user_info_to_datasvr( std::map<int, int> &uid_win_lose, int tally_type,  int alter_type )
{
	game_nums++;
	Jpacket packet_data;
	packet_data.val["cmd"] = SERVER_DZ_UPDATE_USER_INFO;
	int i = 0;
	for( std::map<int, int>::iterator iter = uid_win_lose.begin(); iter != uid_win_lose.end(); ++iter )
	{
		packet_data.val["players"][i]["uid"] = iter->first;
		packet_data.val["players"][i]["alter_money"] = iter->second;
		packet_data.val["players"][i]["al_board"] = 1;
		if( iter->second > 0 ){
			packet_data.val["players"][i]["al_win"] = 1;
			packet_data.val["players"][i]["alter_exp"] = win_exp;
		}else{
			packet_data.val["players"][i]["al_win"] = 0;
			packet_data.val["players"][i]["alter_exp"] = lose_exp;
		}
		++i;
	}
	packet_data.val["vid"] = vid;
	packet_data.val["roomid"] = tid;
	packet_data.end();
	pdk.game->send_to_datasvr( packet_data.tostring() );
	if( 0 != tally_type )
	{
		for( std::map<int, int>::iterator iter = uid_win_lose.begin(); iter != uid_win_lose.end(); ++iter )
		{
			std::map<int, Player*>::iterator iter2 = players.find( iter->first );
			if( iter2 != players.end() && iter2->second )
			{
				update_running_tally( iter->first, iter->second, iter2->second->money, tally_type );
			}
		}
	}
}

void Table::update_running_tally( int uid, long alter_value, long current_value, int tally_type, int alter_type )
{
	if( 0 == alter_value )
	{
		return;
	}
	Jpacket packet_tally;
	packet_tally.val["cmd"] = SERVER_UPDATE_DB_TALLY_REQ;
	packet_tally.val["uid"] = uid;
	packet_tally.val["tid"] = tid;
	packet_tally.val["vid"] = vid;
	packet_tally.val["zid"] = zid;
	packet_tally.val["alter_value"] = (double)alter_value;
	packet_tally.val["current_value"] =(double)current_value;
	packet_tally.val["tally_type"] = tally_type;
	packet_tally.val["alter_type"] = alter_type;
	packet_tally.val["innings"] = innings;
	packet_tally.end();
	pdk.game->send_to_datasvr( packet_tally.tostring() );
}


int Table::game_end()
{
	log.debug("game end...\n");
	ev_timer_stop(pdk.loop, &play_card_timer);
	ev_timer_stop(pdk.loop, &pre_pass_timer);
	ev_timer_stop(pdk.loop, &pass_timer);
	ev_timer_stop(pdk.loop, &robot_timer);
	state = END_GAME;
	win_seatid = cur_seat;
	Seat &seat = seats[cur_seat];
	Player *player = seat.player;
	count_money(player);
	last_players.clear();
	PGame::GameEndAck pGameEndAck;
	pGameEndAck.set_seatid(seat.seatid);
	pGameEndAck.set_fail(0);
	pGameEndAck.set_base_money(base_money);
	pGameEndAck.set_ratio(ratio);
	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = seats[i].player;
		PGame::CountMoney *pCountMoney = pGameEndAck.add_players();
		pCountMoney->set_seatid(seats[i].seatid);
		pCountMoney->set_uid(player->uid);
		pCountMoney->set_rmb(player->rmb);
		pCountMoney->set_money(player->money);
		pCountMoney->set_cur_money(seats[i].cur_money);
		pCountMoney->set_spring(seats[i].spring);
		map<int, Card>::iterator it = seats[i].hole_cards.cards.begin();
		map<int, Card>::iterator it_end = seats[i].hole_cards.cards.end();
		CardList card_l;
		card_l.index = 100;//表示剩余手牌
		for (; it != it_end; it++) {
			pCountMoney->add_holes(it->first);
			card_l.cards.push_back(it->first);
		}
		player->card_list[game_nums-1].push_back(card_l);
		last_players.insert(player->uid);
	}
	Ppacket ppack;
	pGameEndAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_GAME_END_BC, atoi(tid.c_str()));
	broadcast(NULL, ppack.data);
	save_video_data(ppack.header.cmd, ppack.body);
	ev_timer_again(pdk.loop, &video_timer);
	ev_timer_again(pdk.loop, &preready_timer);
	return 0;
}

void Table::map_to_json_array_spec(std::map<int, Card> &cards, Jpacket &packet, int index)
{
	std::map<int, Card>::iterator it;
	for (it = cards.begin(); it != cards.end(); it++) {
		Card &card = it->second;
		packet.val["players"][index]["holes"].append(card.value);
	}
}

int Table::handler_logout(Player *player)
{
	if (seat_players.find(player->uid) != seat_players.end()) {
		if (!can_logout) {
			log.error("handler logout error player in seat uid[%d] state[%d] tid[%s].\n", player->uid, state, tid.c_str());
			return -1;
		}
		handler_downtable(player);
	}
	PGame::LogoutSuccAck pLogoutSuccAck;
	pLogoutSuccAck.set_uid(player->uid);
	Ppacket ppack;
	pLogoutSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_LOGOUT_SUCC_BC);
	unicast(player, ppack.data);
	log.info("handler logout uid[%d] seatid[%d] money[%d] tid[%s].\n", player->uid, player->seatid, player->money, tid.c_str());
	return 0;
}

int Table::del_player(Player *player)
{
	if (players.find(player->uid) == players.end()) {
		log.error("del player talbe uid[%d] is error.\n", player->uid);
		return -1;
	}
	players.erase(player->uid);
	log.info("del_player .\n");
	if( 0 == players.size() )
	{
		return 1;
	}
	return 0;
}


int Table::handler_info(Player *player)
{
	Jpacket packet;
	packet.val["cmd"] = SERVER_UPDATE_INFO_BC;
	packet.val["uid"] = player->uid;
	packet.val["seatid"] = player->seatid;
	packet.val["money"] = player->money;
	packet.val["rmb"] = player->rmb;
	packet.end();
	//	broadcast(NULL, packet.tostring());
	if (seat_players.find(player->uid) != seat_players.end()) {
		broadcast(NULL, packet.tostring());
	} else {
		unicast(player, packet.tostring());
	}
	log.info("handler info uid[%d] seatid[%d] money[%d] tid[%s].\n", player->uid, player->seatid, player->money, tid.c_str());
	return 0;
}


int Table::handler_preready()//
{
	state = READY;
	log.info("handler_preready .state = READY \n");
	reset();
	for(int i = 0; i < MAX_PLAYERS ; i++){
		seats[i].reset();
	}
	std::vector<Player*> temp_del_vec;
	for(std::map<int, Player*>::iterator iter = seat_players.begin(); iter != seat_players.end(); iter++ ){
		Player *player = iter->second;
		if(/*player->client == NULL || */player->daemonize_flag || player->money < stand_money || player->down_tag == 1){
			temp_del_vec.push_back(player);
		}
	}
	for(std::vector<Player*>::iterator iter = temp_del_vec.begin(); iter != temp_del_vec.end(); iter++){
		handler_downtable(*iter);
	}
	temp_del_vec.clear();
	for(std::map<int, Player*>::iterator iter = players.begin(); iter != players.end(); iter++){
		// if(iter->second->client == NULL){
		// 	temp_del_vec.push_back(iter->second);
		// }
		iter->second->reset();
	}
	for(std::vector<Player*>::iterator iter = temp_del_vec.begin(); iter != temp_del_vec.end(); iter++){
		pdk.game->del_player(*iter);
	}
	Ppacket ppack;
	ppack.pack(SERVER_GAME_PREREADY_BC);
	broadcast(NULL, ppack.data);
	return 0;
}

int Table::handler_chat(Player *player)
{
	PGame::ChatReq pChatReq;
	if (!pChatReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_chat parse pChatReq error\n");
		return -1;
	}
	PGame::ChatAck pChatAck;
	pChatAck.set_uid(player->uid);
	pChatAck.set_seatid(player->seatid);
	pChatAck.set_name(player->name);
	pChatAck.set_sex(player->sex);
	pChatAck.set_text(pChatReq.text());
	pChatAck.set_chatid(pChatReq.chatid());
	Ppacket ppack;
	pChatAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_CHAT_BC);
	broadcast(NULL, ppack.data);
	return 0;
}

int Table::handler_face(Player *player)
{
	PGame::FaceReq pFaceReq;
	if (!pFaceReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_face parse pFaceReq error\n");
		return -1;
	}
	PGame::FaceAck pFaceAck;
	pFaceAck.set_uid(player->uid);
	pFaceAck.set_seatid(player->seatid);
	pFaceAck.set_name(player->name);
	pFaceAck.set_sex(player->sex);
	pFaceAck.set_faceid(pFaceReq.faceid());
	Ppacket ppack;
	pFaceAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_FACE_BC);
	broadcast(NULL, ppack.data);
	return 0;
}

int Table::handler_interaction_emotion(Player *player)
{
	PGame::EmotionReq pEmotionReq;
	if (!pEmotionReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_interaction_emotion parse pEmotionReq error\n");
		return -1;
	}
	PGame::EmotionAck pEmotionAck;
	pEmotionAck.set_seatid(player->seatid);
	pEmotionAck.set_money(player->money);
	pEmotionAck.set_target_seatid(pEmotionReq.target_seatid());
	pEmotionAck.set_type(pEmotionReq.type());
	Ppacket ppack;
	pEmotionAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_EMOTION_BC);
	broadcast(NULL, ppack.data);
	return 0;
}

void Table::handler_all_players_info( Player *player )
{
	Json::Value &val = player->client->packet.tojson();
	int start_index = val["start_index"].asInt();
	int end_index = val["end_index"].asInt();
	int all_size = (int) players.size();
	if( start_index < 0 || start_index > end_index || start_index >= all_size )
	{
		log.error("handler all players message error start_index[%d] end_index[%d].\n", start_index, end_index );
		return;
	}

	if( end_index > all_size )
	{
		end_index = all_size;
	}
	std::map<int, Player*>::iterator iter = players.begin();
	std::map<int, Player*>::iterator iter2 = iter;
	std::advance( iter, start_index );
	std::advance( iter2, end_index );
	Jpacket packet;
	packet.val["cmd"] = SERVER_ALL_PLAYER_INFO_UC;
	int i = 0;
	for( std::map<int, Player*>::iterator iter3 = iter; iter3 != iter2 && iter3 != players.end(); iter3++ )
	{
		if( iter3->second && 0 == iter3->second->zid  )
		{
			packet.val["players"][i]["uid"] = iter3->second->uid;
			packet.val["players"][i]["name"] = iter3->second->name;
			packet.val["players"][i]["avatar"] = iter3->second->avatar;
			packet.val["players"][i]["money"] = (double)iter3->second->money;
			packet.val["players"][i]["sex"] = iter3->second->sex;
			i++;
		}
	}
	packet.val["left_len"] = all_size - end_index;
	packet.end();
	unicast( player, packet.tostring() );
}

void Table::handler_detal_info( Player *player )
{
	Json::Value &val = player->client->packet.tojson();
	int uid = val["uid"].asInt();
	std::map<int, Player*>::iterator iter = players.find( uid );
	if( iter != players.end() )
	{
		Jpacket detal_info_packet;
		detal_info_packet.val["cmd"] = SERVER_DETAL_INFO_UC;
		detal_info_packet.val["uid"] = uid;
		detal_info_packet.val["name"] = iter->second->name;
		detal_info_packet.val["avatar"] = iter->second->avatar;
		detal_info_packet.val["money"] = (double)iter->second->money;
		detal_info_packet.end();
		unicast( player, detal_info_packet.tostring() );
	}
	else
	{
		Jpacket detal_info_packet;
		detal_info_packet.val["cmd"] = SERVER_DETAL_INFO_ERROR_UC;
		detal_info_packet.val["uid"] = uid;
		detal_info_packet.end();
		unicast( player, detal_info_packet.tostring() );
	}
}


int Table::split_holecards(HoleCards &holecards,vector<vector<Card> > &heap_group)
{
	//cout<<"start split_heaps"<<endl;
	//order: 1. boom 2. line/three
	std::vector<Card> cards_temp1 ;
	holecards.copy_cards(&cards_temp1);
	CardStatistics card_stat;
	card_stat.statistics(cards_temp1);
	boom_heaps(card_stat,cards_temp1,heap_group);
	three_heaps(card_stat,cards_temp1,heap_group);
	line_heaps(card_stat,cards_temp1,heap_group);
	card_stat.clear();
	card_stat.statistics(cards_temp1);
	two_heaps(card_stat,cards_temp1,heap_group);
	one_heaps(card_stat,cards_temp1,heap_group);
	return heap_group.size();
}


void Table::boom_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group)
{
	int boom_count = card_stat.card4.size();
	for (int i = 0; i < boom_count; i += 4){
		std::vector<Card> heap_card;
		heap_card.push_back(card_stat.card4[i]);
		heap_card.push_back(card_stat.card4[i+1]);
		heap_card.push_back(card_stat.card4[i+2]);
		heap_card.push_back(card_stat.card4[i+3]);
		remove_card(cards_temp,card_stat.card4[i]);
		remove_card(cards_temp,card_stat.card4[i+1]);
		remove_card(cards_temp,card_stat.card4[i+2]);
		remove_card(cards_temp,card_stat.card4[i+3]);
		heap_group.push_back(heap_card);
		return;
	}
}

void Table::three_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group)
{
	int three_count = card_stat.card3.size();
	for (int i = 0;i < three_count; i += 3){
		std::vector<Card> heap_card;
		heap_card.push_back(card_stat.card3[i]);
		heap_card.push_back(card_stat.card3[i+1]);
		heap_card.push_back(card_stat.card3[i+2]);
		remove_card(cards_temp,card_stat.card3[i]);
		remove_card(cards_temp,card_stat.card3[i+1]);
		remove_card(cards_temp,card_stat.card3[i+2]);
		if (card_stat.card1.size()>1){
			heap_card.push_back(card_stat.card1[0]);
			heap_card.push_back(card_stat.card1[1]);
			remove_card(cards_temp,card_stat.card1[0]);
			remove_card(cards_temp,card_stat.card1[1]);
			heap_group.push_back(heap_card);
			card_stat.clear();
			card_stat.statistics(cards_temp);
			continue;
		}
		if (card_stat.card2.size()>0){
			heap_card.push_back(card_stat.card2[0]);
			heap_card.push_back(card_stat.card2[1]);
			remove_card(cards_temp,card_stat.card2[0]);
			remove_card(cards_temp,card_stat.card2[1]);

			heap_group.push_back(heap_card);
			card_stat.clear();
			card_stat.statistics(cards_temp);
			continue;
		}
		heap_group.push_back(heap_card);
		card_stat.clear();
		card_stat.statistics(cards_temp);
	}
}

void Table::line_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group)
{
	std::vector<Card> heap_card;
	CardFind::get_straight(card_stat,heap_card);
	if (heap_card.size() > 0){
		heap_group.push_back(heap_card);
		remove_card_v(cards_temp,heap_card);
	}
}

void Table::two_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group)
{
	int two_count = card_stat.card2.size();
	for (int i = 0;i < two_count; i += 2){
		std::vector<Card> heap_card;
		heap_card.push_back(card_stat.card2[i]);
		heap_card.push_back(card_stat.card2[i+1]);
		remove_card(cards_temp,card_stat.card2[i]);
		remove_card(cards_temp,card_stat.card2[i+1]);
		heap_group.push_back(heap_card);
	}
}

void Table::one_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group)
{
	int one_count = card_stat.card1.size();
	for (int i = 0; i < one_count; i++){
		std::vector<Card> heap_card;
		heap_card.push_back(card_stat.card1[i]);
		remove_card(cards_temp,card_stat.card1[i]);
		heap_group.push_back(heap_card);
	}
}


bool Table::remove_card(std::vector<Card> &cards,Card &card)
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

bool Table::remove_card_v(std::vector<Card> &cards,std::vector<Card> &heap_cards)
{
	//cout<<"remove_card_v"<<endl;
	for(vector<Card>::iterator it=heap_cards.begin(); it!=heap_cards.end();)
	{
		Card temp = *it;
		remove_card(cards,temp);
		++it;
	}
	return true;
}


int Table::handler_dissolve_room(Player *player)
{
	log.info("player[%d] apply dissolve room[%s]\n", player->uid, tid.c_str());
	Jpacket pac;
	if (room_state == 0) {
		// if (player->uid == owner_uid) {
			pac.val["cmd"] = SERVER_DZ_DISSOLVE_ROOM_REQ;
			pac.val["roomid"] = tid;
			pac.val["vid"] = vid;
			pac.end();
			pdk.game->send_to_datasvr(pac.tostring());
		// }
		return 0;
	}
	if (player->dissolve_cnt >= 2) {
		PGame::ApplyDisRoomErrAck pApplyDisRoomErrAck;
		pApplyDisRoomErrAck.set_code(PGame::DIS_ROOM_MORETHAN_TWICE_ERR);
		Ppacket ppack;
		pApplyDisRoomErrAck.SerializeToString(&ppack.body);
		ppack.pack(SERVER_APPLY_DISSOLVE_ROOM_ERR_UC);
		unicast(player, ppack.data);
		return 0;
	}
	if (dissovle_state == 1 || seat_players.find(player->uid) == seat_players.end() || is_dissolved == 1) {
		log.error("player[%d] apply dissovling room error\n", player->uid);
		return -1;
	}
	player->dissolve_cnt++;
	dissovle_state = 1;
	dissolve_applyer = player->uid;
	voters_players.clear();
	PGame::ApplyDisRoomSuccAck pApplyDisRoomSuccAck;
	pApplyDisRoomSuccAck.set_uid(player->uid);
	pApplyDisRoomSuccAck.set_remain_time(dissolve_room_tstamp);
	std::map<int, Player*>::iterator it = seat_players.begin();
	for (; it != seat_players.end(); it++) {
		if (it->second != player /*&& it->second->is_offline == 0*/ ) {
			pApplyDisRoomSuccAck.add_voters_uid(it->first);
			voters_players[it->first] = it->second;
		}
	}
	Ppacket ppack;
	pApplyDisRoomSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_APPLY_DISSOLVE_ROOM_SUCC_BC);
	broadcast(NULL, ppack.data);
	ev_timer_again(pdk.loop, &dissolve_room_timer);
	check_dissovle_result();
	return 0;
}

int Table::handler_dissolve_action(Player *player)
{
	if (dissovle_state != 1) {
		log.error("player[%d] dissovle_state[%d] is error\n", player->uid, dissovle_state);
		return -1;
	}
	if (player->dissovle_state != 2) {
		log.error("player[%d] vote error\n", player->uid);
		return -1;
	}
	PGame::VoteDisRoomReq pVoteDisRoomReq;
	if (!pVoteDisRoomReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_dissolve_action parse pVoteDisRoomReq error\n");
		return -1;
	}
	int uid = player->uid;
	int act = pVoteDisRoomReq.action();
	if (voters_players.find(uid) == voters_players.end()) {
		log.error("voters_uid not contain uid[%d]\n", uid);
		return -1;
	}
	player->dissovle_state = act;
	PGame::VoteDisRoomAck pVoteDisRoomAck;
	pVoteDisRoomAck.set_uid(uid);
	pVoteDisRoomAck.set_action(act);
	Ppacket ppack;
	pVoteDisRoomAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_DISSOLVE_ACTION_SUCC_BC);
	broadcast(NULL, ppack.data);
	check_dissovle_result();
	return 0;
}

void Table::check_dissovle_result()
{
	unsigned int _cnt_yes = 0;
	unsigned int _cnt_no = 0;
	std::map<int, Player*>::iterator it = voters_players.begin();
	for (; it != voters_players.end(); it++) {
		if (it->second->dissovle_state == 0) {
			_cnt_no++;
		}
		else if (it->second->dissovle_state == 1) {
			_cnt_yes++;
		}
	}
	if (_cnt_yes + _cnt_no == voters_players.size() || _cnt_no > 0) {
		ev_timer_stop(pdk.loop, &dissolve_room_timer);
		PGame::DisRoomResultAck pDisRoomResultAck;
		pDisRoomResultAck.set_result(_cnt_yes == voters_players.size() ? 1 : 0);
		Ppacket ppack;
		pDisRoomResultAck.SerializeToString(&ppack.body);
		ppack.pack(SERVER_DISSOLVE_ROOM_RESULT_BC);
		broadcast(NULL, ppack.data);
		if (_cnt_yes == voters_players.size()) {
			Jpacket pac;
			pac.val["cmd"] = SERVER_DZ_DISSOLVE_ROOM_REQ;
			pac.val["roomid"] = tid;
			pac.val["vid"] = vid;
			pac.end();
			pdk.game->send_to_datasvr(pac.tostring());
		}
		dissovle_state = 0;
		std::map<int, Player*>::iterator _it = seat_players.begin();
		for (; _it != seat_players.end(); _it++) {
			_it->second->dissovle_state = 2;
		}
	}
}

void Table::dissolve_room_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*)w->data;
	ev_timer_stop(pdk.loop, &table->dissolve_room_timer);
	std::map<int, Player*>::iterator it = table->voters_players.begin();
	for (; it != table->voters_players.end(); it++) {
		if (it->second->dissovle_state == 2) {
			it->second->dissovle_state = 1;
			table->check_dissovle_result();
		}
	}
}

void Table::save_game_start_video_data()
{
	PGame::TableInfo pTableInfo;
	pTableInfo.set_spade3_first_out(spade3_first_out);
	pTableInfo.set_table_type(table_type);
	pTableInfo.set_single_rule(single_rule);
	pTableInfo.set_bomb_can_split(bomb_can_split);
	pTableInfo.set_base_money(base_money);
	pTableInfo.set_stand_money(stand_money);
	pTableInfo.set_takein(takein);
	pTableInfo.set_banker(banker);
	pTableInfo.set_max_players(MAX_PLAYERS);
	std::map<int, Player*>::iterator it;
	for (it = seat_players.begin(); it != seat_players.end(); it++) {
		Player *p;
		p = it->second;
		if (p == NULL)
		  	continue;
		PGame::UserInfo *pUserInfo = pTableInfo.add_players();
		pUserInfo->set_seatid(p->seatid);
		pUserInfo->set_uid(p->uid);
		pUserInfo->set_name(p->name);
		pUserInfo->set_sex(p->sex);
		pUserInfo->set_avatar(p->avatar);
		pUserInfo->set_money(p->money);
		//pUserInfo->set_offline(p->is_offline);
		std::map<int, Card>::iterator it_c = seats[p->seatid].hole_cards.cards.begin();
		for (; it_c != seats[p->seatid].hole_cards.cards.end(); it_c++) {
			pUserInfo->add_holes(it_c->second.value);
		}
	}
	for (std::set<int>::iterator it_s = player_option.begin(); it_s != player_option.end(); it_s++) {
		pTableInfo.add_player_option(*it_s);
	}
	Ppacket ppack;
	pTableInfo.SerializeToString(&ppack.body);
	ppack.pack(SERVER_TABLE_INFO_UC);
	save_video_data(ppack.header.cmd, ppack.body);
}

void Table::save_video_data(int cmd, std::string str)
{
	ostringstream oss;
	oss << cmd << ":S:" << str << ":E:";
	m_videostr += oss.str();
}

void Table::send_video_to_svr()
{
	//base64编码
	static unsigned char buf[MaxVideoBufLen] ={0};
	int noutlen = MaxVideoBufLen;
	int iRet = base64_encode((const unsigned char *)m_videostr.c_str(), m_videostr.size(), (unsigned char *)buf, &noutlen);
	if (iRet != CRYPT_OK)
	{
		log.error("send video to svr base64_encode is error iRet:%d .\n", iRet);
		return;
	}

	Jpacket packet_data;
	packet_data.val["cmd"] = SERVER_VIDEO_DATA_SAVE;
	packet_data.val["roomid"] = tid;
	std::string packStr((char *)buf,noutlen);
	packet_data.val["content"] = packStr;
	char strbuf[50] = {0};
	sprintf(strbuf,"%s_%04d",tid.c_str(), game_nums - 1);
	std::string strTimes = strbuf;
	packet_data.val["innings"] = strTimes;
	packet_data.end();
	pdk.game->send_to_videosvr( packet_data.tostring() );
	log.info( "send video to svr m_videostr.size[%d] noutlen[%d] .\n", m_videostr.size(), noutlen );
}

void Table::handler_settlement(Player* player)
{
	timeval tv;
	gettimeofday(&tv, NULL);
	end_time = tv.tv_sec;
	PGame::SettleInfo pSettleInfo;
	pSettleInfo.set_owner_uid(owner_uid);
	pSettleInfo.set_end_time(end_time);
	for (int i = 0; i < MAX_PLAYERS;i++) {
		Player *p = seats[i].player;
		if (p == NULL) {
			continue;
		}
		PGame::UserInfo *pUserInfo = pSettleInfo.add_players();
		pUserInfo->set_uid(p->uid);
		pUserInfo->set_seatid(p->seatid);
		pUserInfo->set_name(p->name);
		pUserInfo->set_avatar(p->avatar);
		pUserInfo->set_sex(p->sex);
		pUserInfo->set_max_score(p->settlement.max_score);
		pUserInfo->set_bomb_cnt(p->settlement.bomb_cnt);
		pUserInfo->set_total_win(p->settlement.total_win);
		pUserInfo->set_total_lose(p->settlement.total_lose);
		pUserInfo->set_total_score(p->settlement.total_score);
	}
	Ppacket ppack;
	pSettleInfo.SerializeToString(&ppack.body);
	ppack.pack(SERVER_GAME_SETTLEMENT_INFO);
	unicast(player, ppack.data);
}

void Table::handler_settle_list(Player *player)
{
	PGame::SettleListAck pSettleListAck;
	for (int i = 0; i < MAX_PLAYERS;i++) {
		Player *p = seats[i].player;
		if (p == NULL) {
			continue;
		}
		PGame::PlayerSettle *pPlayerSettle = pSettleListAck.add_players();
		PGame::UserInfo *pUserInfo = pPlayerSettle->mutable_player();
		pUserInfo->set_uid(p->uid);
		pUserInfo->set_seatid(p->seatid);
		pUserInfo->set_name(p->name);
		pUserInfo->set_avatar(p->avatar);
		pUserInfo->set_sex(p->sex);
		pUserInfo->set_total_score(p->settlement.total_score);
		for (int i = 1; i < game_nums; i++) {
			PGame::SettleList *pSettleList = pPlayerSettle->add_settle_list();
			pSettleList->set_index(i);
			pSettleList->set_cur_money(p->settle_list[i].cur_money);
			
		}
	}
	for (int i = 1; i < game_nums; i++) {
		pSettleListAck.add_heart10_seatid(heart10_owner_map[i]);
	}
	Ppacket ppack;
	pSettleListAck.SerializeToString(&ppack.body);
	ppack.pack(SERVERT_SETTLE_LIST_ACK);
	//log.debug("settle list %s\n", pSettleListAck.DebugString().c_str());
	unicast(player, ppack.data);
}

//第x局手牌详情请求
void Table::handler_card_list(Player *player)
{
	PGame::CardListReq pCardListReq;
	if (!pCardListReq.ParseFromString(player->client->ppacket.body)) {
		log.error("in handler_card_list parse CardListAck error\n");
		return;
	}
	if (!pCardListReq.has_index())
	{
		log.error("handler_card_list CardListAck not have index\n");
		return;
	}
	int index = pCardListReq.index();
	PGame::CardListAck pCardListAck;
	pCardListAck.set_heart10_seatid(heart10_owner_map[index]);
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		Player *p = seats[i].player;
		if (p == NULL) 
		{
			continue;
		}
		PGame::TotalCardList *pTotalCardList = pCardListAck.add_players();
		pTotalCardList->set_cur_money(p->settle_list[index].cur_money);
		pTotalCardList->set_bomb_money(p->settle_list[index].bomb_money);
		PGame::UserInfo *pUserInfo = pTotalCardList->mutable_player();
		pUserInfo->set_uid(p->uid);
		pUserInfo->set_seatid(p->seatid);
		pUserInfo->set_name(p->name);
		pUserInfo->set_avatar(p->avatar);
		pUserInfo->set_sex(p->sex);
		for (unsigned int j = 0; j < p->card_list[index].size(); j++) 
		{
			PGame::CardList *pCardList = pTotalCardList->add_out_cards();
			pCardList->set_index(p->card_list[index][j].index);
			for (unsigned int k = 0; k < p->card_list[index][j].cards.size(); k++) 
			{
				pCardList->add_cards(p->card_list[index][j].cards[k]);
			}
		}
	}
	Ppacket ppack;
	pCardListAck.SerializeToString(&ppack.body);
	ppack.pack(SERVERT_CARD_LIST_ACK);
	//log.debug("settle list %s\n", pSettleListAck.DebugString().c_str());
	unicast(player, ppack.data);

}

void Table::video_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Table *table = (Table*)w->data;
	ev_timer_stop(pdk.loop, &table->video_timer);
	table->send_video_to_svr();
}

void Table::game_restart_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{   
	Table *table = (Table*) w->data;
	ev_timer_stop(pdk.loop, &table->game_restart_timer);
	table->game_start_req();
}
