#include "stdafx.h"
#include "ShortcutWindow.h"
#include "BaseIcon.h"

#include "GameResourceManager.h"
#include "DragWindow.h"
#include "ShortcutManager.h"

cShortcutWindow::cShortcutWindow()
:mSelectIcon( 0 )
,mpSelectImage( 0 )
,mShowSelect( false )
{

}

cShortcutWindow::~cShortcutWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cShortcutWindow::Open()
{

}

void cShortcutWindow::Close()
{
	mSelectIcon = 0;
	mShowSelect = false;
}

bool cShortcutWindow::OnCreate(cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 숏컷 메니져에서 아이콘 정보를 가져와서 셋팅
	typedef tPointerHashMap< int, void* > sKeyMap;
	sKeyMap *shortcutMap;
	sKeyMap *keyboardMap;

	shortcutMap = SHORTCUTMAN->GetShortcutScriptMap();
	keyboardMap = SHORTCUTMAN->GetKeyboardScriptMap();

	if( shortcutMap )
	{
		sKeyMap::cIterator pos = shortcutMap->Begin();
		sKeyMap::cIterator end = shortcutMap->End();

		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;
			sShortcutScript* pShortcutInfo = (sShortcutScript*)(*pos).mSecond;
	
			if( pShortcutInfo )
			{
				cBaseIcon *pIcon = new cBaseIcon;
				if( pIcon->CreateBySkinName( "Icon_Shortcut", this, index ) == false )
					return false;

				pIcon->SetIconType( eICON_SHORTCUT );
				pIcon->SetRelativePos( cUIPos(pShortcutInfo->mIconPosX, pShortcutInfo->mIconPosY) );
				mShortcutIcon.Insert( index, pIcon );

				cIconParam* pIconParam = UIMAN->GetIconParam( pShortcutInfo->mPathIconIndex );

				if ( pIconParam )
				{
					pIcon->ChangeImage( pIconParam );
				}
				else
				{
					assert(0);
					return false;
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
	}
	else
	{
		assert(0);
		return false;
	}

	if( keyboardMap )
	{
		sKeyMap::cIterator pos = keyboardMap->Begin();
		sKeyMap::cIterator end = keyboardMap->End();
		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;
			sKeyboardScript* pKeyboardInfo = (sKeyboardScript*)(*pos).mSecond;

			if( pKeyboardInfo )
			{
				cBaseIcon *pIcon = new cBaseIcon;
				if( pIcon->CreateBySkinName( "Icon_Keyboard", this, index ) == false )
					return false;

				pIcon->SetIconType( eICON_SHORTCUT );
				pIcon->SetRelativePos( cUIPos(pKeyboardInfo->mIconPosX, pKeyboardInfo->mIconPosY) );
				mKeyboardIcon.Insert( index, pIcon );
			}
			else
			{
				assert(0);
				return false;
			}
		}
	}
	else
	{
		assert(0);
		return false;
	}

	/// 선택 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "Shortcut_SelectImage" );
	if( selectSkin )
	{
		mpSelectImage = new cUIImage( selectSkin->mpTexture );
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int w = selectSkin->mSkinInfo->mWidth;
		unsigned int h = selectSkin->mSkinInfo->mHeight;
		unsigned int x = selectSkin->mSkinInfo->mX;
		unsigned int y = selectSkin->mSkinInfo->mY;
		mpSelectImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSelectImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	if( UpdateSpecialKeyRect() == false )
		return false;

	return true;
}

void cShortcutWindow::OnCommand( cUINode* pnode, unsigned int id )
{
	switch( id )
	{
		case eUIID_GAME_SHORTCUT_CANCEL:
			{

				if( SHORTCUTMAN->IsSettingChanged() == true )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_SHORTCUT_CANCEL, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 10202 ),
						GAMERESOURCEMAN->GetGameText( 10200 ) );
				}
				else
					Hide();
			}
			break;
		case eUIID_GAME_SHORTCUT_OK:
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									 eMBEVENT_SHORTCUT_OK, eSKIN_YESNO,
									 GAMERESOURCEMAN->GetGameText( 10201 ),
									 GAMERESOURCEMAN->GetGameText( 10200 ) );
			}
			break;
		case eUIID_GAME_SHORTCUT_ORIGINAL:
			{
				/// 키보드에 할당된 정보 및 아이콘 지우기.
				SHORTCUTMAN->ClearKeyboard();
				ClearAllKeyboardIcon();
				HideSelectImage();
				
				/// 키보드의 단축아이콘정보를 처음 셋팅으로 바꾸고 다시 그리기.
				SHORTCUTMAN->ResetKeyboard();
				
				SHORTCUTMAN->UpdateKeyboardIcon();

			}
			break;
		case eUIID_GAME_SHORTCUT_CLEAR:
			{
				/// 키보드에 할당된 정보 및 아이콘 지우기.
				SHORTCUTMAN->ClearKeyboard();
				ClearAllKeyboardIcon();
				HideSelectImage();
				/// 기본 스크립트에서 할당된 위치에 단축키 아이콘 그리기
				SetDefaultShortcutIcon();
			}
			break;
		default:
			{
				assert( 0 && "Bad id" );
			}
	}
}

void cShortcutWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	SHORTCUTMAN->UpdateAllIconByShortcut();
}

void cShortcutWindow::OnHide()
{
	mSelectIcon = 0;
	mShowSelect = false;
}

void cShortcutWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );
}

void cShortcutWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mSelectIcon && mpSelectImage )
	{
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
	}

	UpdateSpecialKeyRect();
	
	// 숏컷 메니져에서 아이콘 정보를 가져와서 셋팅
	typedef tPointerHashMap< int, void* > sKeyMap;
	sKeyMap *shortcutMap;
	sKeyMap *keyboardMap;

	shortcutMap = SHORTCUTMAN->GetShortcutScriptMap();
	keyboardMap = SHORTCUTMAN->GetKeyboardScriptMap();

	if( shortcutMap )
	{
		sKeyMap::cIterator pos = shortcutMap->Begin();
		sKeyMap::cIterator end = shortcutMap->End();

		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;
			sShortcutScript* pShortcutInfo = (sShortcutScript*)(*pos).mSecond;

			if( pShortcutInfo )
			{
				cBaseIcon *pIcon = (cBaseIcon*)mShortcutIcon.GetAt(index);
				if( pIcon )
					pIcon->SetRelativePos( cUIPos(pShortcutInfo->mIconPosX, pShortcutInfo->mIconPosY) );
			}
		}
	}
	
	if( keyboardMap )
	{
		sKeyMap::cIterator pos = keyboardMap->Begin();
		sKeyMap::cIterator end = keyboardMap->End();
		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;
			sKeyboardScript* pKeyboardInfo = (sKeyboardScript*)(*pos).mSecond;

			if( pKeyboardInfo )
			{
				cBaseIcon *pIcon = (cBaseIcon*)mKeyboardIcon.GetAt(index);
				if( pIcon )
					pIcon->SetRelativePos( cUIPos(pKeyboardInfo->mIconPosX, pKeyboardInfo->mIconPosY) );
			}
		}
	}
}

void cShortcutWindow::SetShortcutIcon( int shortcutIndex )
{
	cBaseIcon* pIcon = (cBaseIcon*)mShortcutIcon.GetAt( shortcutIndex );
	if( pIcon )
	{
		cIconParam* pIconParam = UIMAN->GetIconParam( SHORTCUT_ICON_BASE_INDEX + shortcutIndex );
		
		if ( pIconParam )
		{
			pIcon->ChangeImage( pIconParam );
		}
	}
	else
	{
		assert( 0 && "Wrong Shortcut Index" );
		return;
	}

}

void cShortcutWindow::UpdateKeyboardIcon( int keyboardIndex, int shortcutIndex )
{
	/// 키보드에 있는 이미지를 지워야 하는경우
	if( shortcutIndex == -1 )
	{
		//mKeyboardIcon[keyboardIndex]->ClearImage();
		cBaseIcon* pIcon = (cBaseIcon*)mKeyboardIcon.GetAt( keyboardIndex );
		if( pIcon )
			pIcon->ClearImage();
		return;
	}

	/// 키보드에 이미지를 세팅하는경우.
	cBaseIcon* pIcon = (cBaseIcon*)mShortcutIcon.GetAt( shortcutIndex );
	if( pIcon )
		pIcon->ClearImage();

	cIconParam* pIconParam = UIMAN->GetIconParam( SHORTCUT_ICON_BASE_INDEX + shortcutIndex );
	if ( pIconParam )
	{
		cBaseIcon* pIcon = (cBaseIcon*)mKeyboardIcon.GetAt( keyboardIndex );
		if( pIcon )
			pIcon->ChangeImage( pIconParam );
		else
		{
			assert( 0 && "Bad Keyboard Index" );
		}
	}
}

void cShortcutWindow::OnIconClicked( cUINode* caller, unsigned int )
{
	cBaseIcon* pIcon = (cBaseIcon*)caller;

	if( pIcon == 0 )
	{
		assert(0);
		return;
	}

	if( mpSelectImage ) 
	{
		/// 새 선택 아이콘 눌림효과
		cUIRect rc = pIcon ->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
		mSelectIcon = pIcon;
		mShowSelect = true;
	}
	else
		mShowSelect = false;
}

void cShortcutWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cBaseIcon* icon = (cBaseIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_SHORTCUTWINDOW, icon, pos, shift );
}

void cShortcutWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mShowSelect && mpSelectImage )
		mpSelectImage->Draw();
}

void cShortcutWindow::ClearKeyboardIcon( int iconIndex )
{
	cBaseIcon* pIcon = (cBaseIcon*)mKeyboardIcon.GetAt( iconIndex );
	if( pIcon )
		pIcon->ClearImage();
	else
	{
		assert( 0 && "Wrong Keyboard Index" );
		return;
	}
}

void cShortcutWindow::ClearAllKeyboardIcon()
{
	sIconMap::cIterator pos = mKeyboardIcon.Begin();
	sIconMap::cIterator end = mKeyboardIcon.End();
	for( ; pos != end ; ++pos )
	{
		cBaseIcon* pIcon = (cBaseIcon*)(*pos).mSecond;

		if( pIcon )
		{
			pIcon->ClearImage();
		}
		else
		{
			assert(0);
			continue;
		}
	}
}

void cShortcutWindow::ClearShortcutIcon( int iconIndex )
{
	cBaseIcon* pIcon = (cBaseIcon*)mShortcutIcon.GetAt( iconIndex );
	if( pIcon )
		pIcon->ClearImage();
	else
	{
		assert( 0 && "Wrong Shortcut Index" );
		return;
	}

}

void cShortcutWindow::ClearAllShortcutIcon()
{
	sIconMap::cIterator pos = mShortcutIcon.Begin();
	sIconMap::cIterator end = mShortcutIcon.End();
	for( ; pos != end ; ++pos )
	{
		cBaseIcon* pIcon = (cBaseIcon*)(*pos).mSecond;

		if( pIcon )
		{
			pIcon->ClearImage();
		}
		else
		{
			assert(0);
			continue;
		}
	}
}

void cShortcutWindow::SetDefaultShortcutIcon()
{
	sIconMap::cIterator pos = mShortcutIcon.Begin();
	sIconMap::cIterator end = mShortcutIcon.End();
	for( ; pos != end ; ++pos )
	{
		int		   index = (*pos).mFirst;
		cBaseIcon* pIcon = (cBaseIcon*)(*pos).mSecond;
		
		if( pIcon )
		{
			cIconParam* pIconParam = UIMAN->GetIconParam( SHORTCUT_ICON_BASE_INDEX + index );
			if ( pIconParam )
				pIcon->ChangeImage( pIconParam );
			else
			{
				assert(0);
				continue;
			}	
		}
		else
		{
			assert(0);
			continue;
		}
	}

}
void cShortcutWindow::HideSelectImage()
{
	mSelectIcon = 0;
	mShowSelect = false;
}

void cShortcutWindow::UpdateSelect( int iconID )
{
	if( iconID < 0 )
	{
		assert( 0 && "Worning icon ID" );
		return;
	}

	/// 단축키 아이콘 눌림
	cBaseIcon* pIcon = (cBaseIcon*)mShortcutIcon.GetAt( iconID );
	if( pIcon )
	{
		// 새 선택 아이콘 눌림효과
		mSelectIcon = pIcon;
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
		return;
	}
	
	/// 키보드 아이콘 눌림
	pIcon = (cBaseIcon*)mKeyboardIcon.GetAt( iconID );
	if( pIcon )
	{
		// 새 선택 아이콘 눌림효과
		mSelectIcon = pIcon;
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
		return;
	}
	else
	{
		assert( 0 && "Worning icon ID" );
		return;
	}
}

void cShortcutWindow::HideESC( bool onsound /*=true*/ )
{
	if( SHORTCUTMAN->IsSettingChanged() == true )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_SHORTCUT_CANCEL, eSKIN_YESNO,
			GAMERESOURCEMAN->GetGameText( 10202 ),
			GAMERESOURCEMAN->GetGameText( 10200 ) );
	}
	else
		Hide();
}

cUINode* cShortcutWindow::GetSpecialKeyNode( const cUIPos& pos )
{
	if( mTabAbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_TAB);

	if( mLCtrlAbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_CTRLLEFT);

	if( mRCtrlAbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_CTRLRIGHT);

	if( mRAltAbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_ALTRIGHT);

	if( mSpaceBarAbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_SPACEBAR);

	if( mNumPad0AbsoluteRect.ContainPoint( pos ) == true )
		return (cUINode*)mKeyboardIcon.GetAt(eKEYBOARD_NUMPAD0);

	return NULL;
}

bool cShortcutWindow::UpdateSpecialKeyRect()
{
	/// 선택 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* specialKeySkin;
	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_Tab" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mTabAbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
								mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
								mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
								mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );


	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_LCtrl" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mLCtrlAbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
		 						  mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
								  mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
								  mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );


	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_RCtrl" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mRCtrlAbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
								  mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
								  mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
							  	  mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );

	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_RAlt" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mRAltAbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
						  	  	 mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
						  	  	 mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
						  	  	 mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );

	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_Space_Bar" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mSpaceBarAbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
						  	  		 mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
						  	  		 mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
						  			 mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );

	specialKeySkin = pSkin->GetNodeSkin( "Icon_Key_Num0" );
	if( !specialKeySkin )
	{
		assert(0);
		return false;
	}
	mNumPad0AbsoluteRect = cUIRect( mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX,
									mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY,
									mAbsoluteRect.mLeft + specialKeySkin->mSkinInfo->mX + specialKeySkin->mSkinInfo->mTexWidth,
									mAbsoluteRect.mTop  + specialKeySkin->mSkinInfo->mY + specialKeySkin->mSkinInfo->mTexHeight );

	return true;
}
