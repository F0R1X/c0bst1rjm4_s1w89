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
#include <sstream>

#include "pdk.h"
#include "log.h"
#include "game.h"
#include "proto.h"
#include "client.h"
#include "player.h"
#include "table.h"
#include "GameInfo.pb.h"
#include "CommonInfo.pb.h"

extern PDK pdk;
extern Log log;

Game::Game()
{
	_fd = 0;
	link_data_client = NULL;
	ev_reconnect_datasvr_tstamp = 0.3;
	ev_reconnect_datasvr_timer.data = this;
	ev_timer_init( &ev_reconnect_datasvr_timer, Game::reconnect_datasvr_cb, ev_reconnect_datasvr_tstamp, ev_reconnect_datasvr_tstamp );
	server_zid = pdk.conf["tables"]["zid"].asInt();
	server_vid = pdk.conf["tables"]["vid"].asInt();
	log.info("game svr zid[%d] vid[%d].\n", server_zid, server_vid );
	game_name = pdk.conf["game_name"].asString();
	game_number = pdk.conf["game_number"].asInt();
	ev_del_table_tstamp = 5 * 60;
	ev_del_table_timer.data = this;
	ev_timer_init( &ev_del_table_timer, Game::del_table_cb, ev_del_table_tstamp, 0 );
	link_video_client = NULL;
	ev_reconnect_videosvr_timer.data = this;
	ev_reconnect_videosvr_tstamp = 0.3;
	ev_timer_init( &ev_reconnect_videosvr_timer, Game::reconnect_videosvr_cb, ev_reconnect_videosvr_tstamp, ev_reconnect_videosvr_tstamp );
}

Game::~Game()
{
	if( NULL != link_data_client )
	{
		delete link_data_client;
	}
	if( NULL != link_video_client )
	{
		delete link_video_client;
	}
	ev_timer_stop( pdk.loop, &ev_reconnect_datasvr_timer );
	ev_timer_stop( pdk.loop, &ev_del_table_timer );
	ev_timer_stop( pdk.loop, &ev_reconnect_videosvr_timer);
}

int Game::start()
{
	connect_datasvr();
	connect_videosvr();
	init_accept();
	return 0;
}

void Game::del_table_cb( struct ev_loop *loop, struct ev_timer *w, int revents )
{
	Game *p_game = (Game*)w->data;
	p_game->del_table();
}
void Game::del_table()
{
	for( std::map<std::string, Table*>::iterator iter = del_tables_map.begin(); iter != del_tables_map.end(); )
	{
		Table* table = iter->second;
		std::map<std::string, Table*>::iterator iter2 = all_tables.find( table->tid );
		if( iter2 != all_tables.end() )
		{
			all_tables.erase( iter2 );
		}
		del_tables_map.erase(iter++);
		delete table;
	}
	log.debug("del tables.\n");
}

int Game::init_accept()
{
	log.info("Listening on %s:%d\n",
				pdk.conf["game"]["host"].asString().c_str(),
				pdk.conf["game"]["port"].asInt());

	struct sockaddr_in addr;

	_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (_fd < 0) {
		log.error("File[%s] Line[%d]: socket failed: %s\n", __FILE__, __LINE__, strerror(errno));
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(pdk.conf["game"]["port"].asInt());
	addr.sin_addr.s_addr = inet_addr(pdk.conf["game"]["host"].asString().c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE) {
		log.error("game::init_accept Incorrect ip address!");
		close(_fd);
		_fd = -1;
		exit(1);
	}

	int on = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		log.error("File[%s] Line[%d]: setsockopt failed: %s\n", __FILE__, __LINE__, strerror(errno));
		close(_fd);
		exit(1);
		return -1;
	}

	if (bind(_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		log.error("File[%s] Line[%d]: bind failed: %s\n", __FILE__, __LINE__, strerror(errno));
		close(_fd);
		exit(1);
		return -1;
	}

	fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);
	listen(_fd, 10000);

	_ev_accept.data = this;
	ev_io_init(&_ev_accept, Game::accept_cb, _fd, EV_READ);
	ev_io_start(pdk.loop, &_ev_accept);
	log.info("listen ok\n");
	return 0;
}

int Game::connect_datasvr()
{
	log.info("datasvr management ip[%s] port[%d].\n",
				pdk.conf["data-svr"]["host"].asString().c_str(),
				pdk.conf["data-svr"]["port"].asInt() );
	struct sockaddr_in addr;
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		log.error("File[%s] Line[%d]: socket failed: %s\n", __FILE__, __LINE__, strerror(errno));
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pdk.conf["data-svr"]["port"].asInt());
	addr.sin_addr.s_addr = inet_addr(pdk.conf["data-svr"]["host"].asString().c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		log.error("game::connect_datasvr Incorrect ip address!");
		close(fd);
		fd = -1;
		exit(1);
	}
	if( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(fd);
		log.error("connect data svr error: %s(errno: %d)\n", strerror(errno), errno);
		fd = -1;
		exit(1);
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	link_data_client = new(std::nothrow) Client( fd, false, 1);
	if( NULL != link_data_client )
	{
		log.info("connect data svr fd[%d].\n", fd );
		fd_client_map[link_data_client->fd] = link_data_client;
	}
	else
	{
		close(fd);
		log.error("new data svr client error.\n");
		exit(1);
	}
	return 0;

}

void Game::start_reconnect()
{
	ev_timer_again( pdk.loop, &ev_reconnect_datasvr_timer );
}

void Game::reconnect_datasvr_cb( struct ev_loop *loop, struct ev_timer *w, int revents )
{
	Game *p_game = (Game*)w->data;
	p_game->reconnect_datasvr();
}
int Game::reconnect_datasvr()
{
	struct sockaddr_in addr;
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		log.error("File[%s] Line[%d]: socket failed: %s\n", __FILE__, __LINE__, strerror(errno));
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pdk.conf["data-svr"]["port"].asInt());
	addr.sin_addr.s_addr = inet_addr(pdk.conf["data-svr"]["host"].asString().c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		log.error("game::reconnect_datasvr Incorrect ip address!");
		close(fd);
		fd = -1;
		return -1;
	}
	if( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(fd);
		log.error("reconnect datasvr error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	link_data_client = new (std::nothrow) Client( fd, false, 1);
	if( link_data_client )
	{
		log.info("reconnect datasvr link fd[%d].\n", fd );
		fd_client_map[link_data_client->fd] = link_data_client;
		ev_timer_stop(pdk.loop, &ev_reconnect_datasvr_timer );
	}
	return 0;
}
void Game::send_to_datasvr( const std::string &res )
{
	if( NULL != link_data_client )
	{
		link_data_client->send( res );
	}
}
void Game::del_all_client()
{
	std::map<int, Client*> temp_fd_client_map = fd_client_map;
	log.info("for disconnect to datasvr, delete all client fd_client_len[%d] temp_fd_client_map[%d]  .\n", fd_client_map.size(), temp_fd_client_map.size() );
	for(std::map<int, Client*>::iterator iter = temp_fd_client_map.begin(); iter != temp_fd_client_map.end(); iter++ )
	{
		Client::pre_destroy( iter->second );
	}
}
void Game::accept_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	if (EV_ERROR & revents) {
		log.error("got invalid event\n");
		return;
	}

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int fd = accept(w->fd, (struct sockaddr *) &client_addr, &client_len);
	if (fd < 0) {
		log.error("accept error[%s]\n", strerror(errno));
		return;
	}
	Game *game = (Game*) (w->data);
	if( NULL == game->link_data_client )
	{
		close(fd);
		log.error("accept client error for the link of datasvr disconnect.\n");
		return;
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

	Client *client = new (std::nothrow) Client(fd, true, 0);
	if (client)
	{
		client->string_ip = inet_ntoa( client_addr.sin_addr );
		game->fd_client_map[fd] = client;
	}
	else
	{
		close(fd);
	}
}

int Game::request_login(Client *client)
{
	PGame::LoginReq pLoginReq;
	if (!pLoginReq.ParseFromString(client->ppacket.body)) {
		log.error("in request_login pLoginReq parse error\n");
		return -1;
	}
	client->uid = pLoginReq.uid();
	client->skey = pLoginReq.skey();
	client->roomid = pLoginReq.roomid();
	log.info("client[%d] player[%d] request login roomid[%s]\n", client->fd, client->uid, client->roomid.c_str());
	if (all_tables.find(client->roomid) != all_tables.end()) {
		Table *table = all_tables[client->roomid];
		if (table->is_dissolved == 1) {
			log.error("player[%d] request login error room[%s] has dissovle\n", client->uid, client->roomid.c_str());
			dz_login_error_back(client, PGame::ROOM_STATUS_ERR);
			Client::pre_destroy(client);
			return -1;
		}
		if (table->players.find(client->uid) != table->players.end()) {
			Player *player = table->players[client->uid];
			if(player->skey == client->skey){
				client->set_positon(POSITION_TABLE);
				Client *oldClient = player->client;
				player->set_client(client);
				player->stop_offline_timer();
				player->stop_del_player_timer();
				player->down_tag = 0;
				//重连回来后自动上��?
				if (player->seatid < 0) {
					table->handler_apply_uptable(player);
				}
				table->handler_login_succ_bc(player);
				table->handler_table_info(player);
				table->handler_login_succ_uc(player);
				if( oldClient && oldClient->fd != client->fd ){
					SendLogout(oldClient, client->uid, -1);
					oldClient->player = NULL;
					Client::pre_destroy(oldClient, true);
				}
				return 0;
			}
		}
	}
	if(uid_client_map.find(client->uid) != uid_client_map.end()){
		if( uid_client_map[client->uid]->fd != client->fd ){
			del_client(uid_client_map[client->uid]);
		}
	}
	uid_client_map[client->uid] = client;
	//json发送给平台验证
	Jpacket jpac;
	jpac.val["cmd"] = CLIENT_DZ_LOGIN_REQ;
	jpac.val["uid"] = client->uid;
	jpac.val["skey"] = client->skey;
	jpac.val["roomid"] = client->roomid;
	jpac.val["vid"] = server_vid;
	jpac.val["zid"] = server_zid;
	int isGetRoomInfo = 0;
	if(all_tables.find(client->roomid) == all_tables.end()){
		isGetRoomInfo = 1;
	}
	jpac.val["isGetRoomInfo"] = isGetRoomInfo;
	jpac.end();
	send_to_datasvr(jpac.tostring());
	return 0;
}

int Game::login_succ_back( Json::Value &val )
{
	int uid = val["uid"].asInt();
	std::map<int,Client*>::iterator iter = uid_client_map.find( uid );
	if( iter == uid_client_map.end() ){
		log.error("the player uid[%d] socket is not in map.\n", uid );
		return 0;
	}
	Client *client = iter->second;
	uid_client_map.erase(uid);
	if (!val["roomStatus"].isNull()){
		int room_status = val["roomStatus"].asInt();
		//房间已经结束或者解��?
		if( room_status >= 2 ){
			log.error("roomid[%s] status[%d] is over or dissovle\n", client->roomid.c_str(), room_status);
			dz_login_error_back(client, PGame::ROOM_STATUS_ERR);
			Client::pre_destroy(client);
			return -1;
		}
	}
	if( all_tables.find( client->roomid ) != all_tables.end() ){
		Table* table = all_tables[client->roomid];
		if( table->players.find( client->uid) != table->players.end() ){
			log.debug("login succ back rebind roomid[%s] uid[%d] get info ok\n", client->roomid.c_str(), uid);
			Player *player = table->players[client->uid];
			client->set_positon(POSITION_TABLE);
			Client *oldClient = player->client;
			player->set_client(client);
			player->stop_offline_timer();
			player->stop_del_player_timer();
			player->down_tag = 0;
			player->skey = client->skey;
			//重连回来后自动上��?
			if (player->seatid < 0) {
				table->handler_apply_uptable(player);
			}
			table->handler_login_succ_bc(player);
			table->handler_table_info(player);
			table->handler_login_succ_uc(player);
			if( oldClient && oldClient->fd != client->fd ){
				SendLogout(oldClient, client->uid, -1);
				oldClient->player = NULL;
				Client::pre_destroy(oldClient, true);
			}
			return 0;
			//del_client( client );
			//return 0;
		}
	}
	Player *player = new (std::nothrow) Player();
	if (player == NULL){
		log.error("new player err");
		return 0;
	}
	int ret = player->init( val );
	if( 0 != ret ){
		dz_login_error_back(client, PGame::USER_NO_MONEY_LOGIN_ERR);
		Client::pre_destroy(client);
	}
	player->set_client( client );
	log.info("one player[%d] new success.\n", uid );
	handler_login_table( client, val );
	return 0;
}

int Game::dz_login_error_back( Client *client, int code)
{
	PGame::LoginErrAck pLoginErrAck;
	pLoginErrAck.set_uid(client->uid);
	pLoginErrAck.set_code(code);
	Ppacket ppack;
	pLoginErrAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_DZ_LOGIN_ERR_RES);
	client->send(ppack.data);
	return 0;
}

int Game::login_error_back(int uid, int code)
{
	std::map<int, Client*>::iterator iter = uid_client_map.find(uid);
	if (iter == uid_client_map.end()) {
		log.error("uid[%d] is not in socket map\n", uid);
		return 0;
	}
	Client *client = iter->second;
	dz_login_error_back(client, code);
	//广播退出房间协��?防止客户端断线后又发登录请求，一直死循环
	SendLogout(client, client->uid, -1);
	Client::pre_destroy(client);
	return 0;
}

void Game::del_client(Client *client, bool del_oldclient)
{
	//顶号问题
	if ( link_data_client && client->fd != link_data_client->fd && (del_oldclient || client->cmd_type == 1))
	{
		log.info("delete old client, fd:%d \n", client->fd);
		fd_client_map.erase(client->fd);
		delete client;
		client = NULL;
		return;
	}
	if ( fd_client_map.find(client->fd) == fd_client_map.end() )
	{
		log.error("del client free client err[miss] fd[%d].\n", client->fd );
		return;
	}
	fd_client_map.erase(client->fd);
	if( uid_client_map.find( client->uid ) != uid_client_map.end() )
	{
		uid_client_map.erase( client->uid );
	}
	int i=client->fd;
	std::string room_id = client->roomid;
	int flag = 0;
	int flag_v = 0;
	if( link_data_client )
	{
		flag = ( i == link_data_client->fd) ? 1:0;
	}
	if (link_video_client) {
		flag_v = ( i == link_video_client->fd) ? 1:0;
	}
	// todo
	if (client->player)
	{
		Player *player = client->player;
		if (client->position == POSITION_WAIT)
		{
			del_player(player);
		}
		else
		{
			player->client = NULL;
			player->start_offline_timer();
			//player->start_del_player_timer();
		}
	}

	log.info("del client fd[%d].\n", client->fd);
	delete client;
	if( flag == 1){
		link_data_client = NULL;
		del_all_client();
		start_reconnect();
	}
	if (flag_v == 1) {
		link_video_client = NULL;
		start_videoreconnect();
	}
	client = NULL;
	if (table_no_players(room_id)) {
		Jpacket pac;
		pac.val["cmd"] = SERVER_NO_PLAYERS;
		pac.val["roomid"] = room_id;
		pac.val["vid"] = server_vid;
		pac.end();
		send_to_datasvr(pac.tostring());
	}
}

int Game::table_no_players(std::string _roomid)
{
	if (all_tables.find(_roomid) == all_tables.end()) {
		log.error("table no players room[%s] is not find\n", _roomid.c_str());
		return 2;
	}
	Table *temp_table = all_tables[_roomid];
	std::map<int, Player*>::iterator it = temp_table->players.begin();
	for (; it != temp_table->players.end(); it++) {
		if (it->second->client != NULL) {
			return 0;
		}
	}
	return 1;
}

int Game::handler_del_table(Json::Value &val)
{
	if (!val["roomid"].isString()) {
		log.error("roomid is not string\n");
		return -1;
	}
	std::string room_id = val["roomid"].asString();
	if (val["vid"].asInt() != server_vid) {
		log.error("vid[%d] is not my vid[%d]\n", val["vid"].asInt(), server_vid);
		return -1;
	}
	int ret = table_no_players(room_id);
	Jpacket pac;
	pac.val["cmd"] = SERVER_DEL_TABLE_RES;
	pac.val["roomid"] = room_id;
	pac.val["vid"] = server_vid;
	pac.val["type"] = ret > 0 ? 1 : 0;
	pac.end();
	send_to_datasvr(pac.tostring());
	if (ret == 1) {
		Table *tm = all_tables[room_id];
		all_tables.erase(room_id);
		delete tm;
		tm = NULL;
		log.info("del table[%s]\n", room_id.c_str());
	}
	return 0;
}

int Game::dispatch(Client *client)
{
	client->cmd_type = 0;
	int cmd = client->packet.sefe_check();
	if (cmd < 0) {
		log.error("the cmd format is error.\n");
		return -1;
	}
	switch(cmd){
		case SERVER_DZ_LOGIN_ERR_RES:
		{
			int c_uid = client->packet.val["uid"].asInt();
			int c_code = client->packet.val["code"].asInt();
			login_error_back(c_uid, c_code);
			break;
		}
		case SERVER_DZ_LOGIN_SUCC_RES:
			login_succ_back( client->packet.tojson() );
			break;
		case SERVER_DZ_GAME_START_RES:
		case SERVER_DZ_DISSOLVE_ROOM_RES:
			game_start_res( client->packet );
			break;
		case SERVER_DEL_TABLE_REQ:
			handler_del_table(client->packet.tojson());
			break;
		case SERVER_LOGOUT_ROOM_RES:
		{
			Json::Value &val = client->packet.tojson();
			log.info("recv tid[%s] player[%d] logout status[%d] is_succ[%d]\n",
			val["roomid"].asString().c_str(), val["uid"].asInt(), val["status"].asInt(), val["is_success"].asInt());
			break;
		}
		default:
			log.error("datasvr error cmd[%d]\n", cmd);
			break;
	}
	return 0;
}

int Game::dispatch2(Client *client)
{
	client->cmd_type = 0;
	int cmd = client->ppacket.safe_check();
	if (cmd < 0) {
		log.error("the cmd format is error.\n");
		return -1;
	}
	if( link_data_client && client->fd == link_data_client->fd ){
		return 0;
	}
	if( link_video_client && client->fd == link_video_client->fd ){
		return 0;
	}
	if ( cmd == CLIENT_DZ_LOGIN_REQ ){
		return request_login( client );
	}
	if( cmd == CLINET_HEART_BEAT_REQ ){
		Ppacket ppack;
		ppack.pack(SERVER_HEART_BEAT_RES);
		return client->send(ppack.data);
	}
	if (safe_check(client, cmd) < 0) {
		return -1;
	}
	Player *player = client->player;
	/* dispatch */
	switch (cmd) {
		case CLIENT_READY_REQ:
			all_tables[player->tid]->handler_ready(player);
			break;
		case CLIENT_CHAT_REQ:
			all_tables[player->tid]->handler_chat(player);
			break;
		case CLIENT_FACE_REQ:
			all_tables[player->tid]->handler_face(player);
			break;
		case CLIENT_LOGOUT_REQ:
			del_player(player);
			break;
		case CLIENT_TABLE_INFO_REQ:
			all_tables[player->tid]->handler_table_info(player);
			break;
		case CLIENT_EMOTION_REQ:
			all_tables[player->tid]->handler_interaction_emotion(player);
			break;
		case CLIENT_UPTABLE_APPLY_REQ:
			all_tables[player->tid]->handler_apply_uptable(player);
			break;
		case CLIENT_DOWNTABLE_REQ:
			all_tables[player->tid]->handler_downtable(player);
			break;
		case CLIENT_PLAY_CARD_REQ:
			all_tables[player->tid]->handler_play_card(player);
			break;
		case CLIENT_ROBOT_REQ:
			all_tables[player->tid]->handler_robot(player);
			break;
		case CLIENT_ALL_PLAYERS_REQ:
			all_tables[player->tid]->handler_all_players_info( player );
			break;
		case CLIENT_DETAL_INFO_REQ:
			all_tables[player->tid]->handler_detal_info( player );
			break;
		case CLIENT_DISSOLVE_ROOM_REQ:
			all_tables[player->tid]->handler_dissolve_room(player);
			break;
		case CLIENT_DISSOLVE_ACTION_REQ:
			all_tables[player->tid]->handler_dissolve_action(player);
			break;
		case CLIENT_SETTLEMENT_INFO_REQ:
			all_tables[player->tid]->handler_settlement(player);
			break;
		case CLIENT_SETTLE_LIST_REQ:
			all_tables[player->tid]->handler_settle_list(player);
			break;
		case CLIENT_CARD_LIST_REQ:
			all_tables[player->tid]->handler_card_list(player);
			break;
		default:
			log.error("invalid command[%d]\n", cmd);
			return -1;
	}
	return 0;
}

int Game::safe_check(Client *client, int cmd)
{
	Player *player = client->player;
	if (player == NULL) {
		log.error("safe check client player is NULL.\n");
		return -1;
	}
	if (all_tables.find(player->tid) == all_tables.end()){
		log.error("safe_check uid[%d] is not in tid[%s]\n", player->uid, player->tid.c_str());
		return -1;
	}
	return 0;
}

int Game::handler_login_table(Client *client, Json::Value &val)
{
	if (client == NULL) {
		log.error("client is null login table error\n");
		return -1;
	}
	Player *player = client->player;
	if (player == NULL) {
		log.error("client fd[%d] player is null login table error\n", client->fd);
		return -1;
	}
	if (client->position == POSITION_TABLE) {
		log.error("handler_login_table uid[%d] have been in table\n", player->uid);
		return -1;
	}
	return login_table(client, val);
}

int Game::login_table(Client *client, Json::Value &val)
{
	if (client == NULL) {
		log.error("login table client is null\n");
		return -1;
	}
	Player *player = client->player;
	std::string roomid = client->roomid;
	if( !roomid.empty() ){
		if( all_tables.find( roomid ) == all_tables.end() ){
			Table *table = new (std::nothrow)Table();
			table->init( val, client );
			all_tables[roomid] = table;
			log.info("total tables[%d].\n", all_tables.size() );
		}
		client->set_positon( POSITION_TABLE );
		all_tables[roomid]->handler_login( player );
		return 0;
	}
	log.error( "login table error uid[%d] roomid is NULL.\n", player->uid );
	return -1;
}

int Game::del_player(Player *player)
{
	int ret = 0;
	int uid = player->uid;
	string roomid = player->tid;
	if (all_tables.find(roomid) != all_tables.end()){
		player->logout_type = 1;
		ret = all_tables[roomid]->handler_logout(player);
		if (ret < 0) {
			log.error("del player table handler logout error uid[%d].\n", player->uid);
			return -1;
		}
		if( NULL == player ){
			log.info("the function handler_logout have delete player.\n");
			return -1;
		}
		ret = all_tables[roomid]->del_player(player);
		if (ret < 0) {
			log.error("del player table del player\n");
			return -1;
		}
	}
	if (player->client) {
		Client *client = player->client;
		client->position = POSITION_WAIT;
		Client::pre_destroy(client);
	}
	log.info("del player[%p] uid[%d]\n", player, player->uid);
	delete player;
	player = NULL;
	//通知平台玩家已退出房��?
	Jpacket packet;
	packet.val["cmd"] = SERVER_LOGOUT_ROOM_REQ;
	packet.val["roomid"] = roomid;
	packet.val["uid"] = uid;
	packet.val["vid"] = server_vid;
	packet.end();
	send_to_datasvr(packet.tostring());
	log.info("tid[%s] player[%d] logout room send to datasvr\n", roomid.c_str(), uid);
	return 0;
}

void Game::del_all_play_client(std::string roomid)
{
	std::map<std::string, Table*>::iterator iter = all_tables.find(roomid);
	if (iter != all_tables.end()) {
		std::map<int, Player*> all_players_map = iter->second->players;
		for (std::map<int, Player*>::iterator it = all_players_map.begin(); it != all_players_map.end(); it++) {
			Player *player = it->second;
			Client *client = player->client;
			Client::pre_destroy(client);
		}
		log.info("dissovle room del tables..\n");
	}
}


void Game::game_start_res( Jpacket &packet )
{
	std::string roomid = packet.val["roomid"].asString();
	if( all_tables.find( roomid ) != all_tables.end() )
	{
		int ret = all_tables[roomid]->game_start_res( packet );
		if( ret < 0 )
		{
			//del_all_play_client(roomid);
		}
	}
}


void Game::SendLogout(Client *client, int uid, int code)
{
	//广播退出房间协��?防止客户端断线后又发登录请求，一直死循环
	PGame::LogoutSuccAck pLogoutSuccAck;
	pLogoutSuccAck.set_uid(uid);
	pLogoutSuccAck.set_seatid(-1);
	pLogoutSuccAck.set_type(code);
	Ppacket ppack;
	pLogoutSuccAck.SerializeToString(&ppack.body);
	ppack.pack(SERVER_LOGOUT_SUCC_BC);
	client->send(ppack.data);
}


int Game::connect_videosvr()
{
	if (pdk.conf["video-svr"].isNull())
	{
		return -1;
	}
	log.info("videosvr management ip[%s] port[%d].\n",
		pdk.conf["video-svr"]["host"].asString().c_str(),
		pdk.conf["video-svr"]["port"].asInt() );
	struct sockaddr_in addr;
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		log.error("File[%s] Line[%d]: socket failed: %s\n", __FILE__, __LINE__, strerror(errno));
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pdk.conf["video-svr"]["port"].asInt());
	addr.sin_addr.s_addr = inet_addr(pdk.conf["video-svr"]["host"].asString().c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		log.error("game::connect_videosvr Incorrect ip address!");
		close(fd);
		fd = -1;
		exit(1);
	}
	if( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(fd);
		log.error("connect video svr error: %s(errno: %d)\n", strerror(errno), errno);
		fd = -1;
		exit(1);
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	link_video_client = new(std::nothrow) Client( fd, false, 1);
	if( NULL != link_video_client )
	{
		log.info("connect video svr fd[%d].\n", fd );
		fd_client_map[link_video_client->fd] = link_video_client;
	}
	else
	{
		close(fd);
		log.error("new video svr client error.\n");
		//exit(1);
	}
	return 0;
}

int Game::reconnect_videosvr()
{
	struct sockaddr_in addr;
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		log.error("File[%s] Line[%d]: socket failed: %s\n", __FILE__, __LINE__, strerror(errno));
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pdk.conf["video-svr"]["port"].asInt());
	addr.sin_addr.s_addr = inet_addr(pdk.conf["video-svr"]["host"].asString().c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		log.error("game::reconnect_videosvr Incorrect ip address!");
		close(fd);
		fd = -1;
		return -1;
	}
	if( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(fd);
		log.error("reconnect videosvr error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	link_video_client = new (std::nothrow) Client( fd, false, 1);
	if( link_video_client )
	{
		log.info("reconnect videosvr link fd[%d].\n", fd );
		fd_client_map[link_video_client->fd] = link_video_client;
		ev_timer_stop(pdk.loop, &ev_reconnect_videosvr_timer );
	}
	return 0;
}

void Game::send_to_videosvr(const std::string &res)
{
	if( NULL != link_video_client )
	{
		link_video_client->send( res );
		log.info("send to videosvr... \n");
	}
	else
	{
		log.error("link_data client is NULL, send to videosvr failed \n");
	}
}

void Game::start_videoreconnect()
{
	ev_timer_again( pdk.loop, &ev_reconnect_videosvr_timer );
}
void Game::reconnect_videosvr_cb( struct ev_loop *loop, struct ev_timer *w, int revents )
{
	Game *p_game = (Game*)w->data;
	p_game->reconnect_videosvr();
}