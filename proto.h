#ifndef __PROTO_H__
#define __PROTO_H__

enum system_command
{
	SYS_ECHO						= 0001,       /* echo */
	SYS_ONLINE					= 0002,       /* get online */
	SYS_SETUP                = 0003,       /* setup table */

	SERVER_REGISTER_REQ          = 7000, //���Ʒ�����ע��
	LINK_DB_CLIENT_BREAK         = 7001, //�������������ݿ��������Ͽ�����

	SERVER_CONFIGURE_REQ         = 7002, //������ȡ���Ʒ�������ǰ����
	SERVER_CONFIGURE_RES         = 7003, //�������Ʒ�������ǰ����
	UPDATE_SERVER_CONFIGURE_REQ  = 7004, //�����������Ʒ���������
	UPDATE_SERVER_CONFIGURE_RES  = 7005, //���ظ������Ʒ�������������

	SERVER_UPDATE_MONEY          = 7006, //�����Ʒ�������������ͬ�����ҽ��ҵ����Ʒ�����
	SERVER_UPDATE_USER_INFO       = 7007, //���Ʒ��������������������ݿ��������������������Ʒ������ڴ�����
	SERVER_UPDATE_DB_TASK_REQ       = 7008, //���Ʒ��������������������ݿ���������
	SERVER_UPDATE_DB_TALLY_REQ      = 7009, //���Ʒ���������������ˮ��Ϣ�����ݿ�
	SERVER_SPEAK_INFO_WRITE_REQ     = 7010, //���Ʒ������������´�������Ϣ�����ݿ�
	SERVER_USER_BROKE_WRITE_REQ     = 7011, //���Ʒ������������������Ʋ���Ϣ�����ݿ�
	SERVER_USER_ZID_INFO_WRITE_REQ  = 7012, //���Ʒ����������������ҵ�ǰ���ε�vid��zid��Ϣ
	SERVER_ADD_ROBOT_MONEY          = 7104, //���Ʒ����������ӻ����˽���

	SERVER_INFORM_USER_VID_TO_DATASV = 7105, //���Ʒ�����֪ͨdatasvr���ҳɹ������ó�

	SERVER_SET_BANKER_TAG_TO_DB    = 7106,  //���Ʒ����������洢��ׯ��־
	SERVER_SAVE_POOL_TO_DB         = 7107, //���Ʒ����������洢��������
	SERVER_GET_POOL_FROM_DB        = 7108, //���Ʒ�����������ǰ���ӽ�������
	SERVER_GET_POOL_BACK           = 7109, //���ݿⷵ�ؽ������ݸ����Ʒ�����
	SERVER_SAVE_USER_OTHER_INFO    = 7110, //���Ʒ������洢�����������ص�����
	SERVER_SAVE_GAME_INFO          = 7111, //���Ʒ������洢������Ϸ��������
	SERVER_GET_GAME_INFO           = 7112, //���Ʒ�����������ȡ������Ϸ��������
	SERVER_GET_GAME_INFO_BACK      = 7113, //������Ϸ���ݸ����Ʒ�����
	SERVER_LAOHUJI_INFO            = 7114, //���Ʒ����������ϻ�����Ϣ����ˮDB�������ȷ�����
	SERVER_INCR_USER_TIPS		   = 7115, //���Ʒ�����������������������ʾ����

	SERVER_RECONNECT_REQ		   = 7201, //ƽ̨����ͳ�ƿͻ�������
	SERVER_NO_PLAYERS			   = 7217,//房间没有玩家
	SERVER_DEL_TABLE_RES		   = 7218,//回应平台删除房间
	SERVER_DEL_TABLE_REQ		   = 7219,//平台要求删除房间请求
};

enum client_command
{
	CLIENT_LOGIN_REQ				= 1001,     /* ��¼ */
	CLIENT_LOGOUT_REQ				= 1002,		/* �˳� */
	CLIENT_READY_REQ				= 1003,	    /* ׼�� */
	CLIENT_BET_REQ		 			= 1004,		/* ���� */
	CLIENT_CHAT_REQ					= 1005,
	CLIENT_FACE_REQ					= 1006,
	CLIENT_CHANGE_REQ            = 1007,		/* ���� */
	CLIENT_ROBOT_REQ             = 1008,		/* �й� */
	CLIENT_INFO_REQ              = 1009,		/* ������Ϣ */
	CLIENT_TABLE_INFO_REQ        = 1010,		/* ������Ϣ */
	CLIENT_EMOTION_REQ           = 1011,      	/* �������� */
	CLIENT_PROP_REQ              = 1012,      	/* game prop */
	CLIENT_BOX_REQ               = 1013,
	CLIENT_ACTIVE_REQ            = 1014,
	CLIENT_BETTING_REQ           = 1015,		/* ��Χ��ע */
	CLIENT_UPTABLE_INFO_REQ      = 1016,		/* �����б� */
	CLIENT_UPTABLE_APPLY_REQ     = 1017,		/* �������� */
	CLIENT_DOWNTABLE_AHEAD_REQ   = 1018,
	CLIENT_DOWNTABLE_REQ         = 1019,		/* �������� */
	CLIENT_RETURN_REQ            = 1020,
	CLIENT_DAEMONIZE_REQ         = 1021,
	CLINET_UPBANKER_REQ          = 1022,
	CLINET_UPBANKER_INFO_REQ     = 1023,
	CLINET_DOWN_BANKER_REQ       = 1024,
	CLINET_HEART_BEAT_REQ        = 1025,		/* ���� */

	CLINET_TREND_REQ             = 1040,
	CLIENT_ALL_PLAYERS_REQ       = 1051,
	CLIENT_UP_GUEST_REQ          = 1052,
	CLIENT_DOWN_GUEST_REQ        = 1053,
	CLIENT_POOL_INFO_REQ         = 1054,
	CLINET_OTHER_SHOW_REQ        = 1056,
	CLIENT_CHOOSE_CARDS_REQ		 = 1057,
	CLIENT_TIP					 = 1059,
	CLIENT_CHOSE_BETS_NUM		 = 1060,
	CLIENT_DETAL_INFO_REQ        = 1070,

	CLIENT_RANK_REQ              = 1080,
	CLIENT_HISTORY_REQ           = 1090,

	CLIENT_PREPLAY_ONE_REQ		 = 1091,    /*�е���*/
	CLIENT_PREPLAY_TWO_REQ		 = 1092,	/*������*/
	CLIENT_PLAY_CARD_REQ 		 = 1093,	/*����*/
	CLIENT_WIN					 = 1094,
	CLIENT_PC					 = 1095,
	CLIENT_ROBOT_NEED_REQ		 = 1096,

	CLIENT_DZ_LOGIN_REQ			 = 1200,

	CLIENT_COMMON_OPT_REQ		 = 1210,
	CLIENT_SPLIT_CARD_REQ		 = 1211,
	CLIENT_SETTLEMENT_INFO_REQ	 = 1220,
	CLIENT_SETTLE_LIST_REQ		 = 1230,//结算列表请求
	CLIENT_CARD_LIST_REQ		 = 1240,//手牌详情请求
	CLIENT_DISSOLVE_ROOM_REQ 	 = 10000,
	CLIENT_DISSOLVE_ACTION_REQ   = 10001,
};

enum client_action
{
	PLAYER_CALL           = 2001,       /* call */
	PLAYER_RAISE          = 2002,       /* raise */
	PLAYER_COMPARE        = 2003,       /* compare */
	PLAYER_SEE            = 2004,		  /* see */
	PLAYER_FOLD           = 2005,       /* fold */
	PLAYER_ALLIN          = 2006,     /* all in */
	PLAYER_ALLIN_COMPARE  = 2007,     /* all in compare */
};

enum prop_item
{
	CHANGE_CARD = 3001,  /* change card */
	FORBIDDEN_CARD = 3002, /* forbidden compare card */
	DOUBLE_CARD_FOUR = 3003,    /* compare four multiple bet card */
	DOUBLE_CARD_SIX = 3004,    /* compare six multiple bet card */
	DOUBLE_CARD_EIGHT = 3005,    /* compare eight multiple bet card */
	NOSEE_CARD = 3006,    /* cannot see */
};

enum server_command
{
	SERVER_LOGIN_SUCC_UC       		 = 4000,
	SERVER_LOGIN_SUCC_BC       		 = 4001,
	SERVER_LOGIN_ERR_UC         	 = 4002,
	SERVER_REBIND_UC				 = 4003,
	SERVER_LOGOUT_SUCC_BC			 = 4004,
	SERVER_LOGOUT_ERR_UC			 = 4005,
	SERVER_TABLE_INFO_UC			 = 4006,	/* ������Ϣ */
	SERVER_READY_SUCC_BC			 = 4007,	/* ׼���ɹ� */
	SERVER_READY_ERR_UC				 = 4008,
	SERVER_GAME_START_BC			 = 4009,
	SERVER_NEXT_BET_BC				 = 4010, //��һ��ִ�ж���������
	SERVER_BET_SUCC_BC				 = 4011, //���Ҳ��������Ĺ㲥
	SERVER_BET_SUCC_UC				 = 4012,
	SERVER_BET_ERR_UC                = 4013,
	SERVER_GAME_END_BC				 = 4014, //��Ϸ����
	SERVER_GAME_PREREADY_BC          = 4015,
	SERVER_CHAT_BC					 = 4016,
	SERVER_FACE_BC					 = 4017,
	SERVER_ROBOT_SER_UC              = 4018,
	SERVER_ROBOT_NEED_UC             = 4019,
	SERVER_UPDATE_INFO_BC			 = 4020,
	SERVER_EMOTION_BC				 = 4021,
	SERVER_PROP_SUCC_UC              = 4022,
	SERVER_PROP_SUCC_BC              = 4023,
	SERVER_PROP_ERR_UC               = 4024,
	SERVER_BOX_SUCC_BC				 = 4025,
	SERVER_ACTIVE_SUCC_UC            = 4026,
	SERVER_BETTING_SUCC_UC			 = 4027,//Ѻע�ɹ�
	SERVER_ACTIVE_UPDATE_BC			 = 4028,//�Թ�Ѻע����
	SERVER_UPTABLE_INFO_UC			 = 4029,//�����б���Ϣ
	SERVER_APPLY_UPTABLE_UC			 = 4030,//���������ɹ�����
	SERVER_UPTABLE_SUCC_BC			 = 4031,//����������
	SERVER_AHEAD_DOWNTABLE_UC		 = 4032,//
	SERVER_DOWNTABLE_SUCC_BC		 = 4033,//����������
	SERVER_START_BET_BC				 = 4034,//Ѻע��ʼ
	SERVER_STOP_BET_BC				 = 4035,//ֹͣ��ΧѺע
	SERVER_WIN_UC					 = 4036,//Ѻע��������
	SERVER_BETTING_ERR_UC			 = 4037,//Ѻעʧ��
	SERVER_UPTABLE_ERR_UC			 = 4038,//����ʧ��
	SERVER_RETURN_UC                 = 4039,
	SERVER_REWARD_SUCC_UC            = 4040,
	SERVER_DAEMONIZE_SUCC_UC		 = 4041,
	SERVER_BANKER_CANDIDATE_UC       = 4042,
	SERVER_COMPARE_INFO_BC           = 4043,
	SERVER_DOWN_BANKER_UC            = 4044,
	SERVER_GIVE_UP_BANKER_BC         = 4045,
	SERVER_GET_BANKER_BC             = 4046,
	SERVER_SHORT_UPTABLE_INFO_BC     = 4049,

	SERVER_MULTI_BETS				 = 4060,//ѡ������
	SERVER_CHOSE_COMPLETE			 = 4065,//��������
	SERVER_COMPARE_CARDS_BC			 = 4066,//����
	SERVER_TIP						 = 4067,//û�õ�
	SERVER_SECOND_CARD				 = 4068,//�ڶ�������
	SERVER_THIRD_CARD				 = 4069,//����������
	SERVER_GAME_DARK_CARD			 = 4075,//��һ�ַ��İ���
	SERVER_START_COUNTDOWN			 = 4084,//��ͨ����
	SERVER_CHANGE_BANKER			 = 4085,//��ͨ����
	SERVER_BET_OVERSTEP_UC			 = 4086,
	SERVER_BETS_BC					 = 4087,

	SERVER_TREND_UC                  = 4100,
	SERVER_ALL_PLAYER_INFO_UC        = 4110,
	SERVER_UP_GUEST_SEAT_ERROR_UC    = 4111,
	SERVER_UP_GUEST_SEAT_SUC_BC      = 4112,
	//SERVER_DOWN_GUEST_SEAT_ERR_UC  = 4113,
	SERVER_DOWN_GUEST_SEAT_BC        = 4114,
	SERVER_POOL_INFO_UC              = 4115,

	SERVER_POOL_ALTER_BC             = 4116,
	SERVER_ROBOT_SER                 = 4117,
	SERVER_COMMUNITY_BC              = 4118,

	SERVER_TWO_COMMUNITY_UC          = 4119,
	SERVER_EMOTION_NOSEAT_BC         = 4120,
	SERVER_OTHER_SHOW_BC             = 4121,
	SERVER_BANNE_UC 			     = 4122,
	SERVER_GAME_RESULT_BC			 = 4123,
	SERVER_UPBANKER_ERR_UC			 = 4124,

	SERVER_DETAL_INFO_UC             = 4140,
	SERVER_DETAL_INFO_ERROR_UC       = 4141,

	SERVER_RANK_UC                   = 4160,
	SERVER_LOTTERY_SHOW_CARDS        = 4170,
	SERVER_HISTORY_UC                = 4171,
	SERVER_MULTI_OF_LOSE_BC          = 4210,
	SERVER_COPARE_RESULT_FORROBOT    = 4212,

	SERVER_PREPLAY_ONE_BC		 	 = 4214,		/* ��ʼ�е��� */
	SERVER_PREPLAY_ONE_SUCC_BC	 	 = 4215,		/* �е��������ɹ� */
	SERVER_PREPLAY_TWO_BC		 	 = 4216,		/* ������ */
	SERVER_PREPLAY_TWO_SUCC_BC	 	 = 4217,		/* �������ɹ� */
	SERVER_LANDLORD_BC 			 	 = 4218,		/* �����㲥 */
	SERVER_PLAY_CARD_BC				 = 4219,		/* ���� */
	SERVER_PLAY_CARD_SUCC_BC		 = 4220,		/* ���Ƴɹ� */
	SERVER_PLAY_CARD_ERR_BC		     = 4221,		/* ���ƴ��� */
	SERVER_ROBOT_BC					 = 4222,		/* �й� */
	SERVER_COMFIMATION_TASK_BC		 = 4223,		/* ȷ�����ƺ��������� */
	SERVER_TASK_SUCC_BC				 = 4224,		/* �����ƻ��������� */
	SEVER_BOOM_BC					 = 4225, 		/* ը���㲥 */
	SEVER_COUNT_CARD	             = 4226,		/* */
	SERVER_GAME_SINGLECARD			 = 4227,		/* �������ƹ㲥 */
	SERVER_PLAYER_WIN				 = 4228,
	SERVER_ROUND_END				 = 4229,
	SEVER_PC_BC						 = 4230,
	SERVER_GAME_START_UC			 = 4231,		/* ��Ϸ��ʼ���������� */
	SERVER_GAME_NOCALL				 = 4232,


	SERVER_DZ_LOGIN_SUCC_RES		= 4400, /* 对战模式登录成功返回 */
	SERVER_DZ_LOGIN_ERR_RES			= 4401, /* 对战模式登录失败返回 */

	SERVER_HEART_BEAT_RES			= 5014,

	SERVER_COMMON_OPT_START_BC		= 5100,
	SERVER_COMMON_OPT_SUCC_BC		= 5101,
	SERVER_COMMON_OPT_ERR			= 5102,

	SERVER_FIRST_PLAY_BC			= 5110,
	SERVER_MINGPAI_SUCC_BC			= 5111,
	SERVER_BOMB_BC					= 5112,
	SERVER_PASS_CARD_BC				= 5113,
	SERVER_SINGLE_CARD_UC			= 5114,
	SERVER_MINGPAI_PLAY_SUCC_BC		= 5115,
	SERVER_SPLIT_CARD_UC 			= 5116,
	SERVER_SPLIT_CARD_BC 			= 5117,
	SERVER_PLAYER_OFFLINE_BC		= 5130,
	SERVER_PLAYER_TIMEOUT_BC		= 5131,
	SERVER_GAME_SETTLEMENT_INFO		= 5140,
	SERVERT_SETTLE_LIST_ACK			= 5150,
	SERVERT_CARD_LIST_ACK			= 5160,

	/* 对战 */
	SERVER_DZ_GAME_START_REQ		= 7210, /* 对战模式玩牌服务器请求游戏开始 */
	SERVER_DZ_GAME_START_RES		= 7211, /* 对战模式玩牌服务器请求游戏开始返回 */
	SERVER_DZ_UPDATE_USER_INFO		= 7212, /* 对战模式玩牌服务器更新玩家个人信息 */
	SERVER_DZ_DISSOLVE_ROOM_REQ	 	= 7214, /* 对战模式玩牌服务器解散房间请求 */
	SERVER_DZ_DISSOLVE_ROOM_RES	    = 7215, /* 对战模式玩牌服务器解散房间请求 */
	SERVER_LOGOUT_ROOM_REQ			= 7220, /* 对战模式玩牌服务器退出房间请求 */
	SERVER_LOGOUT_ROOM_RES			= 7221, /* 对战模式玩牌服务器退出房间请求返回 */

	SERVER_APPLY_DISSOLVE_ROOM_ERR_UC 	= 15000,
	SERVER_APPLY_DISSOLVE_ROOM_SUCC_BC 	= 15001,
	SERVER_DISSOLVE_ACTION_SUCC_BC 		= 15002,
	SERVER_DISSOLVE_ROOM_RESULT_BC		= 15003,

	SERVER_VIDEO_DATA_SAVE        	 = 10002,
	SERVER_VIDEO_DATA_RES			 = 10003,
	SERVER_ROOM_CONFIG_REQ 			 = 10004,
	SERVER_ROOM_CONFIG_RES 			 = 10005,

	SERVER_REQUEST_RECOVER_DATA		 = 10010,
	SERVER_RECOVER_DATA_RES			 = 10011,
	SERVER_RECOVER_DATA_OVER		 = 10012,
};

#endif
