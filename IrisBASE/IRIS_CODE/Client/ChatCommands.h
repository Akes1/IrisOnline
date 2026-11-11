/* ====================================================================
 * 파    일 :	ChatCommands.h
 * 목    적 :	채팅창에 사용할 채팅 명령어를 처리한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	
 * =================================================================== */
#pragma once

class cChatCommands
{
public:
	cChatCommands();
	~cChatCommands();

	bool		MessageParser( LPCTSTR message );
	bool		MessageParserByType( unsigned char type, LPCTSTR message );

	/// 감정표현 관련데이터 삽입
	bool		AddCommunityData( LPCTSTR key, unsigned long index );

protected:
	/// 공백 문자열이거나 스페이스 입력만 되어잇는지 검사
	bool CheckChatMsg( LPCTSTR chatmsg, unsigned int len );

	/// 일반 메시지 보내기 
	void SendNormalMsg( LPCTSTR chatmsg );

	/// 귓말 보내기 
	void SendWhisperMsg( LPCTSTR toname, LPCTSTR chatmsg );

	/// 외치기 
	void SendShoutMsg( LPCTSTR chatmsg );

	/// 파티 메시지 보내기
	void SendPartyMsg( LPCTSTR chatmsg );

	/// 거래 메시지 보내기
	void SendTradeMsg( LPCTSTR chatmsg );

	/// 길드 메시지 보내기
	void SendGuildMsg( LPCTSTR chatmsg );

	/// GM 채팅 메시지 보내기
	void SendGMNormalMsg( LPCTSTR chatmsg );

	/// 전체외치기 보내기
	bool SendMegaphoneMsg( LPCTSTR chatmsg );

private:
	/// 감정 커맨드 관련 맵 
	typedef tHashMap<cStringT, unsigned long> cCommunityCommandMap;
	cCommunityCommandMap		mCommunityCommandMap;
};
