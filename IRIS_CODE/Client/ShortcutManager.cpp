#include "StdAfx.h"
#include "ShortcutManager.h"

#include "KeyboardAgent.h"

#include "Application.h"
#include "RenderSystem.h"
#include "UIManager.h"
#include "GameUIManager.h"

#include "ShortcutWindow.h"
#include "RowQuickSlot.h"
#include "ColumnQuickSlot.h"

#include "Parser.h"

#include "StageManager.h"
#include "GameResourceManager.h"

cShortcutManager* cShortcutManager::mpShortcutManager = NULL;
const char* const SCRIPT_PATH_FILE = "./script/interface/path_shortcut.txt";

cShortcutLexer::cShortcutLexer(const char *buffer, unsigned int size)
: cLexer( buffer, size )
{
	BindKeyword( "shortcut", eTOKEN_SHORTCUT );
	BindKeyword( "keyboard", eTOKEN_KEYBORAD );
}

cShortcutManager::cShortcutManager()
{
	mpShortcutManager = this;
}

cShortcutManager::~cShortcutManager()
{
	mpShortcutManager = 0;


	sKeyboardMap::cIterator pos = mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		SAFE_DELETE( pKeyInfo );
	}

	sKeyMap::cIterator pos2 = mKeyboardScriptMap.Begin();
	sKeyMap::cIterator end2 = mKeyboardScriptMap.End();
	for( ; pos2 != end2; ++pos2 )
	{
		sKeyboardScript* pKeyboardInfo = (sKeyboardScript*)(*pos2).mSecond;
		SAFE_DELETE( pKeyboardInfo );
	}

	sKeyMap::cIterator pos3 = mShortcutScriptMap.Begin();
	sKeyMap::cIterator end3 = mShortcutScriptMap.End();
	for( ; pos3 != end3; ++pos3 )
	{
		sShortcutScript* pShorcutInfo = (sShortcutScript*)(*pos3).mSecond;
		SAFE_DELETE( pShorcutInfo );
	}
}

bool cShortcutManager::ProcessKeyPressed( cKeyboardAgent* /*keyInfo*/ )
{
	/// stage에 상관 없는 공통 처리부분을 넣어라..
	return false;
}

bool cShortcutManager::ProcessKeyUpDown( cKeyboardAgent* keyInfo )
{
	/// stage에 상관 없는 공통 처리부분을 넣어라..
	/// 시스템관련 키 입력은 이곳에 넣을것..
	if( keyInfo->GetKeyPressed(KEY_CONTROL) )
	{
#if (defined(_DEVSYS) || defined(_DEBUG))
		/// 프로그램을 종료하지 않고 스킨만 재적용 가능하다.
		if( keyInfo->GetKeyDown( KEY_F5 ) )
		{
			/// 스킨 파일 로딩 후 재적용 
			UIMAN->ChangeSkin( "./Script/Interface/List_SkinImage.txt" );
			return true;
		}
#endif
		return false;
	}

	if( keyInfo->GetKeyDown( KEY_SCREENSHOT ) )
	{
		/// 스크린샷..
		THEAPP->SetSaveScreenShot( true );
		return true;
	}
	else if( keyInfo->GetKeyDown( KEY_SCROLL ) )
	{
		/// 게임 스테이지나 캐릭터 선택, 생성 중에만 사용가능
		if( RENDERSYS )
		{
			switch( STAGEMAN->GetCurrentStage() )
			{		
			case	eStage_Login :
			case	eStage_Select:
			case	eStage_Make  :
			case	eStage_Game  :
				if( RENDERSYS->IsVideoCapturing() )
					RENDERSYS->ToggleVideoCapture();
				else
				{
					/// Msgbox 출력
					UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
						eMBEVENT_VIDEO_CAPTURE, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 10003 ),
						GAMERESOURCEMAN->GetGameText( 2 ) // 알림
						);
				}
				break;
			default :
				///모든 모드에서 끄기 지원
				if( RENDERSYS->IsVideoCapturing() == true )
					RENDERSYS->ToggleVideoCapture();
				break;
			}
		}
		return true;
	}

	return false;
}

bool cShortcutManager::InitShortcut()
{
	sKeyMap::cIterator pos = mShortcutScriptMap.Begin();
	sKeyMap::cIterator end = mShortcutScriptMap.End();
	for( ; pos != end; ++pos )
	{
		int				 index = (*pos).mFirst;
		sShortcutScript* pShorcutInfo = (sShortcutScript*)(*pos).mSecond;
	
		if( pShorcutInfo )
			mShortCutKey.Insert( index, pShorcutInfo->mKeyboardIndex );
		else
		{
			assert(0);
			return false;
		}
	}

	if( mShortcutScriptMap.GetSize() > 0 )
		return true;
	else
		return false;
}

bool cShortcutManager::InitKeyboard()
{
	sKeyMap::cIterator pos = mKeyboardScriptMap.Begin();
	sKeyMap::cIterator end = mKeyboardScriptMap.End();
	for( ; pos != end; ++pos )
	{
		int				 index = (*pos).mFirst;
		sKeyboardScript* pKeyboardInfo = (sKeyboardScript*)(*pos).mSecond;


		if( pKeyboardInfo )
		{
			sKeyboardInfo* pKeyInfo = new sKeyboardInfo;
			pKeyInfo->mKey    = pKeyboardInfo->mKey;
			pKeyInfo->mMixKey = pKeyboardInfo->mMixKey;
			mKeyboardKey.Insert( index, pKeyInfo );
		}
		else
		{
			assert(0);
			return false;
		}
	}

	if( mKeyboardScriptMap.GetSize() > 0 )
		return true;
	else
		return false;
}

void cShortcutManager::ResetShortcut()
{
	/// 단축키 정보 초기화
	sShortCutMap::cIterator pos = mShortCutKey.Begin();
	sShortCutMap::cIterator end = mShortCutKey.End();
	for( ; pos!=end; ++pos )
	{
		int index = (*pos).mFirst;
		sShortcutScript* pShorcutInfo = (sShortcutScript*)mShortcutScriptMap.GetAt( index );
		if( pShorcutInfo )
		{
			(*pos).mSecond = pShorcutInfo->mKeyboardIndex;
		}
		else
			(*pos).mSecond = -1;
	}
}

void cShortcutManager::ResetKeyboard()
{
	sKeyMap::cIterator pos = mShortcutScriptMap.Begin();
	sKeyMap::cIterator end = mShortcutScriptMap.End();
	for( ; pos != end; ++pos )
	{
		int				 index = (*pos).mFirst;
		sShortcutScript* pShorcutInfo = (sShortcutScript*)(*pos).mSecond;

		if( pShorcutInfo )
		{
			sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( pShorcutInfo->mKeyboardIndex );
			if( pKeyInfo )
			{
				pKeyInfo->mShortcutIndex = index;
			}
		}
		else
			assert(0);
	}
}

void cShortcutManager::UpdateAllIconByKeyboard()
{
	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( !pShortcutWindow )
		return;

	cFSRowQuickSlot* fsRowQuickSlotWindow = GAMEUI->GetFSRowQuickSlot();
	cFSColumnQuickSlot* fsColumnQuickSlotWindow = GAMEUI->GetFSColumnQuickSlot();
	if( !fsRowQuickSlotWindow || !fsColumnQuickSlotWindow )
	{
		assert(0);
		return;
	}

	cRowQuickSlot*         rowQuickSlotWindow = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON );
	cRowQuickSlot*       rowQuickExtra1Window = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	cRowQuickSlot*       rowQuickExtra2Window = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON_EXTRA2 );
	cColumnQuickSlot*   columnQuickSlotWindow = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON );
	cColumnQuickSlot* columnQuickExtra1Window = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	cColumnQuickSlot* columnQuickExtra2Window = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON_EXTRA2 );

	if( !rowQuickSlotWindow || !rowQuickExtra1Window || !rowQuickExtra2Window || 
		!columnQuickSlotWindow || !columnQuickExtra1Window || !columnQuickExtra2Window )
	{
		assert(0);
		return;
	}

	/// 단축키 정보 초기화
	sShortCutMap::cIterator pos = mShortCutKey.Begin();
	sShortCutMap::cIterator end = mShortCutKey.End();
	for( ; pos!=end; ++pos )
	{
		(*pos).mSecond = -1;
	}

	/// 키보드에 있는 단축키 정보대로 단축 아이콘 정보를 셋팅한다.
	sKeyboardMap::cIterator pos2 = mKeyboardKey.Begin();
	sKeyboardMap::cIterator end2 = mKeyboardKey.End();
	for( ; pos2 != end2; ++pos2 )
	{
		// 키보드에 있는 단축키 정보대로 단축 아이콘 정보를 셋팅한다.
		int index = (*pos2).mFirst;
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos2).mSecond;
		if( pKeyInfo && pKeyInfo->mShortcutIndex != -1 )
		{
			mShortCutKey[pKeyInfo->mShortcutIndex] = index;
		}
	}

	/// 키보드에 그려진 아이콘을 그리고 단축아이콘들을 기본 위치에 셋팅한다.
	pShortcutWindow->ClearAllKeyboardIcon();
	pShortcutWindow->ClearAllShortcutIcon();

	sShortCutMap::cIterator pos3 = mShortCutKey.Begin();
	sShortCutMap::cIterator end3 = mShortCutKey.End();
	for( ; pos3!=end3; ++pos3 )
	{
		int index = (*pos3).mFirst;

		/// 퀵슬롯 상에 이미지 갱신
		if( eKEY_COMMON_QUICK1 <= index && index <= eKEY_COMMON_QUICK0 )
		{
			rowQuickSlotWindow->ChangeShortcut( index - eKEY_COMMON_QUICK1, mShortCutKey[index] );
			columnQuickSlotWindow->ChangeShortcut( index - eKEY_COMMON_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_QUICK1 <= index && index <= eKEY_QUICK0 )
		{
			fsRowQuickSlotWindow->ChangeShortcut( index - eKEY_QUICK1, mShortCutKey[index] );
			fsColumnQuickSlotWindow->ChangeShortcut( index - eKEY_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_EXTRA1_QUICK1 <= index && index <= eKEY_EXTRA1_QUICK0 )
		{
			rowQuickExtra1Window->ChangeShortcut( index - eKEY_EXTRA1_QUICK1, mShortCutKey[index] );
			columnQuickExtra1Window->ChangeShortcut( index - eKEY_EXTRA1_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_EXTRA2_QUICK1 <= index && index <= eKEY_EXTRA2_QUICK0 )
		{
			rowQuickExtra2Window->ChangeShortcut( index - eKEY_EXTRA2_QUICK1, mShortCutKey[index] );
			columnQuickExtra2Window->ChangeShortcut( index - eKEY_EXTRA2_QUICK1, mShortCutKey[index] );
		}

		/// 키를 셋팅하지 않은 아이콘은 기본 단축키 셋팅 위치에 표시한다.
		if( mShortCutKey[index] < 0 )
		{
			pShortcutWindow->SetShortcutIcon( index );
			continue;
		}

		pShortcutWindow->UpdateKeyboardIcon( mShortCutKey[index], index );		
	}
}

void cShortcutManager::UpdateAllIconByShortcut()
{
	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( !pShortcutWindow )
	{
		assert(0);
		return;
	}

	cFSRowQuickSlot* fsRowQuickSlotWindow = GAMEUI->GetFSRowQuickSlot();
	cFSColumnQuickSlot* fsColumnQuickSlotWindow = GAMEUI->GetFSColumnQuickSlot();
	if( !fsRowQuickSlotWindow || !fsColumnQuickSlotWindow )
	{
		assert(0);
		return;
	}

	cRowQuickSlot*         rowQuickSlotWindow = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON );
	cRowQuickSlot*       rowQuickExtra1Window = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	cRowQuickSlot*       rowQuickExtra2Window = GAMEUI->GetRowQuickSlot( QUICKTYPE_COMMON_EXTRA2 );
	cColumnQuickSlot*   columnQuickSlotWindow = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON );
	cColumnQuickSlot* columnQuickExtra1Window = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	cColumnQuickSlot* columnQuickExtra2Window = GAMEUI->GetColumnQuickSlot( QUICKTYPE_COMMON_EXTRA2 );

	if( !rowQuickSlotWindow || !rowQuickExtra1Window || !rowQuickExtra2Window || 
		!columnQuickSlotWindow || !columnQuickExtra1Window || !columnQuickExtra2Window )
	{
		assert(0);
		return;
	}


	/// 키보드에 있는 단축키 정보 초기화.
	sKeyboardMap::cIterator pos= mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		// 키보드에 있는 단축키 정보대로 단축 아이콘 정보를 셋팅한다.
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		if( pKeyInfo )
		{
			pKeyInfo->mShortcutIndex = -1;
		}
		else
		{
			assert(0);
		}
	}


	/// 단축키에 있는 정보대로 키보드에 단축 아이콘 셋팅
	sShortCutMap::cIterator pos2 = mShortCutKey.Begin();
	sShortCutMap::cIterator end2 = mShortCutKey.End();
	for( ; pos2!=end2; ++pos2 )
	{
		int index		= (*pos2).mFirst;
		int shortcutKey = (*pos2).mSecond;
		if( shortcutKey != -1 )
		{
			sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( shortcutKey );
			if( pKeyInfo )
			{
				pKeyInfo->mShortcutIndex = index;
			}
		}
	}

	/// 키보드에 그려진 아이콘을 그리고 단축아이콘들을 기본 위치에 셋팅한다.
	pShortcutWindow->ClearAllKeyboardIcon();
	pShortcutWindow->ClearAllShortcutIcon();

	sShortCutMap::cIterator pos3 = mShortCutKey.Begin();
	sShortCutMap::cIterator end3 = mShortCutKey.End();
	for( ; pos3!=end3; ++pos3 )
	{
		int index = (*pos3).mFirst;

		/// 퀵슬롯 상에 이미지 갱신
		if( eKEY_COMMON_QUICK1 <= index && index <= eKEY_COMMON_QUICK0 )
		{
			rowQuickSlotWindow->ChangeShortcut( index - eKEY_COMMON_QUICK1, mShortCutKey[index] );
			columnQuickSlotWindow->ChangeShortcut( index - eKEY_COMMON_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_QUICK1 <= index && index <= eKEY_QUICK0 )
		{
			fsRowQuickSlotWindow->ChangeShortcut( index - eKEY_QUICK1, mShortCutKey[index] );
			fsColumnQuickSlotWindow->ChangeShortcut( index - eKEY_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_EXTRA1_QUICK1 <= index && index <= eKEY_EXTRA1_QUICK0 )
		{
			rowQuickExtra1Window->ChangeShortcut( index - eKEY_EXTRA1_QUICK1, mShortCutKey[index] );
			columnQuickExtra1Window->ChangeShortcut( index - eKEY_EXTRA1_QUICK1, mShortCutKey[index] );
		}

		if( eKEY_EXTRA2_QUICK1 <= index && index <= eKEY_EXTRA2_QUICK0 )
		{
			rowQuickExtra2Window->ChangeShortcut( index - eKEY_EXTRA2_QUICK1, mShortCutKey[index] );
			columnQuickExtra2Window->ChangeShortcut( index - eKEY_EXTRA2_QUICK1, mShortCutKey[index] );
		}

		/// 키를 셋팅하지 않은 아이콘은 기본 단축키 셋팅 위치에 표시한다.
		if( mShortCutKey[index] < 0 )
		{
			pShortcutWindow->SetShortcutIcon( index );
			continue;
		}

		pShortcutWindow->UpdateKeyboardIcon( mShortCutKey[index], index );		
	}

}

void cShortcutManager::UpdateKeyboardIcon()
{
	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( !pShortcutWindow )
	{
		assert(0);
		return;
	}

	sKeyboardMap::cIterator pos= mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		int	index = (*pos).mFirst;
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		if( pKeyInfo && pKeyInfo->mShortcutIndex >= 0 )
		{
			pShortcutWindow->UpdateKeyboardIcon( index, pKeyInfo->mShortcutIndex );	 	
		}
	}
}

void cShortcutManager::SetShortcut( int fromShortcutIndex, int toKeyboardIndex )
{
	sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( toKeyboardIndex );
	if( pKeyInfo == 0 )
	{
		assert( 0 && "Wrong Keyboard Index" );
		return;
	}

	if( mShortCutKey.Find( fromShortcutIndex ) == mShortCutKey.End() )
	{
		assert( 0 && "Wrong Shortcut Index" );
		return;
	}

	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( !pShortcutWindow )
	{
		assert( 0 );
		return;
	}

	/// 기존에 들어있는 정보가 있다면 초기화 시키고 데이터 삽입.
	int toShortcutIdx = pKeyInfo->mShortcutIndex;
	if(	mShortCutKey.Find( toShortcutIdx ) != mShortCutKey.End() )
	{
		pShortcutWindow->SetShortcutIcon( toShortcutIdx );
		pKeyInfo->mShortcutIndex = fromShortcutIndex;
	}
	else
	{
		pKeyInfo->mShortcutIndex = fromShortcutIndex;
	}

	pShortcutWindow->UpdateKeyboardIcon( toKeyboardIndex, fromShortcutIndex );
	pShortcutWindow->UpdateSelect( toKeyboardIndex );
	
}

void cShortcutManager::MoveShortcut( int fromKeyboardIndex, int toKeyboardIndex )
{
	sKeyboardInfo* pFromKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( fromKeyboardIndex );
	if( pFromKeyInfo == 0 )
	{
		assert( 0 && "Wrong Keyboard Index" );
		return;
	}

	sKeyboardInfo* pToKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( toKeyboardIndex );
	if( pToKeyInfo == 0 )
	{
		assert( 0 && "Wrong Keyboard Index" );
		return;
	}

	int fromShortcutIdx;
	int   toShortcutIdx;

	fromShortcutIdx = pFromKeyInfo->mShortcutIndex;
	toShortcutIdx = pToKeyInfo->mShortcutIndex;

	/// 새로 이동할 곳에 이미 단축키가 설정되어 있어서 swap 해야 하는경우
	if(	mShortCutKey.Find( toShortcutIdx ) != mShortCutKey.End() )
	{
		/// 키보드의 단축키 인덱스 갱신
		pFromKeyInfo->mShortcutIndex = toShortcutIdx;
		pToKeyInfo->mShortcutIndex = fromShortcutIdx;
	}
	else if( mShortCutKey.Find( fromShortcutIdx ) != mShortCutKey.End() && pFromKeyInfo )
	{
		/// 새로 이동할 곳에 아무런 단축키가 설정 되어 있지 않는 경우.
		pToKeyInfo->mShortcutIndex = fromShortcutIdx;
		pFromKeyInfo->mShortcutIndex = -1;
	}

	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( pShortcutWindow )
	{
		pShortcutWindow->UpdateKeyboardIcon( fromKeyboardIndex,   toShortcutIdx );
		pShortcutWindow->UpdateKeyboardIcon(   toKeyboardIndex, fromShortcutIdx );
		pShortcutWindow->UpdateSelect( toKeyboardIndex );
	}
}

void cShortcutManager::EraseShortcut(int keyboardIndex )
{
	sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( keyboardIndex );
	if( pKeyInfo == 0 )
	{
		assert( 0 && "Wrong Keyboard Index" );
		return;
	}

	int shortcutIdx = pKeyInfo->mShortcutIndex;

	/// 키보드에서 단축키 정보 삭제
	pKeyInfo->mShortcutIndex = -1;

	/// 키보드 창에 아이콘을 삭제하고 단축키 창에서 아이콘을 표시한다
	cShortcutWindow* pShortcutWindow = GAMEUI->GetShortcutWindow();
	if( pShortcutWindow )
	{
		pShortcutWindow->ClearKeyboardIcon( keyboardIndex );
		pShortcutWindow->SetShortcutIcon( shortcutIdx );
		pShortcutWindow->HideSelectImage();
	}
}

bool cShortcutManager::IsSettingChanged()
{
	/// 키보드의 단축키 인덱스 검사
	sKeyboardMap::cIterator pos= mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		int	index = (*pos).mFirst;
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		if( pKeyInfo && pKeyInfo->mShortcutIndex >= 0 )
		{
			sShortCutMap::cIterator pos;
			pos = mShortCutKey.Find( pKeyInfo->mShortcutIndex );
			if( pos != mShortCutKey.End() )
			{
				int findKeyboardIndex = pos->mSecond;
				if( index != findKeyboardIndex )
					return true;
			}
		}
	}

	/// 단축키 인덱스 검사.
	sShortCutMap::cIterator pos2 = mShortCutKey.Begin();
	sShortCutMap::cIterator end2 = mShortCutKey.End();
	for( ; pos2!=end2; ++pos2 )
	{
		int index		= (*pos2).mFirst;
		int shortcutKey = (*pos2).mSecond;
		
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( shortcutKey );
		if( pKeyInfo )
		{
				if( index != pKeyInfo->mShortcutIndex )
					return true;
		}
	}

	return false;
}

void cShortcutManager::ClearKeyboard()
{
	sKeyboardMap::cIterator pos= mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		if( pKeyInfo )
			pKeyInfo->mShortcutIndex = -1;
		else
			assert(0);
	}
}

unsigned char cShortcutManager::GetShortcutKey( int index )
{
	sShortCutMap::cIterator pos;
	pos = mShortCutKey.Find( index );
	if( pos != mShortCutKey.End() )
	{
		int findKeyboardIndex = pos->mSecond;
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( findKeyboardIndex );
		if( pKeyInfo )
			return pKeyInfo->mKey;
	}
	return 0;
}

sKeyboardInfo* cShortcutManager::GetShortcutKeyInfo( int index )
{
	sShortCutMap::cIterator pos;
	pos = mShortCutKey.Find( index );
	if( pos != mShortCutKey.End() )
	{
		int findKeyboardIndex = pos->mSecond;
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( findKeyboardIndex );
		if( pKeyInfo )
			return pKeyInfo;
	}
	return 0;
}

void cShortcutManager::SetShortCutValue( int index, int value )
{
	sShortCutMap::cIterator pos;
	pos = mShortCutKey.Find( index );
	if( pos == mShortCutKey.End() )
	{
		//assert(0);  ///기존 단축키를 삭제할 경우 들어옵니다.
		return;
	}

	sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)mKeyboardKey.GetAt( value );
	if( pKeyInfo )
	{
		mShortCutKey[index] = value;
	}
	else
	{
		mShortCutKey[index] = -1;
		//assert(0); /// 기존 키보드에서 사용못하는 키를 추가했을 경우 들어옵니다.
	}

}

int cShortcutManager::GetShortCutValue( int index )
{
	sShortCutMap::cIterator pos;
	pos = mShortCutKey.Find( index );
	if( pos != mShortCutKey.End() )
	{
		return pos->mSecond;
	}
	else 
	{
		assert(0);
		return -1;
	}
}

bool cShortcutManager::Init()
{
	/// scirpt load
	if ( LoadShortcutPath( SCRIPT_PATH_FILE ) )
	{
		if( InitShortcut() == true && InitKeyboard() == true )
			return true;
	}

	return false;
}

bool cShortcutManager::LoadShortcutPath( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load path shortcut" );
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cShortcutLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case cShortcutLexer::eTOKEN_SHORTCUT:
			{
				if( parser.ExpectTokenString( "{" ) == false )
				{
					assert( 0 && "wrong script" );
					return false;
				}

				cToken token;
				cLexer* lexer = parser.GetLexer();

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
					{
						break;
					}
					
					int index = token.ToInt();
					sShortcutScript *pShortcutInfo = new sShortcutScript;
					if( pShortcutInfo )
					{
						pShortcutInfo->mPathIconIndex = parser.ParseInt();
						pShortcutInfo->mKeyboardIndex = parser.ParseInt();
						pShortcutInfo->mIconPosX = parser.ParseInt();
						pShortcutInfo->mIconPosY = parser.ParseInt();
						
						if( mShortcutScriptMap.Insert( index, pShortcutInfo ) == false )
						{
							assert(0);
							SAFE_DELETE( pShortcutInfo );
							return false;
						}
					}
				}
			}
			break;
		case cShortcutLexer::eTOKEN_KEYBORAD:
			{
				if( parser.ExpectTokenString( "{" ) == false )
				{
					assert( 0 && "wrong script" );
					return false;
				}

				cToken token;
				cLexer* lexer = parser.GetLexer();

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
					{
						break;
					}

					int index = token.ToInt();
					sKeyboardScript *pKeyboardInfo = new sKeyboardScript;
					if( pKeyboardInfo  )
					{
						pKeyboardInfo->mKey      = (unsigned char)parser.ParseInt();
						pKeyboardInfo->mMixKey   = (unsigned char)parser.ParseInt();
						pKeyboardInfo->mIconPosX = parser.ParseInt();
						pKeyboardInfo->mIconPosY = parser.ParseInt();
						
						if( mKeyboardScriptMap.Insert( index, pKeyboardInfo ) == false )
						{
							assert(0);
							SAFE_DELETE( pKeyboardInfo );
							return false;
						}
					}
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}
