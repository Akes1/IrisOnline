/* ====================================================================
 * 颇    老 :	serverProtocol.h
 * 格    利 :	
 * 累 己 磊 :	胞枚吝
 * 累 己 老 :	06/12/26
 * 林狼荤亲 :	
 * =================================================================== */
#ifndef __SERVERL_PROTOCOL_H__
#define __SERVERL_PROTOCOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

// 扩档快令 庆歹
#include <windows.h>
#include "msgbuf.h"

// IRIS Login Srv Ports
#define T_LOGIN_CPORT			14400

// IRIS Game Srv Ports
#define T_GAME_CPORT			14600

// IRIS Server & Channel
#define MAX_SERVER		22				// 弥措 辑滚荐
#define MAX_CHANNEL		42				// 弥措 盲澄荐(1~10老馆,11~20傈厘,21~42抛付)

#define MAKECID(S,C) (((S<<16)&0xffff0000)|(C&0x0000ffff))

#define STATUS_SYNC_INIT		0		// 00檬
#define STATUS_SYNC_TIME		20000	// 20檬
#define STATUS_SYNC_WAIT		60000	// 60檬

#define SERVER_CCU_S1			0		// 盔劝
#define SERVER_CCU_S2			1500	// 剧龋
#define SERVER_CCU_S3			2300	// 去棱

#define CHANNEL_CCU_S1			0		// 盔劝
#define CHANNEL_CCU_S2			300		// 剧龋
#define CHANNEL_CCU_S3			700		// 去棱
#define CHANNEL_CCU_S4			1200	// 父盔

// 惑怕 (0霖厚/1盔劝/2焊烹/3去棱)
enum SERVER_STATUS
{
	_E_STATUS_CLOSED_ = 0,				// 殿废等 盲澄捞 绝绰 惑怕
	_E_STATUS_RUNABLE_,					// 累诀 荐青措扁
	_E_STATUS_RUNNING_S1_,				// 累诀 荐青窍绊 乐澜 - 老馆 荤侩磊 力茄 秦瘤 (盔劝)
	_E_STATUS_RUNNING_S2_,				// 累诀 荐青窍绊 乐澜 - 老馆 荤侩磊 力茄 秦瘤 (焊烹)
	_E_STATUS_RUNNING_S3_,				// 累诀 荐青窍绊 乐澜 - 老馆 荤侩磊 力茄 秦瘤 (去棱)
	_E_STATUS_RUNNING_S4_,				// 累诀 荐青窍绊 乐澜 - 老馆 荤侩磊 力茄 秦瘤 (父盔)
	_E_STATUS_CLOSING_,					// 盲澄 摧扁吝

	_E_STATUS_FIN_WAIT1_,				// 付瘤阜 措扁 1
	_E_STATUS_FIN_WAIT2_,				// 付瘤阜 措扁 2
	_E_STATUS_LAST_ACK_,				// 弥辆 犬牢
	_E_STATUS_DELETE_,					// 碍力昏力
	_E_STATUS_ERROR_,					// 坷幅惯积
};

enum PVP_STATUS
{
	_E_PVP_CLOSED_ = 0,					// 辆丰(肯丰)
	_E_PVP_NOTICE_,						// 傍瘤 惯价
	_E_PVP_READY_,						// 累诀 荐青措扁
	_E_PVP_PLAYERLISTWAIT_,				// 曼咯磊 格废 措扁
	_E_PVP_RUNNING_,					// 累诀 荐青窍绊 乐澜
	_E_PVP_CLOSING_,					// 辆丰吝
};

enum THEME_STATUS
{
	_E_THEME_CLOSED_ = 0,				// 辆丰(肯丰)
	_E_THEME_READY_,					// 措扁吝
	_E_THEME_RUNNING_,					// 累诀 荐青窍绊 乐澜
};

enum SERVER_PROTOCOL
{
	// 盲澄
	MB_CH_SYNC_SYN = 0,
	MB_CH_LIST_SYN,
	MB_CH_STATUS_SYN,

	// 傍瘤
	MB_SERVER_NOTICE_SYN,
	MB_SERVER_DOWN_COUNT_NOTICE_SYN,

	// 盲泼
	MB_CHAT_MEGAPHONE_SYN,
	MB_CHAT_SYN,

	// PVP
	MB_PVP_STATUS_SYN,
	MB_PVP_NOTICE_SYN,
	MB_PVP_PLAYERLIST_SYN,
	MB_PVP_GMCHANNELMOVE_SYN,

	// 模备
	MB_FRIEND_SYN,

	// 辨靛
	MB_GUILD_SYNC_SYN,

	// 阁胶磐
	MB_MONSTER_SYN_SYN,

	// 抛付牢带
	MB_THEME_STATUS_SYN,
	
	// 快祈
	MB_POST_RECEIVED_SYN,

	// 颇萍
	MB_PARTY_SYN,

    // 碍拳
    MB_ENHANCED_RESULT_SYN,

	// 盲泼 陛瘤
	MB_NO_CHAT_SYN,

	// 蜡聪农酒捞袍 嚼垫
	MB_UNIQUEITEM_GET_SYN,

	// GM 捞亥飘 昏力
	MB_ADD_GM_EVENT_SYN,

	// GM 捞亥飘 昏力
	MB_DEL_GM_EVENT_SYN,

	// GM 碍力 辆丰
	MB_GM_KICK_EVENT_SYN,

	MB_ITEM_DROPLIMIT_UPDATE,
};

// 1Byte 沥纺 汲沥
#pragma pack(1)

// SERVER_PROTOCOL::MB_CH_SYNC_SYN
struct MB_SYN_CH_SYNC : MSGBUF {};

// SERVER_PROTOCOL::MB_CH_LIST_SYN
struct MB_SYN_CH_LIST : MSGBUF
{
	long rowCount;
	struct
	{
		long cid;							// Connection Index (Server & Channel Number)
		BYTE status;						// SERVER_STATUS 曼炼
	} channels[1];
	long Length ( ) { return (sizeof(MB_SYN_CH_LIST)-sizeof(channels)) + (sizeof(channels)*rowCount); }
};

// SERVER_PROTOCOL::MB_CH_STATUS_SYN
struct MB_SYN_CH_STATUS : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	BYTE status;							// SERVER_STATUS 曼炼
};

// SERVER_PROTOCOL::MB_SERVER_NOTICE_SYN
// defined commondefines.h file
#define MAX_SERVER_NOTICE_LEN	127
#define MAX_SERVER_NOTICE_BUF	MAX_SERVER_NOTICE_LEN+1

struct MB_SYN_SERVER_NOTICE : MSGBUF
{
	long    cid;							// Connection Index (Server & Channel Number)
	wchar_t message[MAX_SERVER_NOTICE_BUF];	// 皋技瘤
};

// SERVER_PROTOCOL::MB_SERVER_DOWN_COUNT_NOTICE_SYN
// defined commondefines.h file
struct MB_SYN_SERVER_DOWN_COUNT_NOTICE : MSGBUF
{
	long			 cid;							// Connection Index (Server & Channel Number)
	unsigned short   count;							// 皋技瘤
};

// SERVER_PROTOCOL::MB_CHAT_MEGAPHONE_SYN
// defined commondefines.h file
#define MAX_NAME_LEN	16
#define MAX_NAME_BUF	MAX_NAME_LEN+1
#define MAX_CHAT_LEN	127
#define MAX_CHAT_BUF	MAX_CHAT_LEN+1

struct MB_SYN_CHAT_MEGAPHONE : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_PVP_STATUS_SYN
struct MB_SYN_PVP_STATUS : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	BYTE status;							// PVP_STATUS 曼炼
	unsigned short pvpType;				
	unsigned long aryPos;
};

// SERVER_PROTOCOL::MB_PVP_NOTICE_SYN
struct MB_SYN_PVP_NOTICE : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	unsigned char leftMinute;				// 巢篮矫埃(盒)
	unsigned char pvpType;					// ePVP_TYPE
	unsigned long aryPos;
};

struct MB_SYN_PVP_PLAYERLIST : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// MSG_SYN_POSTPVP_PLAYERLIST 曼炼
};

struct MB_SYN_PVPGM_CHANNELMOVE : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	unsigned long playerIdx;
	unsigned short mapType;
		 
};

// SERVER_PROTOCOL::MB_THEME_STATUS_SYN
struct MB_SYN_THEME_STATUS : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	BYTE status;							// THEME_STATUS 曼炼
	unsigned short mapType;
};

// SERVER_PROTOCOL::MB_THEME_STATUS_SYN
struct MB_SYN_THEME_INVATE_RESPONE : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	long requestor;							// 夸没磊 idx
	wchar_t        name[MAX_NAME_BUF];		// 翠厘 某腐磐 捞抚
	char errorCode;							// 俊矾内靛 曼炼
};

// SERVER_PROTOCOL::MB_THEME_INVITE_SYN
struct MB_SYN_THEME_INVITE : MSGBUF
{
	long  cid;							// Connection Index (Server & Channel Number)
	short themeIdx;
	char  themeMode;
	short roomIdx;
	long  requestor;					// 檬措 夸没磊
	long  characterIdx;					// 檬措且 某腐磐 绊蜡锅龋
	char  msg[1];						// 皋矫瘤
};

// SERVER_PROTOCOL::MB_FRIEND_SYN
struct MB_SYN_FRIEND_SYN : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_GUILD_SYNC_SYN
struct MB_SYN_GUILD_SYNC : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_MONSTER_SYNC_SYN
struct MB_SYN_MONSTER_SYNC : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_CHAT_SYN
struct MB_SYN_CHAT_SYNC : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_POST_RECEIVED_SYN
struct MB_SYN_POST_RECEIVED : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	long characterIdx;						// 某腐磐 绊蜡锅龋
};

// SERVER_PROTOCOL::MB_PARTY_SYN
struct MB_SYN_PARTY : MSGBUF
{
	long cid;								// Connection Index (Server & Channel Number)
	char msg[1];							// 皋矫瘤
};

// SERVER_PROTOCOL::MB_ENHANCED_RESULT_SYN
struct MB_SYN_ENHANCED_RESULT : MSGBUF
{
	long        cid;						// Connection Index (Server & Channel Number)
	wchar_t     name[MAX_NAME_BUF];		    // 某腐磐 捞抚
    char        msg[1];                     // 牢亥配府
};

// SERVER_PROTOCOL::MB_NO_CHAT_SYN
struct MB_SYN_NO_CHAT : MSGBUF
{
	long        cid;						// Connection Index (Server & Channel Number)
	long		characterIdx;				// 某腐磐 绊蜡锅龋
	long		validThru;					// 盲泼 陛瘤 矫埃
};

// SERVER_PROTOCOL::MB_UNIQUEITEM_GET_SYN
struct MB_SYN_UNIQUEITEM_GET : MSGBUF
{
	long        cid;						// Connection Index (Server & Channel Number)
	wchar_t     name[MAX_NAME_BUF];		    // 某腐磐 捞抚
	char        msg[1];                     // 牢亥配府
};

// SERVER_PROTOCOL::MB_ADD_GM_EVENT_SYN
struct MB_SYN_ADD_GM_EVENT : MSGBUF
{
	long          cid;						// Connection Index (Server & Channel Number)
	unsigned long index;					// DB Index;
	unsigned long influenceIdx; 			// 瓤苞锅龋
	time_t		  startTime;				// 矫累矫埃
	time_t		  validThru;				// 蜡瓤父扁
};

// SERVER_PROTOCOL::MB_DEL_GM_EVENT_SYN
struct MB_SYN_DEL_GM_EVENT : MSGBUF
{
	long          cid;						// Connection Index (Server & Channel Number)
	unsigned long index;					// DB Index;
	unsigned long influenceIdx; 			// 瓤苞锅龋
};


// SERVER_PROTOCOL::MB_GM_KICK_EVENT_SYN
struct MB_SYN_GM_KICK_EVENT : MSGBUF
{
	long          cid;						// Connection Index (Server & Channel Number)
	long		characterIdx;				// 某腐磐 绊蜡锅龋
};

struct MB_SYN_DROPITEM_UPDATE : MSGBUF
{
	long				mCid;			// Connection Index (Server & Channel Number)
	unsigned long		mItemIdx;		// 酒捞袍 辆幅
	TIMESTAMP_STRUCT	mDropEndTime;	// 备埃 辆丰 朝楼
};

// 1Byte 沥纺 秦力
#pragma pack( )

#endif // __SERVERL_PROTOCOL_H__
