#ifndef _TABLE_H_
#define _TABLE_H_

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
#include <vector>

#include <json/json.h>

#include <map>
#include <set>

#include "deck.h"
#include "hole_cards.h"
#include "card_type.h"
#include "jpacket.h"
#include "card_statistics.h"
#include "card_analysis.h"

#include "GameInfo.pb.h"
#include "CommonInfo.pb.h"

//#define MAX_PLAYERS 2

#define MaxVideoBufLen 1024 * 10
#define MaxVideoUncompressLen 1024 * 50

using namespace std;

class Player;
class Client;

//座位类
typedef struct
{
	int 					seatid;
	int 					occupied; // 是否被坐下
	int 					uid; // user id
	Player 					*player;
	HoleCards 				hole_cards;
	std::vector<Card>  		last_play_cards;
	int 					cardno;//剩余牌数量
	int 					robot;//客户端托管
	int 					is_biggest;//报单是是否出的最大牌
	int 					card_type;   // random card type
	int 					is_single;	//报单
	int						cur_money;//当局输赢金币
	int						spring;//是否全关
	int						is_max_single;//是否出的最大单牌 用于判定放走包赔
	int						bomb_money;
	void clear(void)
	{
		occupied = 0; ////占用
		uid = 0;
		hole_cards.clear();
		last_play_cards.clear();
		card_type = 0;
		player = NULL;
		cardno = 16;
		is_single = 0;
		robot = 0;
		is_biggest = -1;
		spring = 0;
		cur_money = 0;
		is_max_single = 1;
		bomb_money = 0;
	}
	void reset(void)
	{
		hole_cards.clear();
		last_play_cards.clear();
		card_type = 0;
		cardno = 16;
		is_single = 0;
		robot = 0;
		is_biggest = -1;
		spring = 0;
		cur_money = 0;
		is_max_single = 1;
		bomb_money = 0;
	}
} Seat;

//玩牌状态
typedef enum
{
	READY = 0,
	MINGPAI_START_STATE,
	QIANGGUAN_STATE,
	BASE_DOUBLE_STATE,
	MINGPAI_PLAY_STATE,
	PLAYING,
	END_GAME,
	SPLIT_CARD,
} State;

typedef enum
{
	NEW_CARD = 0,
	FOLLOW_CARD,
	PASS_CARD,//要不起
	LAST_CARD,//最后一手
}Action;

typedef enum
{
	QIANG_GUAN 							= 1,		//抢关
	MINGPAI_START 						= 2,		//明牌发牌
	MINGPAI_PLAY 						= 3,		//明牌打牌
	ALLOW_FOUR_WITH_TWO 				= 4,		//允许四带二
	ALLOW_FOUR_WITH_THREE				= 5,		//允许四带三
	ALLOW_THREE_WITH_ONE_LAST			= 6,		//最后一手允许三带一出牌
	ALLOW_PLANE_WITH_LESS_LAST 			= 7,		//最后一手允许飞机带少出牌
	BASE_MONEY_DOUBLE 					= 8,		//底分加倍
	BOMB_CANNOT_SPLIT 					= 9,		//炸弹不可拆
	ALLOW_THREE_LINE 					= 10,		//允许出三顺
	ALLOW_THREE_WITH_ONE_LAST_JIE		= 11,		//最后一手允许三带一接牌
	ALLOW_PLANE_WITH_LESS_LAST_JIE		= 12,		//最后一手允许飞机带少接牌
	HEART_TEN							= 13,		//红桃10扎鸟
	THREE_K_IS_BOMB  					= 14,
	THREE_A_IS_BOMB 					= 15,
}PlayerOption;

class Table
{
public:
	//牌桌基本信息
	string						tid; //table id
	int             			vid; //玩法id
	int							zid; //端口id
	int							base_money;//底注
	int							stand_money;//上桌分数
	int							lose_exp;
	int							win_exp;
	int							bomb_base;//炸弹底分
	map<int, Player*>			players;//房间内所有玩家
	Seat                        seats[3];//房间座位
	int							table_type;//玩法类型 16--16张玩法 15--15张玩法
	int 						first_rule;//先出规则 0--首局黑桃3先出 第二局赢家先出 前提条件玩家不变 1--每局都是黑桃3先出
	set<int>					last_players;//上局玩家的uid 用于判断是否玩家不变(先出规则)
	int							last_card_rule;//剩一张规则 0--剩一张不输分 1--剩一张输分
	int							single_rule;//报单规则 0--放走包赔 1--报单顶大
	set<int>					player_option;//玩家选项
	int							qiangguan_ratio;//抢关倍数
	int							mingpai_play_ratio;//明牌打牌倍数
	int							mingpai_start_ratio;//明牌发牌倍数
	int							base_money_double_ratio;//底分加倍倍数
	int							spring_ratio;//春天倍数
	int							bomb_ratio;//炸弹倍数
	int							max_ratio;//封顶倍数
	int							bomb_can_split;//炸弹是否可拆 0--不可拆 1--可拆
	int							spade3_owner;//黑桃三拥有者
	//玩牌时的信息
	Deck 						deck;//牌库
	State						state; // game state
	int							start_seat;//每一轮首出牌玩家
	int							cur_seat;//当前出牌玩家
	int							last_seatid;//上一轮最大牌座位id
	int							win_seatid;//赢家座位号
	map<int, Player*>			seat_players;   // seat sit player
	vector<Card> 				play_cards; //当前出的牌
	vector<Card> 				hole_cards; //用于复制玩家手牌
	vector<Card> 				last_cards; //游戏中上一家出的牌
	int							card_type;//牌型
	Action						action;
	string						innings;
	int							dissovle_state;
	int							is_dissolved;
	int							room_state;//房间状态 0--未开始 1--正在进行
	int							owner_uid;//房主uid用于解散房间
	int							game_nums;
	set<int>					opt_seats;//公共操作座位号
	int							let_slip_seat;//放走包赔玩家
	int							takein;//初始携带
	int							ratio;
	map<int, Player*>			voters_players;//解散房间投票者
	int							dissolve_applyer;//解散房间申请者
	int							ready_players;
	int 						can_logout;
	int							split_pos;//切牌位置
	int							split_seat;//切牌座位
	int							split_ensure;//0--未选择是否切牌 1--不切牌 2--切牌 3--切牌中 4--确定切牌
	int							is_split;//0--没有切牌选项 1--有切牌选项
	int							spade3_first_out;//1--黑桃3必须首出 0--不必须
	string  					m_videostr;//录像数据
	int							banker;//本局庄家
	int							end_time;//结束时间
	int							MAX_PLAYERS;
	int							is_display_left;//是否显示剩余手牌数量 0--显示 1--不显示
	int							out_card_index;//第多少手
	bool						m_bAlreadyResp;
	int 						heart10_owner;//红桃10玩家
	map<int, int>				heart10_owner_map;//每局红桃十玩家
	int 						settle_type;	//结算方式
	bool 						k_is_bomb;
	bool 						a_is_bomb;

	//计时器
	ev_timer                    preready_timer;				//重新准备计时  相对事件处理
	ev_tstamp                   preready_timer_stamp;		//重新准备计时  1.5

	ev_timer					play_card_timer;			//出牌
	ev_tstamp					play_card_timer_tstamp;		//20

	ev_timer					robot_timer;				//托管
	ev_tstamp					robot_timer_tstamp;			//2

	ev_timer					pass_timer;					
	ev_tstamp					pass_timer_tstamp;			//0.5
	ev_tstamp					pass_timer_more_tstamp;		//1.0

	ev_timer					call_timer;	
	ev_tstamp					call_timer_stamp;			//1.5

	ev_timer					com_opt_timer;				//公共操作定时器
	ev_tstamp					com_opt_tstamp;

	ev_timer					dissolve_room_timer;		//解散房间倒计时
	ev_tstamp					dissolve_room_tstamp;		//180

	ev_timer					pre_pass_timer;
	ev_tstamp					pre_pass_tstamp;			//0.5

	ev_timer					video_timer;
	ev_tstamp					video_tstamp;				//0.5

	ev_timer                    game_restart_timer;
	ev_tstamp                   game_restart_timer_stamp; 	//3.0
public:
	Table();
	virtual ~Table();
	int init(Json::Value &val, Client *client);
	void init_member_variable();
	int broadcast(Player *player, const std::string &packet);
	int unicast(Player *player, const std::string &packet);
	int random(int start, int end, int seed);
	void reset();
	void vector_to_json_array(std::vector<Card> &cards, Jpacket &packet, string key);
	void map_to_json_array(std::map<int, Card> &cards, Jpacket &packet, string key);
	void json_array_to_vector(std::vector<Card> &cards, Jpacket &packet, string key);
	int sit_down(Player *player);
	void stand_up(Player *player);
	int del_player(Player *player);
	int handler_login(Player *player);
	int handler_login_succ_uc(Player *player);
	int handler_login_succ_bc(Player *player);
	int handler_ready(Player *player);
	int handler_preready();
	int handler_table_info(Player *player);
	//int handler_betting(Player *player);
	int handler_apply_uptable(Player *player);
	int handler_uptable(Player *player);
	void uptable_error_uc(Player *player, int code);
	int game_start_req();
	int handler_downtable(Player *player);
	int game_start_res( Jpacket &packet );
	int game_start();
	void start_play();
	int start_next_player();
	void handler_pass_card();
	void handler_put_card(int _type = 0);
	void card_type_count(int type);
	int handler_play_card(Player *player);
	int handler_robot(Player *player);
	void handler_robot_play(Player *player);
	void bomb_balance(Player *player);
	void count_money(Player *player);
	void update_running_tally( int uid, long alter_value, long current_value, int tally_type, int alter_type = 2 );
	void update_user_info_to_datasvr( std::map<int, int> &uid_win_lose, int tally_type = 0, int alter_type = 2 );
	int game_end();
	int handler_chat(Player *player);
	int handler_face(Player *player);
	int handler_interaction_emotion(Player *player);
	void handler_all_players_info( Player *player );
	void handler_detal_info( Player *player );
	int handler_logout(Player *player);
	int handler_info(Player *player);
	int next_seat(int pos);
	void map_to_json_array_spec(std::map<int, Card> &cards, Jpacket &packet, int index);

	static void preready_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void ready_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void play_card_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void robot_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void pass_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void call_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void dissolve_room_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void com_opt_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void split_card_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void pre_pass_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void video_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);
	static void game_restart_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);


	int split_holecards(HoleCards &holecards,vector<vector<Card> > &heap_group);
	bool remove_card(std::vector<Card> &cards,Card &card);
	bool remove_card_v(std::vector<Card> &cards,std::vector<Card> &heap_cards);
	void boom_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group);
	void three_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group);
	void line_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group);
	void two_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group);
	void one_heaps(CardStatistics &card_stat,std::vector<Card> &cards_temp,vector<vector<Card> > &heap_group);
	int single_conf(string conf_id, int &conf_value, bool user_input = false);
	int multi_conf(string conf_id, set<int> &conf_value);
	void check_dissovle_result();
	int handler_dissolve_action(Player *player);
	int handler_dissolve_room(Player *player);
	void handler_timeout(Player *player);
	void handler_offline(Player *player);
	int advance_game_end();
	int check_cards_error(HoleCards &h, vector<Card> &c);
	void safe_check_ratio();
	int handler_base_double(Player *player);
	int handler_mingpai_play(Player *player);
	int handler_qiangguan(Player *player);
	int handler_mingpai_start(Player *player);
	void common_operation_ack(Player *player, int opt_type);
	void common_operation_send(int opt_type);
	void common_operation_err(Player *player, int error_code);
	int handler_common_operation(Player *player);
	int test_game_start();
	void send_card();
	void qiangguan_fail();
	int check_card_type(int _card_type, bool _last_hand);
	int handler_split_card(Player *player);
	int handler_split_card_exec(int _ensure);
	void handler_settlement(Player* player);//总结算信息
	void handler_settle_list(Player *player);
	void handler_card_list(Player *player);


	void save_game_start_video_data();
	void send_video_to_svr();//发送录像
	void save_video_data(int cmd, std::string str);

	//数据恢复
	void set_to_json_array(set<int> &cards, Json::Value &val, string key);
	void map_to_json_array(map<int, Card> &cards, Json::Value &val, string key);
	void vector_to_json_array(vector<Card> &cards, Json::Value &val, string key);
	void json_array_to_map(map<int, Card> &cards, Json::Value &val, string key);
	void json_array_to_vector(vector<Card> &cards, Json::Value &val, string key);
	void json_array_to_set(set<int> &cards, Json::Value &val, string key);
	void recover_seat(int index, const PGame::SeatInfo &pSeatInfo);
	void recover_table(const PGame::TableInfo &pTableInfo);
	void recover_data(const PGame::RecoverData &pRecoverData);
	void decompress_str(string &src, string &des);
	void compress_str(string &src, string &des);
	void send_table_info_to_recoversvr();

};

#endif
