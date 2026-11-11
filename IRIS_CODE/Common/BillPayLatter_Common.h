#pragma once

#pragma pack(push,1)

#define SERVERNUM_UNIT		1000
#define SERVER_REFRESH		20000;

struct _PAYLETTER_HEADER
{
	unsigned short	mPacketType;		/// 전문 구분자
	unsigned short	mPacketSize;		/// 전문길이(바디부 포함)
	unsigned long	mSeqNo;				/// 전문 일련 번호
};

/// 시스템 에러 코드집
/*--------------------------------------------
51 : 입력값 오류
61 : DB 오류
69 : DB 기타 오류
71 : 요청 처리 불가(너무 많은 요청 대기중)
79 : GBA 처리 기타 오류
--------------------------------------------*/

/// BILL_PAYLETTER = BP
/// Game Billing Agent = GBA

/// 전문 구분자 정의 번호
///		서버 연결 요청
///		- 게임서버는 GBA에 연결 후 최초로 서버 연결 요청을 하여
///		- GBA가 게임서버의 연결을 인식 할 수 있도록 해야 한다.
#define BP_ServerConnect_Req		10
#define BP_ServerConnect_Res		11
///		인증 요청
///		- 사용자가 로그인시 PC방에서 접속한 사용자 인지 확인 요청 한다.
///		* PC방 정보 리턴(프리미엄/일반/PC방아님)
///		* 예기치 못한 재접속/재로그인이 발생시에도 인증 요청을 한다.
#define BP_UserAuthentication_Req	20
#define BP_UserAuthentication_Res	21
///		서버 상태 확인 요청
///		- 게임서버는 일정주기마다 GBA 서버가 정상 동작 중인지 확인 요청 한다.
#define BP_ServerAlive_Req			40
#define BP_ServerAlive_Res			41
///		유저 접속 상태 확인 요청
///		- 게임서버는 일정주기(10분)마다 PC방 접속 사용자에게 대해 접속 시간 관리가
///		  잘 되고 있는지 GBA로 확인 요청한다.
#define BP_UserAlive_Req			50
#define BP_UserAlive_Res			51
///		아이템 지급 알림
///		- GBA는 이벤트 아이템 지급이 발생하면 게임서버에 알리도록 한다.
///		  (아이템 지급방식은 캐쉬 아이템과 같이 아이템 연동DB에 이를 넣어두고 알림한다.)
#define BP_ItemGive_Syn				70
///		유저 인증 종료 요청
///		- PC방 접속 사용자가 게임을 로그아웃시에 요청 한다.
#define BP_UserOut_Req				80
#define BP_UserOut_Res				81
///		서버 종료 요청
///		- 게임 서버의 종료시 모든 사용자에 대한 로그아웃을 요청한다.
#define BP_ServerDisConnect_Req		90


/// BP_Req_ServerConnect
struct BP_REQ_CONNECT : public _PAYLETTER_HEADER
{
	unsigned short	mLoginServerNo;		/// 게임 서버 식별자
};
/// BP_Res_ServerConnect
struct BP_RES_CONNECT : public _PAYLETTER_HEADER
{
	/// 응답코드(0:접속허용, 1:접속거부, >50:처리실패)
	/// *50초과의 에러코드는 에러코드집 참조
	unsigned char	mResult;
};

/// BP_UserAuthentication_Req
struct BP_REQ_AUTHENTICATION : public _PAYLETTER_HEADER
{
	/// 중복 로그인시 기존 연결의 강제접속 해지 여부
	/// ( 0 : 접속해제하지 않음, 1 : 접속 해제 )
	unsigned char	mDisconnect;
	/// 사용자의 접속 IP주소
	unsigned long	mIPAddr;
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};
/// BP_UserAuthentication_Res
struct BP_RES_AUTHENTICATION : public _PAYLETTER_HEADER
{
	/// 응답 코드
	/// ( 0 : 게임가능, 1 : 중복로그인, 3 : blocked user, > 50 : failed )
	/// *50초과의 에러코드는 에러코드집 참조
	unsigned char	mResult;
	/// 접속 장소
	/// ( 0 : PC방아님, 1 : 프리미엄PC방, 2 : 일반PC방 )
	unsigned char	mUserStatus;
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};

/// BP_ServerAlive_Req
struct BP_REQ_SERVERALIVE : public _PAYLETTER_HEADER
{
	/// 게임서버 식별자
	unsigned short	mLoginServerNo;
};
/// BP_ServerAlive_Res
struct BP_RES_SERVERALIVE : public _PAYLETTER_HEADER
{
	/// 응답코드
	/// ( 0 : 조회성공, <> 0 : failed )
	unsigned char	mResult;
};

/// BP_UserAlive_Req
struct BP_REQ_USERALIVE : public _PAYLETTER_HEADER
{
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};
/// BP_UserAlive_Res
struct BP_RES_USERALIVE : public _PAYLETTER_HEADER
{
	/// 응답코드
	/// ( 0 : 정상관리중, 1 : PC방사용자아님 )
	unsigned char	mResult;
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};

/// BP_ItemGive_Syn
struct BP_SYN_ITEMGIVE : public _PAYLETTER_HEADER
{
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};

/// BP_UserOut_Req
struct BP_REQ_USEROUT : public _PAYLETTER_HEADER
{
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};
/// BP_UserOut_Res
struct BP_RES_USEROUT : public _PAYLETTER_HEADER
{
	/// 응답코드
	/// ( 0 : 성공, <> 0 : failed )
	/// *50초과의 에러코드는 에러코드집 참조
	unsigned char	mResult;
	/// 사용자 번호 - 계정idx
	unsigned long	mUserNo;
};

/// BP_ServerDisConnect_Req
struct BP_RES_SERVERDISCONNECT : public _PAYLETTER_HEADER
{
	/// 게임서버 식별자
	unsigned long	mLoginServerNo;
};

#pragma pack(pop)
