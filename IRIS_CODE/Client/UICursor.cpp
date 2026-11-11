#include "StdAfx.h" 
#include "UICursor.h"

#include "Application.h"
//#include "RenderSystem.h"

#include "StageManager.h"
#include "SkillExecuter.h"

#include "resource.h"

cUICursor* cUICursor::mpSingleton = 0;

cUICursor::cUICursor()
: mCusorAlpha(0.f)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	for( int i = 0; i < eCURSOR_MAX; ++i )
	{
		mCursor[i] = 0;
	}
}

cUICursor::~cUICursor()
{
	mpSingleton = 0;

}

/// 주의사항 :
bool cUICursor::Init()
{

		HINSTANCE hins = THEAPP->GetWindowInstance();

		mCursor[eCURSOR_DEFAULT] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR1 ) );
		mCursor[eCURSOR_ATTACK] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR2 ) );
		mCursor[eCURSOR_SKILL] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR3 ) );
		mCursor[eCURSOR_TALK] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR4 ) );
		mCursor[eCURSOR_MOVEMAP] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR5 ) );
		mCursor[eCURSOR_MOVESTOP] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR6 ) );
		mCursor[eCURSOR_DROPITEM] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR7 ) );
		mCursor[eCURSOR_GATHERING] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR8 ) );

		mCursor[eCURSOR_CLICK]	   = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR9  ) );
		mCursor[eCURSOR_USER]	   = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR10 ) );
		mCursor[eCURSOR_USER_SELL] = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR11 ) );
		mCursor[eCURSOR_TAROT]	   = LoadCursor( hins, MAKEINTRESOURCE( IDC_CURSOR12 ) );


/*
	char pathName[256] = {0,};
 	for( unsigned int i=0; i<eCURSOR_MAX; i++ )
 	{
		switch( i )
		{
		case eCURSOR_DEFAULT: 
			::sprintf( pathName, "%s", "./Data/2DData/default.cur" ); break;
		case eCURSOR_ATTACK: 
			::sprintf( pathName, "%s", "./Data/2DData/attack.cur" ); break;
		case eCURSOR_SKILL: 
			::sprintf( pathName, "%s", "./Data/2DData/skill.cur" ); break;
		case eCURSOR_TALK: 
			::sprintf( pathName, "%s", "./Data/2DData/talk.cur" ); break;
		case eCURSOR_MOVEMAP: 
			::sprintf( pathName, "%s", "./Data/2DData/movemap.cur" ); break;
		case eCURSOR_MOVESTOP: 
			::sprintf( pathName, "%s", "./Data/2DData/movestop.cur" ); break;
		case eCURSOR_DROPITEM: 
			::sprintf( pathName, "%s", "./Data/2DData/dropitem.cur" ); break;
		default:
			continue;
		}

		/// load in default folder
		mCursor[i] = LoadCursorFromFileA( pathName );
		if( mCursor[i] )
			continue;

		/// load in pack 
		cFileLoader loader;
		if( loader.Open( pathName, true ) == false )
		{
			assert( 0 );
			return false;
		}
		mCursor[i] = (HCURSOR)CreateIconFromResourceEx( (PBYTE)loader.GetBufferPtr(), loader.GetSize(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR );
		if( mCursor[i] == 0 )
		{
			assert(0);
			return false;
		}
	}
*/

	/// 초기커서 설정
	mCurCursorType = eCURSOR_DEFAULT;
	::SetCursor( mCursor[ mCurCursorType ]);

	return true;
}

/// 주의사항 :
void cUICursor::Process( float )
{
	if( mCurCursorType == eCURSOR_MAX )
		::SetCursor( NULL );
	else
		::SetCursor( mCursor[ mCurCursorType ]);

	/// SetClassLong는 RegisterClassEx함수에 의해서 등록된 특정 정보를 바꿀 때 사용하는 함수입니다. 
	::SetClassLong( THEAPP->GetHWND(),GCL_HCURSOR,(LONG)mCursor); 
}

/// 주의사항 :
void cUICursor::SetCursor( eCursor_Type eType )
{
	if( eType == eCURSOR_MAX  )
	{
		mCurCursorType = eCURSOR_MAX;
		::SetCursor( NULL );
		return;
	}

	eCursor_Type type = eType;

	/// 클릭 이미지 상태에서는 언클릭 이미지(=기본이미지)로만 변경이 가능하다.
	if( mCurCursorType == eCURSOR_CLICK )
	{
		if( type == eCURSOR_UNCLICK )
		{
			type = eCURSOR_DEFAULT;
		}
		else
			return;
	}
	else if( type == eCURSOR_CLICK ) 	/// 클릭 이미지는 기본상태에서만 변경 가능하다.
	{
		if( mCurCursorType == eCURSOR_DEFAULT )
		{
			type = eCURSOR_CLICK;
		}
		else
			return;
	}
	else if( type == eCURSOR_UNCLICK ) /// 그 외에 언클릭 이미지는 무시한다.( ex>타로 상태 -> 언클릭 이미지 )
	{
		return;
	}

	if( STAGEMAN->GetCurrentStage() == eStage_Game )
	{
		if( SKILLEXECUTER->IsFieldSkillCharge() == false )
		{
			if( SKILLEXECUTER->IsSkillCharge() == true )
				type = eCURSOR_SKILL;
		}
	}
	/// 동일하면 누적되어서 제대로 실행되지 않는 경우가
	/// 있으므로 리턴시킨다 
	if( mCurCursorType == type )
	{
		return;
	}

	mCurCursorType = type;

	::SetCursor( mCursor[ mCurCursorType ]);
}