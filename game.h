#ifndef _GAME_H_
#define _GAME_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ev++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "client.h"

class Client;
class Player;
class Table;

class Game
{
	public:
		std::map<std::string, Table*>       all_tables;
		std::map<std::string, Table*>       del_tables_map;
		std::map<int, Client*>      uid_client_map;
		std::map<int, Client*>      fd_client_map;
		Client *link_data_client;
		Client *link_video_client;
		std::string game_name;
		int game_number;
	private:
		ev_io _ev_accept;
		int _fd;

		ev_timer	ev_reconnect_datasvr_timer;
		ev_tstamp   ev_reconnect_datasvr_tstamp;

		ev_timer    ev_delall_client_timer;
		ev_tstamp   ev_delall_client_tstamp;

		ev_timer    ev_del_table_timer;
		ev_tstamp   ev_del_table_tstamp;
		int server_vid;
		int server_zid;

	public:
		Game();
		virtual ~Game();
		int start();
		int connect_datasvr();
		static void reconnect_datasvr_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
		static void del_table_cb( struct ev_loop *loop, struct ev_timer *w, int revents );
		void del_table();
		void start_reconnect();
		int reconnect_datasvr();
		void send_to_datasvr( const std::string &res );
		void del_all_client();

		int request_login( Client *client );
		int login_succ_back( Json::Value &val );
		int login_error_back(int uid, int code );
		static void accept_cb(struct ev_loop *loop, struct ev_io *w, int revents);
		void del_client(Client *client, bool del_oldclient=false);
		int dispatch(Client *client);
		int safe_check(Client *client, int cmd);
		int handler_login_table(Client *client, Json::Value &val);
		int login_table( Client *client, Json::Value &val);
		int login_table(Client *client, std::map<int, Table*> &a, std::map<int, Table*> &b);
		int del_player(Player *player);

		void game_start_res( Jpacket &packet );
		void del_all_play_client(std::string roomid);
		int handler_del_table(Json::Value &val);
		int table_no_players(std::string _roomid);
		//发送登出房间协议给客户端
		void SendLogout(Client *client, int uid, int code);
		void send_to_videosvr(const std::string &res);
		int dz_login_error_back( Client *client, int code);
		int dispatch2(Client *client);

	private:
		int init_table();
		int init_accept();
	private:
		ev_timer	ev_reconnect_videosvr_timer;
		ev_tstamp   ev_reconnect_videosvr_tstamp;
		int connect_videosvr();
		void start_videoreconnect();
		int reconnect_videosvr();
		static void reconnect_videosvr_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
};

#endif
