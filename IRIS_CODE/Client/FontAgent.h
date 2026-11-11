/* ====================================================================
 * 파    일 :	FontAgent.h
 * 목    적 :	생성된 폰트 객체의 관리 및 폰트 화면 출력 처리
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.10.17
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		06.10.17	이법석		생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

class cFontObject;

class cFontAgent
{
public:
	static cFontAgent*		mpFontAgent;

	cFontAgent();
	~cFontAgent();

	enum eFont
	{
		eFont_UI = 0,		/// UI용 폰트 ( UI용, 고정 SystemMsg )
		eFont_UI_Bold,
		eFont_System,		/// 시스템 폰트 (Tooltip, GameInfo, 유동SystemMsg)
		eFont_Chat,			/// 채팅용

		eFont_Notice,		/// 공지용(use back)
		eFont_DepthFont,	/// 깊이가 있는 면에 그려질 폰트용(use back)
		eFont_Npc,			/// NPC 대화창 폰트

		eFont_Max,
	};

	/// 초기화
	bool Init();

	/// 종료부
	void Exit();

	void BeginRender();
	void EndRender();

	/// 장치 소실시 처리부
	void Invalidate();
	void Restore();

	/// 같이 찍을수 있는 형태를 묶어서 랜더링 한다. (Font Keeper와 연동시키기 위한 함수)
	void AddText( unsigned short fontIdx, LPTSTR text, int textLen, POINT* textPos, unsigned long textColor );
	void BatchDrawText();

	/// DrawText 인터페이스 제공
	void DrawText( unsigned short fontIdx, LPTSTR text, int textLen, POINT* textPos, unsigned long textColor, bool outerLine = false );
	void DrawText( unsigned short fontIdx, LPTSTR text, int textLen, unsigned int posX, unsigned int posY, unsigned long textColor, bool outerLine = false );

	/// 문자열 길이 정보
	long GetTextExtent( unsigned short fontIdx, LPCTSTR text, unsigned int textLen );
	long GetTextWidth( unsigned short fontIdx );
	long GetTextHeight( unsigned short fontIdx );

protected:
	/// FontObject 리스트
//	typedef tArray<cFontObject*>		cFontArray;
	typedef tArray<void*> cFontArray;
	cFontArray	mFontArray;

	LPD3DXSPRITE        mpSprite;
	LPD3DXSPRITE        mpSpriteTemp;

	HANDLE				mhFont;
};

#define FONTAGENT	cFontAgent::mpFontAgent
