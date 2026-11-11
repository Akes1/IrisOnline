#include "stdafx.h"
#include "GMTool.h"
#include <CommCtrl.h>
#include "ObjectManager.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "Protocol_Cheat.h"
#include "Hero.h"
#include "resource.h"
#include "Application.h"

#include "WorldManager.h"
#include "NaviMesh.h"
#include "SmallMiniMapWindow.h"

#include "DuelManager.h"

bool cGMTool::LoadMapData()
{
	/// 대분류 콤보
	HWND combo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST );
	if( ::IsWindow( combo ) == false )
	{
		assert(0);
		return false;
	}

	LPCTSTR highName = NULL;

	typedef tPointerHashMap<unsigned long, void*> cArrayHashMap;
	cArrayHashMap* mapInfo = GAMERESOURCEMAN->GetMapInfo();
	
	cArrayHashMap::cConstIterator pos;
	cArrayHashMap::cConstIterator end;
	if( mapInfo )
	{
		pos = mapInfo->Begin();
		end = mapInfo->End();
	}
	else
		pos = end;

	//for( unsigned int i=MAP_MIN; i < MAP_COUNT+1; ++i )
	for( ; pos != end; ++pos )
	{
		unsigned long mapIndex = (*pos).mFirst;
		
		///중복 체크
		if( mMapImgHashMap.GetAt( mapIndex ) != 0 )
			continue;

		/// 난이도 체크
		tArray<unsigned char> modeArr;
		GAMERESOURCEMAN->GetMapMode( mapIndex, &modeArr );
		if( modeArr.GetSize() < 1 )
		{
			assert(0);
			continue;
		}

		highName = GAMERESOURCEMAN->GetMapName( mapIndex );
		if( highName == NULL )
		{
			assert(0 && "fail to get name" );
			return false;
		}

		/// CB_ADDSTRING 메시지를 보내서 성공하면 할당 받은 위치를 받음
		LRESULT ret = SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)(highName));
		if ( ret == CB_ERR || ret == CB_ERRSPACE )
			return false;
		
		/// 항목 데이터 입력.
		SendMessage( combo, CB_SETITEMDATA, ret, (LPARAM)mapIndex );

		{
			/// 현재 hero의 맵 번호 알아오기
			unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIndex, modeArr[0] );

			/// 미니맵 로딩
			cString pathName;
			pathName.Format( "./language/%s/MapImage_%02d.tga", cApplication::mLangaugeFolder.Cstr(), folderIdx );
					
			cTargaImage mapImgLoader;
			mapImgLoader.Load( pathName.Cstr() );
			mapImgLoader.ChangeColorByteOrder();

			/// 이미지로부터 장치 독립 비트맵을 생성
			BITMAPINFO bi;
			bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
			bi.bmiHeader.biWidth = mapImgLoader.GetWidth();
			bi.bmiHeader.biHeight = mapImgLoader.GetHeight();
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = mapImgLoader.GetBytesPerPixel() * 8;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage = (DWORD)mapImgLoader.GetSize();
			bi.bmiHeader.biXPelsPerMeter = 0;
			bi.bmiHeader.biYPelsPerMeter = 0;
			bi.bmiHeader.biClrUsed = 0;
			bi.bmiHeader.biClrImportant = 0;
			bi.bmiColors[0].rgbBlue = 0;
			bi.bmiColors[0].rgbGreen = 0;
			bi.bmiColors[0].rgbRed = 0;
			bi.bmiColors[0].rgbReserved = 0;

			void* pBuf = 0;
			HDC memdc  = CreateCompatibleDC( 0 );
			HBITMAP bitmap = CreateDIBSection( memdc, &bi, DIB_RGB_COLORS, &pBuf, 0, 0 );
			mMapImgHashMap.Insert( mapIndex, bitmap );

			if( pBuf )
			{
				unsigned int bytes = (((bi.bmiHeader.biBitCount*mapImgLoader.GetWidth() + 31) & (~31))/8)*mapImgLoader.GetHeight();
				if( bytes == mapImgLoader.GetSize() )
				{
					::memcpy( pBuf, mapImgLoader.GetBufferPtr(), mapImgLoader.GetSize() );
				}
				else
				{
					char* src = (char*)mapImgLoader.GetBufferPtr();
					char* dest = (char*)pBuf;

					unsigned int srcOffset = mapImgLoader.GetSize()/mapImgLoader.GetHeight();
					unsigned int destOffset = bytes/mapImgLoader.GetHeight();

					for( unsigned int i = 0;i<mapImgLoader.GetHeight();i++ )
					{
						::memcpy( dest, src, srcOffset );

						src += srcOffset;
						dest += destOffset;
					}
				}
			}
			DeleteDC( memdc );
		}
	}

	/// 유저 위치, 추적자 위치 이미지 로딩
	/// 미니맵 로딩
	cString pathName;
	pathName.Format( "./language/%s/U_Basic_01.tga", cApplication::mLangaugeFolder.Cstr() );

	cTargaImage mapImgLoader;
	mapImgLoader.Load( pathName.Cstr() );
	mapImgLoader.ChangeColorByteOrder();

	/// 이미지로부터 장치 독립 비트맵을 생성
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bi.bmiHeader.biWidth = mapImgLoader.GetWidth();
	bi.bmiHeader.biHeight = mapImgLoader.GetHeight();
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = mapImgLoader.GetBytesPerPixel() * 8;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = (DWORD)mapImgLoader.GetSize();
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	bi.bmiColors[0].rgbBlue = 0;
	bi.bmiColors[0].rgbGreen = 0;
	bi.bmiColors[0].rgbRed = 0;
	bi.bmiColors[0].rgbReserved = 0;

	void* pBuf = 0;
	HDC memdc  = CreateCompatibleDC( 0 );
	mHeroTargetPosImg = CreateDIBSection( memdc, &bi, DIB_RGB_COLORS, &pBuf, 0, 0 );

	if( pBuf )
	{
		unsigned int bytes = (((bi.bmiHeader.biBitCount*mapImgLoader.GetWidth() + 31) & (~31))/8)*mapImgLoader.GetHeight();
		if( bytes == mapImgLoader.GetSize() )
		{
			::memcpy( pBuf, mapImgLoader.GetBufferPtr(), mapImgLoader.GetSize() );
		}
		else
		{
			char* src = (char*)mapImgLoader.GetBufferPtr();
			char* dest = (char*)pBuf;

			unsigned int srcOffset = mapImgLoader.GetSize()/mapImgLoader.GetHeight();
			unsigned int destOffset = bytes/mapImgLoader.GetHeight();

			for( unsigned int i = 0;i<mapImgLoader.GetHeight();i++ )
			{
				::memcpy( dest, src, srcOffset );

				src += srcOffset;
				dest += destOffset;
			}
		}
	}
	DeleteDC( memdc );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	// HERO 이미지
	cUINodeSkin* heroSkin = pSkin->GetNodeSkin( "SmallMiniMap_Hero" );
	if( heroSkin )
	{
		mHeroImg.mTexturePos.x   = heroSkin->mSkinInfo->mTexX;
		mHeroImg.mTexturePos.y   = heroSkin->mSkinInfo->mTexY;
		mHeroImg.mImgWidHeight.x = heroSkin->mSkinInfo->mTexWidth;
		mHeroImg.mImgWidHeight.y = heroSkin->mSkinInfo->mTexHeight;
	}
	else
	{
		mHeroImg.mTexturePos.x   = 838;
		mHeroImg.mTexturePos.y   = 991;
		mHeroImg.mImgWidHeight.x = 13;
		mHeroImg.mImgWidHeight.y = 17;
	}

	cUINodeSkin* findSkin = pSkin->GetNodeSkin( "SmallMiniMap_Position" );
	if( findSkin )
	{
		mSearchImg.mTexturePos.x   = findSkin->mSkinInfo->mTexX;
		mSearchImg.mTexturePos.y   = findSkin->mSkinInfo->mTexY;
		mSearchImg.mImgWidHeight.x = findSkin->mSkinInfo->mTexWidth;
		mSearchImg.mImgWidHeight.y = findSkin->mSkinInfo->mTexHeight;
	}
	else
	{
		mSearchImg.mTexturePos.x   = 303;
		mSearchImg.mTexturePos.y   = 719;
		mSearchImg.mImgWidHeight.x = 32;
		mSearchImg.mImgWidHeight.y = 32;
	}

	SendMessage( combo, CB_SETCURSEL, 0, 0 );
	
	return true;
}

void cGMTool::OnShowMap()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_MAP];

	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, _T("") );

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_MAP].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_MAP].y = GetSystemMetrics(SM_CYCAPTION);

	EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), FALSE );
	SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_X, "" );
	SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_Y, "" );

	/// 맵 분류 콤보 선택 셋팅하기
	HWND combo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST );
	int  count = SendMessage( combo, CB_GETCOUNT, NULL, NULL );

	if( HERO )
	{
		unsigned short  heroMapIndex = HERO->GetMapNum();
		for( int index = 0; index < count; ++index )
		{
			unsigned short comboMapIdx = (unsigned short)SendMessage( combo, CB_GETITEMDATA, index, 0 );
			if( comboMapIdx == heroMapIndex )
			{
				SendMessage( combo, CB_SETCURSEL, index, 0 );
				break;
			}
		}
		DrawMap( heroMapIndex );
	}

	mIsLeaveEventTracking = false;

	/// 방 리스트 초기화
	SendMessage( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST ), LVM_DELETEALLITEMS, 0, 0 );

	SetTimer( mSubDlg[GMTOOL_SUBDLG_MAP], eGMTOOL_TIMER_HERO_POS_UPDATE, 1000, NULL );
}

void cGMTool::OnHideMap()
{
	mChasePosImg.mIsShow = false;
	mChasePosImg.mMapIndex = 0;
	mChasePosImg.mRoomIndex = 0;

	KillTimer( mSubDlg[GMTOOL_SUBDLG_MAP], eGMTOOL_TIMER_HERO_POS_UPDATE );

}

void cGMTool::OnCommandMap( int id, int msg )
{
	if( !HERO )
		return;

	switch( id )
	{
	case IDC_BUTTON_MAP_CANCEL:
	case IDC_BUTTON_MAP_CANCEL2:
	case IDC_BUTTON_MAP_CANCEL3:
	case IDC_BUTTON_MAP_CANCEL4:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_MAP], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;

	case IDC_BUTTON_MAP_TELEPORT:
		{
			// 결투 모드시 맵 이동 불가
			if( DUELMAN->IsDuelEnable() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			/// 상태 검사
			if( !(HERO->GetState() == eOBJECT_STATE_IDLE || HERO->GetState() == eOBJECT_STATE_MOVE) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			if( HERO->IsJumping() == true )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			if( HERO->IsTransformMonster() == false )
			{
				if( HERO->IsRideVehicle() == true )
				{
					MSGROOT msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_PLAYER;
					msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					HERO->AlightVehicle();
				}
			}

			/// 맵 번호 받아오기
			int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
			unsigned short mapIdx = (unsigned short)SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );

			/// 위치 받아 오기
			BOOL retcodeX, retcodeY;
			unsigned int posX, posY;
			
			posX = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_X, &retcodeX, false );
			posY = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_Y, &retcodeY, false );

			if( retcodeX == FALSE)
				posX = 0;
			if( retcodeY == FALSE )
				posY = 0;

			/// 맵 위치 받아오기
			if( posX < 0 || posX > MAPTEX_W || posY < 0 || posY > MAPTEX_H )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8087 ) );
				return;
			}

			if( select == CB_ERR || mapIdx == CB_ERR )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8059 ) );
				return;
			}

			/// 테마, PVP 맵에서 일반맵으로 이동은 가능하지만 테마, PVP, 튜토리얼 간의 맵 이동은 안된다.
			if( HERO->GetMapNum() > MAP_MAX && mapIdx > MAP_MAX  && HERO->GetMapNum() != mapIdx &&mChasePosImg.mRoomIndex == mChasePosImg.mMapIndex )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8085 ) );
				return;
			}

			if( HERO->GetStopFlag() == eSTOP_MAPCHANGE )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8086 ) );
				return;
			}

			HERO->CancelByMapchange();

			/// 맵이동 대기
			if( HERO->GetState() == eOBJECT_STATE_MOVE )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );

				/// 따라가기 종료
				if( HERO->IsFollowing() )
					HERO->ReleaseFollowPlayer();
			}

			HERO->SetStopFlag( eSTOP_MAPCHANGE );
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_GM_TOOL_GOTO_DUNGEON, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
				);

			/// 일반 맵 이동
			if( mapIdx <= MAP_MAX )
			{
				if( HERO->GetMapNum() > MAP_MAX )
				{
					/// 테마, PVP, 튜토리얼에서 일반맵으로 이동
					if( NETWORK->IsConnected() )
					{
						MSG_REQ_GOTO_GAMESERVER msg;
						::memset( &msg, 0, sizeof(msg) );
						msg.Category = NM_CHEAT;
						msg.Protocol = NM_CHEAT_GOTO_GAMESERVER_REQ;
						msg.mMapNum = (short)mapIdx;
						msg.mPosX = (float)posX*100;
						msg.mPosY = (float)posY*100;

						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					}
				}
				else
				{
					/// 좌표이동 시점
					if( NETWORK->IsConnected() )
					{
						MSG_REQ_CHEAT_GO msg;
						::memset( &msg, 0, sizeof(msg) );
						msg.Category = NM_CHEAT;
						msg.Protocol = NM_CHEAT_GO_REQ;
						msg.mMapNum = (short)mapIdx;
						msg.mDestX = (float)posX*100;
						msg.mDestY = (float)posY*100;

						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					}
				}
			}
			else
			{
				/// 맵(채널)이 같은지 체크
				if( HERO->GetMapNum() == mapIdx )
				{
					/// 추적중인 유저가 있다면 해당 방의 해당 좌표로 이동.
					if( mChasePosImg.mIsShow == true && mChasePosImg.mMapIndex == mapIdx && mChasePosImg.mRoomIndex != mapIdx )
					{
						/// 좌표이동 시점
						if( NETWORK->IsConnected() )
						{
							MSG_REQ_CHEAT_GO msg;
							::memset( &msg, 0, sizeof(msg) );
							msg.Category = NM_CHEAT;
							msg.Protocol = NM_CHEAT_GO_REQ;
							msg.mMapNum = (short)mChasePosImg.mRoomIndex;
							msg.mDestX = (float)posX*100;
							msg.mDestY = (float)posY*100;

							NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
						}
					}
					else
					{
						/// 순간 이동 좌표 시점
						if( NETWORK->IsConnected() )
						{
							MSG_REQ_CHEAT_TELEPORT msg;
							::memset( &msg, 0, sizeof(msg) );
							msg.Category = NM_CHEAT;
							msg.Protocol = NM_CHEAT_TELEPORT_REQ;
							msg.mDestX = (float)posX*100;
							msg.mDestY = (float)posY*100;
							NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
						}
					}
				}
				else
				{
					/// 채널 이동 메시지 발송
					MSG_REQ_GOTO_INSTANT_DUNGEON msg;
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GOTO_INSTANT_DUNGEON_REQ;
					msg.type = (unsigned short)mapIdx;
					if( NETWORK->IsConnected() )
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_GOTO_INSTANT_DUNGEON) );
				}
			}
		}
		break;

	case IDC_COMBO_MAP_LIST:
		if( msg == CBN_SELCHANGE )
		{
			InvalidateRect( mSubDlg[GMTOOL_SUBDLG_MAP], NULL, FALSE );
		}
		break;
	case IDC_EDIT_MAP_ID:
		if( msg == EN_CHANGE )
		{
			EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), FALSE );
		}
		break;
	case IDC_BUTTON_MAP_SEARCH:
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, _T("") );

			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_SEARCH msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_SEARCH_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case IDC_BUTTON_MAP_CALL:
		{
			if( mChasePosImg.mIsShow == false )
			{
				EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), FALSE );
				return;
			}
			
			/// 일반 맵은 클라이언트에서 같은 맵인지 체크. 그 외의 맵은 서버에서 같은 방인지 체크.
			if( HERO->GetMapNum() <= MAP_MAX )
			{
				if( mChasePosImg.mMapIndex != HERO->GetMapNum() )
				{
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8078 ) );
					EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), FALSE );
					return;
				}
			}
			
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_CALL msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_CALL_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_ID, _T("") );
			mChasePosImg.mIsShow = false;
			InvalidateRect( mSubDlg[GMTOOL_SUBDLG_MAP], NULL, FALSE );
		}
		break;
	case IDC_BUTTON_MAP_REFRESH:
		{
			/// 리스트 초기화
			SendMessage( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LIST_MAP_ROOM_LIST ), LB_RESETCONTENT, 0, 0 );

			/// 일반 맵은 클라이언트에서 같은 맵인지 체크. 그 외의 맵은 서버에서 같은 방인지 체크.
			if( HERO->GetMapNum() <= MAP_MAX )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8082 ) );
				return;
			}

			/// 서버에 PVP, 테마 리스트 요구
			int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
			unsigned int mapIdx = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );
			if( select == CB_ERR || mapIdx == CB_ERR )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8059 ) );
				return;
			}

			/// 일반 맵은 클라이언트에서 같은 맵인지 체크. 그 외의 맵은 서버에서 같은 방인지 체크.
			if( HERO->GetMapNum() != mapIdx )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8082 ) );
				return;
			}

			MSG_REQ_USED_ROOM_ID msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_USED_ROOM_ID_REQ;
			msg.type = (unsigned short)mapIdx;

			if( NETWORK->IsConnected() )
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_USED_ROOM_ID ) );

		}
		break;
	case IDC_BUTTON_MAP_ROOM_MOVE:
		{
			// 결투 모드시 맵 이동 불가
			if( DUELMAN->IsDuelEnable() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			if( HERO->GetStopFlag() == eSTOP_MAPCHANGE )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8086 ) );
				return;
			}

			long roomIndex = 255;
			int selectIdx = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_GETNEXTITEM, (WPARAM)(int)-1, (LPARAM)( LVNI_ALL | LVNI_SELECTED ) );

			if( selectIdx == -1 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8057 ) );
				return;
			}
			else
			{
				LVITEM LvItem;
				LvItem.mask = LVIF_PARAM;
				LvItem.iItem = selectIdx;
				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_GETITEM, 0, (LPARAM)&LvItem );

				roomIndex = (long)LvItem.lParam;
			}

			HERO->CancelByMapchange();

			/// 맵이동 대기
			if( HERO->GetState() == eOBJECT_STATE_ATTACK || HERO->GetState() == eOBJECT_STATE_MOVE )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );

				/// 따라가기 종료
				if( HERO->IsFollowing() )
					HERO->ReleaseFollowPlayer();
			}

			HERO->SetStopFlag( eSTOP_MAPCHANGE );
 			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 				eMBEVENT_GM_TOOL_GOTO_DUNGEON, eSKIN_NONE,
 				GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
 				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
 				);

			MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_REQ;
			msg.mapNum   = (unsigned short)roomIndex;

			if( NETWORK->IsConnected() )
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON ) );
		}
		break;
	case IDC_BUTTON_MAP_DIRECT_MOVE:
		{			
			// 결투 모드시 맵 이동 불가
			if( DUELMAN->IsDuelEnable() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}
			
			/// 상태 검사
			if( !(HERO->GetState() == eOBJECT_STATE_IDLE || HERO->GetState() == eOBJECT_STATE_MOVE) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			if( HERO->IsJumping() == true )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
				return;
			}

			if( HERO->IsTransformMonster() == false )
			{
				if( HERO->IsRideVehicle() == true )
				{
					MSGROOT msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_PLAYER;
					msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					HERO->AlightVehicle();
				}
			}

			/// 내용 받기
			BOOL ret = FALSE;
			unsigned int uniqueIndex = 0;
			uniqueIndex = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_UNIQUE_INDEX, &ret, FALSE );
			if( ret == false )
			{
				/// 방정보 받아오기 실패
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8059 ) );
				return;
			}

			/// unique index 값에 해당하는 방으로 바로 이동하기.
			if( 0 < uniqueIndex && uniqueIndex < UINT_MAX )
			{
				WORD mapIndex = HIWORD(uniqueIndex);
				WORD roomIndex = LOWORD(uniqueIndex);

				int i = SendMessage( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST ), CB_GETCURSEL, 0, 0 );
				int SelectedMapIndex = SendMessage( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST ), CB_GETITEMDATA, (WPARAM)( i ), 0 );
				if( i == CB_ERR || SelectedMapIndex == CB_ERR )
				{
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8059 ) );
					return;
				}

				///현재 선택된 방과 이동할 방의 맵이(채널이) 같은지 검사
				if( mapIndex != SelectedMapIndex )
				{
					/// 다른 맵으로(채널으로)는 한번에 이동이 불가함.
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8053 ) );
					return;
				}

				if( HERO->GetStopFlag() == eSTOP_MAPCHANGE )
				{
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8086 ) );
					return;
				}

				HERO->CancelByMapchange();

				/// 맵이동 대기
				if( HERO->GetState() == eOBJECT_STATE_MOVE )
				{
					HERO->SetState( eOBJECT_STATE_IDLE );

					/// 따라가기 종료
					if( HERO->IsFollowing() )
						HERO->ReleaseFollowPlayer();
				}

				HERO->SetStopFlag( eSTOP_MAPCHANGE );
 				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 					eMBEVENT_GM_TOOL_GOTO_DUNGEON, eSKIN_NONE,
 					GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
 					GAMERESOURCEMAN->GetGameText( 2 ) // 알림
 					);

				MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON msg;
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_REQ;
				msg.mapNum   = roomIndex;

				if( NETWORK->IsConnected() )
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON ) );				
			}		
		}
		break;
	}
}

void cGMTool::DrawMap( int mapIndex )
{
	HDC hdc = NULL;
	hdc = GetDC(mSubDlg[GMTOOL_SUBDLG_MAP]);
	HDC MemDC;
	HBITMAP OldBitmap;

	MemDC=CreateCompatibleDC(hdc);
	HBITMAP bitmap = (HBITMAP)mMapImgHashMap.GetAt( mapIndex );
	OldBitmap=(HBITMAP)SelectObject(MemDC, bitmap );

	BitBlt( hdc, MAP_IMG_POS_X, MAP_IMG_POS_Y, MAP_IMG_WIDTH, MAP_IMG_HEIGHT, MemDC, 0, 0, SRCCOPY );

	SelectObject(MemDC,OldBitmap);
	DeleteDC(MemDC);
	ReleaseDC( mSubDlg[GMTOOL_SUBDLG_MAP], hdc );
}

void cGMTool::OnLButtonDownMap( int x, int y )
{
	/// 마우스 좌표 - 다이얼로그 좌표 = 맵에서 좌표값
	cUIRect mapRect( MAP_IMG_POS_X, MAP_IMG_POS_Y, MAP_IMG_POS_X + MAP_IMG_WIDTH, MAP_IMG_POS_Y + MAP_IMG_HEIGHT );
	if( mapRect.ContainPoint( cUIPos( x, y ) ) == true )
	{
		// 결투 모드시 맵 이동 불가
		if( DUELMAN->IsDuelEnable() )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
			return;
		}

		/// 상태 검사
		if( !(HERO->GetState() == eOBJECT_STATE_IDLE || HERO->GetState() == eOBJECT_STATE_MOVE) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
			return;
		}

		if( HERO->IsJumping() == true )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
			return;
		}

		if( HERO->IsTransformMonster() == false )
		{
			if( HERO->IsRideVehicle() == true )
			{
				MSGROOT msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_PLAYER;
				msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				HERO->AlightVehicle();
			}
		}
		cUIPos clickPos( x - MAP_IMG_POS_X , y - MAP_IMG_POS_Y );

		/// 맵 위치 받아오기
		cNaviMesh* navi = WORLDMAN->GetNaviMesh();
		if( !navi )
			return;

		float segLen = navi->GetSegmentLength();

		unsigned int mapX = (unsigned int)((clickPos.mX * segLen) / MAPTEX_W * 0.01f );
		unsigned int mapY = (unsigned int)((MAPTEX_H - clickPos.mY) * (segLen / MAPTEX_H) * 0.01f );

		if( mapX < 0 || mapX > MAPTEX_W || mapY < 0 || mapY > MAPTEX_H )
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8087 ) );
			return;
		}

		/// 맵 번호 받아오기
		int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
		unsigned int mapIdx = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );

		if( select == CB_ERR || mapIdx == CB_ERR )
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8059 ) );
			return;
		}

		/// 테마, PVP 맵에서 일반맵으로 이동은 가능하지만 테마, PVP, 튜토리얼 간의 맵 이동은 안된다.
		if( HERO->GetMapNum() > MAP_MAX && mapIdx > MAP_MAX && HERO->GetMapNum() != mapIdx &&mChasePosImg.mRoomIndex == mChasePosImg.mMapIndex )
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8085 ) );
			return;
		}

		if( HERO->GetStopFlag() == eSTOP_MAPCHANGE )
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP],IDC_EDIT_MAP_NOTICE, GAMERESOURCEMAN->GetGameText( 8086 ) );
			return;
		}

		HERO->CancelByMapchange();

		/// 맵이동 대기
		if( HERO->GetState() == eOBJECT_STATE_MOVE )
		{
			HERO->SetState( eOBJECT_STATE_IDLE );

			/// 따라가기 종료
			if( HERO->IsFollowing() )
				HERO->ReleaseFollowPlayer();
		}

		HERO->SetStopFlag( eSTOP_MAPCHANGE );
 		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 			eMBEVENT_GM_TOOL_GOTO_DUNGEON, eSKIN_NONE,
 			GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
 			GAMERESOURCEMAN->GetGameText( 2 ) // 알림
 			);

		/// 일반 맵 이동
		if( mapIdx <= MAP_MAX )
		{
			if( HERO->GetMapNum() > MAP_MAX )
			{
				/// 테마, PVP, 튜토리얼에서 일반맵으로 이동
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_GOTO_GAMESERVER msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GOTO_GAMESERVER_REQ;
					msg.mMapNum = (short)mapIdx;
					msg.mPosX = (float)mapX*100;
					msg.mPosY = (float)mapY*100;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
			else
			{
				/// 좌표이동 시점
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_CHEAT_GO msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GO_REQ;
					msg.mMapNum = (short)mapIdx;
					msg.mDestX = (float)mapX*100;
					msg.mDestY = (float)mapY*100;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
		}
		else
		{
			/// 맵(채널)이 같은지 체크
			if( HERO->GetMapNum() == mapIdx )
			{
				/// 추적중인 유저가 있다면 해당 방의 해당 좌표로 이동.
				if( mChasePosImg.mIsShow == true && mChasePosImg.mMapIndex == mapIdx && mChasePosImg.mRoomIndex != mapIdx )
				{
					/// 좌표이동 시점
					if( NETWORK->IsConnected() )
					{
						MSG_REQ_CHEAT_GO msg;
						::memset( &msg, 0, sizeof(msg) );
						msg.Category = NM_CHEAT;
						msg.Protocol = NM_CHEAT_GO_REQ;
						msg.mMapNum = (short)mChasePosImg.mRoomIndex;
						msg.mDestX = (float)mapX*100;
						msg.mDestY = (float)mapY*100;

						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					}
				}
				else
				{
					/// 순간 이동 좌표 시점
					if( NETWORK->IsConnected() )
					{
						MSG_REQ_CHEAT_TELEPORT msg;
						::memset( &msg, 0, sizeof(msg) );
						msg.Category = NM_CHEAT;
						msg.Protocol = NM_CHEAT_TELEPORT_REQ;
						msg.mDestX = (float)mapX*100;
						msg.mDestY = (float)mapY*100;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					}
				}
			}
			else
			{
				/// 채널 이동 메시지 발송
				MSG_REQ_GOTO_INSTANT_DUNGEON msg;
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_GOTO_INSTANT_DUNGEON_REQ;
				msg.type = (unsigned short)mapIdx;
				if( NETWORK->IsConnected() )
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_GOTO_INSTANT_DUNGEON) );
			}
		}
	}
}

void cGMTool::OnPaintMap( HDC hdc )
{

	/// 맵 이미지 다시 그리기
	int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
	unsigned short mapIdx = (unsigned short)SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );

	if( select == -1 || mapIdx == -1 )
		return;

	RECT clientRect;
	GetWindowRect( mSubDlg[GMTOOL_SUBDLG_MAP], &clientRect );

	HDC MemDC;
	HBITMAP OldBitmap;
	MemDC     = CreateCompatibleDC( hdc );
	//OldBitmap = (HBITMAP)SelectObject(MemDC, mMapImg[mapIdx]);
	HBITMAP bitmap = (HBITMAP)mMapImgHashMap.GetAt( mapIdx );
	OldBitmap=(HBITMAP)SelectObject(MemDC, bitmap );

	HDC backSurface;
	HBITMAP backBitmap;
	HBITMAP oldBackBitmap;
	backSurface = CreateCompatibleDC( hdc );
	backBitmap  = CreateCompatibleBitmap( hdc, MAP_IMG_WIDTH, MAP_IMG_HEIGHT );
	oldBackBitmap = (HBITMAP)SelectObject( backSurface, backBitmap );
	
	BitBlt( backSurface, 0, 0, MAP_IMG_WIDTH, MAP_IMG_HEIGHT, MemDC, 0, 0, SRCCOPY );

	SelectObject(MemDC,OldBitmap);
	DeleteDC(MemDC);

	if( HERO && HERO->GetMapNum() == mapIdx )
	{
		HDC memDC2;
		HBITMAP oldBitmap2;
		memDC2 = CreateCompatibleDC( hdc );
		oldBitmap2 = (HBITMAP)SelectObject( memDC2, mHeroTargetPosImg );

		cNaviMesh* navi = WORLDMAN->GetNaviMesh();
		if( !navi )
		{
			assert(0);
			return;
		}

		float segLen = navi->GetSegmentLength();

		int heroX = (int)(HERO->GetXPos() / segLen * MAPTEX_W);
		int heroY = (int)(MAPTEX_H - (HERO->GetYPos() / segLen * MAPTEX_H ));

		cUIPos point;
		point.mX = heroX - ( mHeroImg.mImgWidHeight.x / 2 );
		point.mY = heroY - ( mHeroImg.mImgWidHeight.y / 2 );
#ifdef _GMTOOL
		TransparentBlt( backSurface, point.mX, point.mY, mHeroImg.mImgWidHeight.x , mHeroImg.mImgWidHeight.y , 
							  memDC2, mHeroImg.mTexturePos.x , mHeroImg.mTexturePos.y, mHeroImg.mImgWidHeight.x , mHeroImg.mImgWidHeight.y, RGB(114,114,114) );
#endif
		SelectObject( memDC2, oldBitmap2 );
		DeleteDC( memDC2 );
	}

	if( mChasePosImg.mIsShow == true && mChasePosImg.mMapIndex == mapIdx )
	{
		HDC memDC3;
		HBITMAP oldBitmap3;
		memDC3 = CreateCompatibleDC( hdc );
		oldBitmap3 = (HBITMAP)SelectObject( memDC3, mHeroTargetPosImg );
	
		cUIPos point;
		point.mX = mChasePosImg.mPosX - ( mSearchImg.mImgWidHeight.x / 2 );
		point.mY = mChasePosImg.mPosY - ( mSearchImg.mImgWidHeight.y / 2 );
#ifdef _GMTOOL
		TransparentBlt( backSurface, point.mX, point.mY, mSearchImg.mImgWidHeight.x, mSearchImg.mImgWidHeight.x, 
			memDC3, mSearchImg.mTexturePos.x, mSearchImg.mTexturePos.y, mSearchImg.mImgWidHeight.x, mSearchImg.mImgWidHeight.x, RGB(0,0,0) );
#endif
		SelectObject( memDC3, oldBitmap3 );
		DeleteDC( memDC3 );
	}

	BitBlt( hdc, MAP_IMG_POS_X, MAP_IMG_POS_Y, clientRect.right, clientRect.bottom, backSurface, 0, 0, SRCCOPY );
	SelectObject( backSurface, oldBackBitmap );
	DeleteObject( backBitmap );
	DeleteDC( backSurface );

}

void cGMTool::OnMouseMoveMap( HWND wnd, int x, int y )
{
	/// 마우스 좌표 - 다이얼로그 좌표 = 맵에서 좌표값
	cUIRect mapRect( MAP_IMG_POS_X, MAP_IMG_POS_Y, MAP_IMG_POS_X + MAP_IMG_WIDTH, MAP_IMG_POS_Y + MAP_IMG_HEIGHT );
	if( mapRect.ContainPoint( cUIPos( x, y ) ) == true )
	{
		cUIPos mousePos( x - MAP_IMG_POS_X , y - MAP_IMG_POS_Y );
		mousePos.mY = MAP_IMG_HEIGHT - mousePos.mY;

		SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_X, mousePos.mX, FALSE );
		SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_Y, mousePos.mY, FALSE );

		if( mIsLeaveEventTracking == false )
		{
			TRACKMOUSEEVENT mouseEvent;
			mouseEvent.cbSize = sizeof( TRACKMOUSEEVENT );
			mouseEvent.dwFlags = TME_LEAVE;
			mouseEvent.dwHoverTime = HOVER_DEFAULT;
			mouseEvent.hwndTrack = wnd;
			TrackMouseEvent( &mouseEvent );
			mIsLeaveEventTracking = true;
		}
	}
}

void cGMTool::OnMouseLeaveMap()
{
	if( mIsLeaveEventTracking == true )
	{
		SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_X, "" );
		SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_POS_Y, "" );
	}

	mIsLeaveEventTracking = false;
}

void cGMTool::ShowSearchPlayer( unsigned short mapIndex, float posX, float posY )
{
	/// 일반 맵에서 검색
	if( HERO->GetMapNum() <= MAP_MAX )
	{
		/// 맵 분류 콤보 선택 셋팅하기
		HWND combo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST );
		int  count = SendMessage( combo, CB_GETCOUNT, NULL, NULL );

		for( int index = 0; index < count; ++index )
		{
			int comboMapIdx = SendMessage( combo, CB_GETITEMDATA, index, 0 );
			if( comboMapIdx == mapIndex )
			{
				SendMessage( combo, CB_SETCURSEL, index, 0 );
				break;
			}
		}

		cNaviMesh* navi = WORLDMAN->GetNaviMesh();
		if( !navi )
		{
			assert(0);
			return;
		}

		float segLen = navi->GetSegmentLength();

		mChasePosImg.mIsShow = true;
		mChasePosImg.mPosX = (int)( posX / segLen * MAPTEX_W );
		mChasePosImg.mPosY = (int)( MAPTEX_H - ( posY / segLen * MAPTEX_H ) );
		mChasePosImg.mMapIndex = mapIndex;
		mChasePosImg.mRoomIndex = mapIndex;

		InvalidateRect( mSubDlg[GMTOOL_SUBDLG_MAP], NULL, FALSE );

		EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), TRUE );
	}
	else
	{
		/// 테마인던/PVP/튜토리얼 에서 검색
		cNaviMesh* navi = WORLDMAN->GetNaviMesh();
		if( !navi )
		{
			assert(0);
			return;
		}

		float segLen = navi->GetSegmentLength();

		mChasePosImg.mIsShow = true;
		mChasePosImg.mPosX = (int)( posX / segLen * MAPTEX_W );
		mChasePosImg.mPosY = (int)( MAPTEX_H - ( posY / segLen * MAPTEX_H ) );
		mChasePosImg.mMapIndex = HERO->GetMapNum();
		mChasePosImg.mRoomIndex = mapIndex;

		InvalidateRect( mSubDlg[GMTOOL_SUBDLG_MAP], NULL, FALSE );

		/// 주인공과 방 번호가 같으면 표시한다.
//		if( HERO && HERO->GetMapNum() == mapIndex )
			EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), TRUE );
// 		else 
// 			EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_MAP],IDC_BUTTON_MAP_CALL), FALSE );
	}
}

void cGMTool::ShowSearchChannelPlayer( short channelNum )
{
	cStringT msg;
	if( ( channelNum >= MAP_PVP_MIN   && channelNum <= MAP_PVP_MAX   ) || 
		( channelNum >= MAP_THEME_MIN && channelNum <= MAP_THEME_MAX ) || 
		( channelNum >= MAP_ETC_MIN   && channelNum <= MAP_ETC_MAX   ))
		msg.Format( GAMERESOURCEMAN->GetGameText( 8098 ), GAMERESOURCEMAN->GetMapName(channelNum) );
	else
		msg.Format( GAMERESOURCEMAN->GetGameText( 8097 ), channelNum );

	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_EDIT_MAP_NOTICE, msg.Cstr() );
}

void cGMTool::SetPvpThemeList( MSG_RES_USED_ROOM_ID* pMsg )
{
	/// 리스트 초기화
	SendMessage( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST ), LVM_DELETEALLITEMS, 0, 0 );

	/// 맵 번호 받아오기
	int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
	unsigned int mapIdx = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );

	HWND listbox = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LIST_MAP_ROOM_LIST );
	if( listbox == NULL )
	{
		assert( 0 && "fail to get list" );
		return;
	}

	cStringT str;
	long*	 pRoomUniqueIndex = pMsg->roomInfo;
	for( long cnt = 0; cnt < pMsg->rowCount; cnt++, pRoomUniqueIndex++ )
	{
		int index;
		cStringT countText;
		countText.Format(_T("%d"), cnt);

		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = cnt;
		LvItem.iSubItem = 0;
		LvItem.pszText = (LPWSTR)countText.Cstr();
		LvItem.lParam = (LPARAM)*pRoomUniqueIndex;
		index = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));
		
		if( index == -1 )
		{	
			assert(0);
			continue;
		}

		LvItem.iSubItem = 1;
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetMapName( mapIdx );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_SETITEMTEXT, (WPARAM)(int)index, (LPARAM)(const LV_ITEM *)(&LvItem));

		cStringT roomNum;
		roomNum.Format(_T("%d"), *pRoomUniqueIndex);
		LvItem.iSubItem = 2;
		LvItem.pszText = (LPWSTR)roomNum.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_SETITEMTEXT, (WPARAM)(int)index, (LPARAM)(const LV_ITEM *)(&LvItem));
	}
}

void cGMTool::OnTimerMap( int timerIndex )
{
	switch( timerIndex )
	{
		case eGMTOOL_TIMER_HERO_POS_UPDATE:
		{
			if( IsWindowVisible( mSubDlg[GMTOOL_SUBDLG_MAP] ) == FALSE )
			{
				KillTimer( mSubDlg[GMTOOL_SUBDLG_MAP], eGMTOOL_TIMER_HERO_POS_UPDATE );
				return;
			}

			int select = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETCURSEL, 0, 0 );
			unsigned short mapIdx = (unsigned short)SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_COMBO_MAP_LIST, CB_GETITEMDATA, select, 0 );
			if( HERO && HERO->GetMapNum() == mapIdx )
			{
				InvalidateRect( mSubDlg[GMTOOL_SUBDLG_MAP], NULL, FALSE );
			}
		}
		break;
	}
}
