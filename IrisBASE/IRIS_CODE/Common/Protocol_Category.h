#pragma once

enum NM_CATEGORY	/// NM = Network Message
{
	NM_ERROR = 0,
	NM_PACKEDDATA,					/// Network Packeddata
	NM_USER,						/// Dist & Agent
	NM_PLAYER,						/// Player
	NM_MONSTER,						/// Monster
	NM_NPC,							/// NPC
	NM_ITEM,						/// Item
	NM_CHAT,						/// Chatting
	NM_SKILL,						/// Skill
	NM_TAROT,						/// Tarot Card
	NM_CHEAT,						/// 치트
	NM_PARTY,						/// Party
	NM_QUEST,						/// Quest
	NM_DUEL,						/// duel
	NM_SERVERTOOL,					/// Server Tool
	NM_PVP,
	NM_TOTEM,
	NM_GUILD,						/// guild
	NM_GATHERING,
	NM_MAKESKILL,
	NM_THEME,
	NM_PARTYUNION,					/// party union
	NM_OBSTRUCTION,
	NM_USERPORTAL,
	NM_TUTORIAL,					/// 튜토리얼
	NM_NOTICE,						/// notice
	NM_TELEPORTPORTAL,
	NM_HACKSHIELD,					/// 핵쉴드
	NM_TWITTER,						/// 트위터
	NM_TRADE,						/// 유저간 거래 관련
	NM_PLAYEREXT,                   /// NM_PLAYER 시즌 2
	NM_MAX
};
