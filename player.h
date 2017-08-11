#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ev++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include <ev.h>
#include "GameInfo.pb.h"
#include "CommonInfo.pb.h"
class Client;

typedef struct
{
	int			max_score;//最高得分
	int			bomb_cnt;//打出的炸弹数
	int			total_win;//总胜利局数
	int			total_lose;//总失败局数
	int			total_score;//总得分
	void clear(void)
	{
		max_score = 0;
		bomb_cnt = 0;
		total_win = 0;
		total_lose = 0;
		total_score = 0;
	}
	void update_max_score(int score){if(score > max_score) max_score = score;}
	void update_bomb_cnt(int cnt = 1){bomb_cnt += cnt;}
	void update_win_lose(int type){if(type == 0) total_lose++;else if(type == 1) total_win++;}
	void update_total_score(int score){total_score += score;}
}Settlement;

struct CardList
{
	int index;
	vector<int> cards;
};

struct SettleList
{
	int cur_money;
	int bomb_money;
};

class Player
{
	public:
		// table router information
		int					vid;
		int					zid;
		std::string			tid;
		int					seatid;
		int					is_offline;
		// player information
		int                 uid;
		std::string			avatar;
		std::string			skey;
		std::string			name;
		int					rmb;
		int					sex;
		int					money;
		int                 usertype;

		Client              *client;
		int					login_type;
		int					ready;
		int					cur_money;
		int					cur_coin;
		int 				logout_type;
		int             	down_tag;   // game end down table
		int             	play_num;   // player number
		int					robot;

		bool				daemonize_flag;
		std::map<int, int> 	com_opt;//玩家通用操作
		int					ratio;
		int					dissolve_cnt;
		int					dissovle_state;//0--反对解散 1--同意解散 2--未操作
		Settlement			settlement;//玩家结算信息
		map<int, SettleList>		settle_list;//每局输赢
		map<int, vector<CardList> >	card_list;//手牌详情

	private:
		ev_timer			_offline_timer;
		ev_tstamp			_offline_timeout;

		ev_timer			_del_player_timer;
		ev_tstamp			_del_player_tstamp;

	public:
		Player();
		virtual ~Player();
		void set_client(Client *c);
		int init( Json::Value &val );
		void reset();
		void clear();
		int update_info();
		void update_info( long alter_money, int alter_total_win);
		void start_offline_timer();
		void stop_offline_timer();
		void start_del_player_timer();
		void stop_del_player_timer();
		static void offline_timeout(struct ev_loop *loop, ev_timer *w, int revents);
		static void del_player_timer_cb(struct ev_loop *loop, ev_timer *w, int revents);


		//数据恢复
		int recover_data(const PGame::UserInfo &pUserInfo);

	private:

};

#endif
