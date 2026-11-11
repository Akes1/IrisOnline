#include "StdAfx.h"
#include "QuestSimpleWindow.h"
#include "PlaneObject.h"

#include "QuestManager.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "FontAgent.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PVPManager.h"
#include "TriggerManager.h"

cQuestSimpleWindow::cQuestSimpleWindow()
: mpHighImage(0)
, mpLowImage(0)
, mHighCheckPos(0, 0)
, mHighNoCheckPos(0, 0)
, mLowCheckPos(0, 0)
, mImageSize(0, 0)
, mInitWidth(0)
, mInitX(0)
, mpSelectImage(0)
, mShowSelect( false )
, mpOnSkin(0)
, mpOnImage(0)
, mShowOn(false)
{
	memset( mpBackImage, 0, sizeof(mpBackImage) );
	memset( mpBackSkin, 0, sizeof(mpBackSkin) );
}

cQuestSimpleWindow::~cQuestSimpleWindow()
{
	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
		SAFE_DELETE( mpBackImage[i] );

	SAFE_DELETE( mpOnImage );
	SAFE_DELETE( mpSelectImage );
	SAFE_DELETE( mpLowImage );
	SAFE_DELETE( mpHighImage );

	Clear();
}

void cQuestSimpleWindow::Clear()
{
	/// 클리어
	typedef tPointerList<void*> cProgressList;

	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( data )
		{
			cProgressList::cIterator b = data->mQuestProgressList.Begin();
			cProgressList::cIterator bend = data->mQuestProgressList.End();
			for( ; b != bend; ++b )
			{
				sQuestProgress* progress = (sQuestProgress*)(*b);
				SAFE_DELETE( progress );
			}
			data->mQuestProgressList.Clear();
		}

		SAFE_DELETE( data );
	}
	mQuestMap.Clear();

	mShowSelect = false;
}

void cQuestSimpleWindow::Open()
{
}

void cQuestSimpleWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cQuestSimpleWindow::Show( bool onsound )
{
	if( PVPMAN->IsEnableBattle() == true )
		return;

	cUIWindow::Show( onsound );
}

void cQuestSimpleWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int x = GetScreenWidth() - GetAbsoluteRect().GetWidth() - mInitX;
	int y = GetAbsoluteRect().mTop;
	SetRelativePos( cUIPos(x, y) );

	UpdateSize();
}

bool cQuestSimpleWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 다른윈도우와 스냅 적용 안하도록 함
	SetExceptSnap( true );

	/// 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "Quest_FolderHigh" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "Quest_FolderHighNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		mHighNoCheckPos.mX = pHighSkin->mSkinInfo->mTexX;
		mHighNoCheckPos.mY = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		unsigned int x = pHighSkin->mSkinInfo->mX;
		unsigned int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( mHighCheckPos.mX, mHighCheckPos.mY, mHighCheckPos.mX + w, mHighCheckPos.mY + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		mHighCheckPos.mX = pHighNoSkin->mSkinInfo->mTexX;
		mHighCheckPos.mY = pHighNoSkin->mSkinInfo->mTexY;

		mImageSize.mWidth = w;
		mImageSize.mHeight = h;
	}

	cUINodeSkin* pLowSkin = pSkin->GetNodeSkin( "Quest_FolderLow" );
	if( pLowSkin )
	{
		mpLowImage = new cUIImage( pLowSkin->mpTexture );

		mLowCheckPos.mX = pLowSkin->mSkinInfo->mTexX;
		mLowCheckPos.mY = pLowSkin->mSkinInfo->mTexY;
		unsigned int w = pLowSkin->mSkinInfo->mWidth;
		unsigned int h = pLowSkin->mSkinInfo->mHeight;
		unsigned int x = pLowSkin->mSkinInfo->mX;
		unsigned int y = pLowSkin->mSkinInfo->mY;

		mpLowImage->SetTextureRect( mLowCheckPos.mX, mLowCheckPos.mY, mLowCheckPos.mX + w, mLowCheckPos.mY + h );
		mpLowImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}
	
	// 선택 이미지
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "QuestSimple_Select" );
	if( selectSkin && selectSkin->mpTexture )
	{
		mpSelectImage = new cPlaneObject;

		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)selectSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)selectSkin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)selectSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)selectSkin->mSkinInfo->mTexHeight;
		unsigned int x = GetAbsoluteRect().mLeft;
		unsigned int y = GetAbsoluteRect().mTop;

		if( mpSelectImage->Create( selectSkin->mpTexture, x, y, tw, th, tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create select image");
			return false;
		}
	}

	// 추적 이미지
	mpOnSkin = pSkin->GetNodeSkin( "Quest_ChaseOn" );
	if( mpOnSkin && mpOnSkin->mpTexture )
	{
		mpOnImage = new cPlaneObject;

		unsigned short tx = (unsigned short)mpOnSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)mpOnSkin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)mpOnSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)mpOnSkin->mSkinInfo->mTexHeight;
		unsigned int x = GetAbsoluteRect().mLeft + GetAbsoluteRect().GetWidth() - tw;
		unsigned int y = GetAbsoluteRect().mTop;

		if( mpOnImage->Create( mpOnSkin->mpTexture, x, y, tw, th, tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create mpOnSkin image");
			return false;
		}
	}

	/// 배경 이미지 생성
	mpBackSkin[eBACKIMAGE_TOP] = pSkin->GetNodeSkin( "QuestSimple_Top" );
	mpBackSkin[eBACKIMAGE_BOTTOM] = pSkin->GetNodeSkin( "QuestSimple_Bottom" );
	mpBackSkin[eBACKIMAGE_CENTER] = pSkin->GetNodeSkin( "QuestSimple_Back" );
	mpBackSkin[eBACKIMAGE_LEFT] = pSkin->GetNodeSkin( "QuestSimple_Left" );
	mpBackSkin[eBACKIMAGE_RIGHT] = pSkin->GetNodeSkin( "QuestSimple_Right" );

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackSkin[i] && mpBackSkin[i]->mpTexture )
		{
			/// 텍스처 영역 설정
			unsigned short tx = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexY;
			unsigned short tw = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexWidth;
			unsigned short th = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexHeight;
			unsigned int x = GetAbsoluteRect().mLeft;
			unsigned int y = GetAbsoluteRect().mTop;

			mpBackImage[i] = new cPlaneObject;

			if( mpBackImage[i]->Create( mpBackSkin[i]->mpTexture, x, y, x+1, y+1, tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create back image");
				return false;
			}
		}
	}

	mInitWidth = mAbsoluteRect.GetWidth();
	cUINodeSkin* winSkin = pSkin->GetNodeSkin( mSkinName );
	if( winSkin )
		mInitX = winSkin->mSkinInfo->mX;

	int x = GetScreenWidth() - GetAbsoluteRect().GetWidth() - mInitX;
	int y = GetAbsoluteRect().mTop;
	SetRelativePos( cUIPos(x, y) );
	return true;
}

void cQuestSimpleWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mQuestMap.GetSize() > 0 )
	{
		/// 윈도우 사이즈 설정
		int x = GetAbsoluteRect().mLeft;
		int y = GetAbsoluteRect().mTop;

		/// 이미지 설정
		if( mpBackImage[eBACKIMAGE_TOP] )
		{
			//int skinHeight = mpBackSkin[eBACKIMAGE_TOP]->mSkinInfo->mHeight;
			mpBackImage[eBACKIMAGE_TOP]->SetScreenXY( x, y );
		}
		if( mpBackImage[eBACKIMAGE_CENTER] )
		{
			mpBackImage[eBACKIMAGE_CENTER]->SetScreenXY( x + START_GAPX, y + START_GAPY );
		}
		if( mpBackImage[eBACKIMAGE_BOTTOM] )
		{
			int backHeight = GetAbsoluteRect().GetHeight() - ( 2 * START_GAPY );
			mpBackImage[eBACKIMAGE_BOTTOM]->SetScreenXY( x, y + backHeight + START_GAPY );
		}
		if( mpBackImage[eBACKIMAGE_LEFT] )
		{
			mpBackImage[eBACKIMAGE_LEFT]->SetScreenXY( x, y + START_GAPY );
		}
		if( mpBackImage[eBACKIMAGE_RIGHT] )
		{
			mpBackImage[eBACKIMAGE_RIGHT]->SetScreenXY( x + START_GAPX + mInitWidth, y + START_GAPY );
		}
	}

	// 선택이미지 위치 갱신
	UpdateSelect();
}

void cQuestSimpleWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 데이터 출력
	if( mQuestMap.GetSize() == 0 )
		return;

	/// 이미지 렌더링
	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	/// 선택 이미지
	if( mpSelectImage && mShowSelect )
		mpSelectImage->Draw();

	// 추적이미지 
	if( mpOnImage && mShowOn )
		mpOnImage->Draw();

	int textposX = GetAbsoluteRect().mLeft + START_GAPX;
	int textPosY = GetAbsoluteRect().mTop + START_GAPY;
	int rowHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );
	
	typedef tPointerList<void*> cProgressList;

	/// 출력
	cQuestMap::cIterator b = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; b != end; ++b )
	{
		sQuestHaveData* data = (sQuestHaveData*)(b->mSecond);
		if( !data )
		{
			assert(0);
			return;
		}

		int addX = 0;
		if( data->mCloseImage )
		{
			/// 이미지 텍스쳐 좌표 세팅
			cUIPos texPos = ( data->mClose == false ) ? mHighCheckPos : mHighNoCheckPos;
			data->mCloseImage->SetTextureRect( cUIRect( texPos.mX, texPos.mY,texPos.mX + data->mCloseImage->GetWidth(), texPos.mY + data->mCloseImage->GetHeight() ) );
			data->mCloseImage->SetPos( textposX, textPosY );
			data->mCloseImage->Draw();

			addX = (data->mCloseImage->GetWidth() + 2);
		}

		/// 현재 가진갯수 / 총 갯수 출력
		int have = data->mQuestProgressList.GetSize();

		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText(446), (LPTSTR)data->mMapName.Cstr(), have, data->mTotalCount );

		/// 맵 이름 출력
		pKeeper->AddFontItem( cFontAgent::eFont_UI, temp, textposX + addX, textPosY, COLOR_YELLOW );
		textPosY += rowHeight;

		/// 펼쳐져 있으면 퀘스트 리스트 출력
		if( data->mClose == false )
		{
			addX = 0;

			cProgressList::cIterator b = data->mQuestProgressList.Begin();
			cProgressList::cIterator bend = data->mQuestProgressList.End();
			for( ; b != bend; ++b )
			{
				sQuestProgress* progress = (sQuestProgress*)(*b);
				if( !progress )
				{
					assert(0);
					return;
				}
				
				/// 체크 안되있으면 리스트에 출력 자체를 안함
				if( progress->mCloseImage )
				{
					/// 이미지 텍스쳐 좌표 세팅
					cUIPos texPos = mLowCheckPos;
					progress->mCloseImage->SetTextureRect( cUIRect( texPos.mX, texPos.mY,texPos.mX + progress->mCloseImage->GetWidth(), texPos.mY + progress->mCloseImage->GetHeight() ) );
					progress->mCloseImage->SetPos( textposX, textPosY );
					progress->mCloseImage->Draw();

					addX = (progress->mCloseImage->GetWidth() + 2);
				}

				/// 완료 체크
				cStringT questName;
				unsigned long questColor = COLOR_GREEN;
				questName = progress->mQuestName;

//				cQuestDefine *questDefine = progress->mDefine;
				/// 반복
//				if( questDefine && questDefine->mRepeatType == eQUEST_REPEAT )
//					questName.Append( GAMERESOURCEMAN->GetGameText( 472 ) );

				/// 저 레벨 퀘스트 
				short lowLevel = TRIGGERMAN->GetLowLevel( progress->mIndex );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP ) )
				{
					questName.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
					questColor = COLOR_LOW_LEVEL_QUEST;
				}
			
				/// 퀘스트 이름 출력
				pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)questName.Cstr(), textposX + addX, textPosY, questColor );
				textPosY += rowHeight;
				
				/// 퀘스트 의무조건출력
				if( progress->mDefine )
				{
					/// 1. 시간 제한 출력
					cQuestDefine* define = progress->mDefine;
					cQuestLimit* limit = define->mQuestLimit;
					if( limit && limit->mTime > 0 )
					{
						long restTime = QUESTMAN->GetQuestRestTime( progress->mIndex );
						int time = (int)(restTime*0.001);
						if( time >= 0 )
						{
							int h = time / 3600;
							int m = time / 60;
							int s = time % 60;

							cStringT th,tm,ts;
							if( h != 0 )
								th.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );

							if( m != 0 )
								tm.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
							ts.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );

							th = (th+tm+ts);

							cStringT timeStr;
							timeStr.Format( GAMERESOURCEMAN->GetGameText( 471 ), (LPCTSTR)th.Cstr() );

							pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)timeStr.Cstr(), textposX + addX, textPosY, COLOR_ORANGE );
							textPosY += rowHeight;
						}
					}

					/// 2. 의무조건 상세 표현
					cQuestDuty* duty = define->mQuestDuty;
					if( duty )
					{
						cStringT str;

						__int64 end = 0xFF;
						__int64 check = *progress->mCheck;	/// 참조

						for( unsigned int i = 0; i < MAX_DUTY; ++i )
						{
							eDutyType type = duty->mDuty[i].dutyType;
							unsigned long targetIdx = duty->mDuty[i].targetIdx;
							unsigned int count = duty->mDuty[i].count;
							unsigned long skillIndex = duty->mDuty[i].skillIndex;

							if( type == eDUTY_NONE )
								break;

							/// 정보 출력
							switch( type )
							{
							case eDUTY_HUNT:		/// 사냥
								{
									LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
									str.Format( GAMERESOURCEMAN->GetGameText( 451 ), monName, count );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
									/// 진행중
									else
									{
										__int64 cnt = check & end;
										for( unsigned int b = 0; b < i; ++b)
											cnt = cnt >> 8;

										cStringT str1;
										str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

										str += str1;
									}

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_COLLECT:		/// 수집
								{
									LPCTSTR itemName = ITEMMAN->GetItemName( targetIdx );
									str.Format( GAMERESOURCEMAN->GetGameText( 453 ), itemName, count );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
									/// 진행중
									else
									{
										__int64 cnt = check & end;
										for( unsigned int b = 0; b < i; ++b)
											cnt = cnt >> 8;

										cStringT str1;
										str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

										str += str1;
									}

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_MEET:		/// npc 만나기
								{
									LPCTSTR npcName = GAMERESOURCEMAN->GetNpcName( targetIdx );
									str.Format( GAMERESOURCEMAN->GetGameText( 455 ), npcName );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_ACTIVESKILL:	// 스킬발동
								{
									LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( skillIndex );
									LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
									str.Format( GAMERESOURCEMAN->GetGameText( 456 ), skillName, monName );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_ENHANCE:	// 스킬발동
								{
									str.Format( GAMERESOURCEMAN->GetGameText( 457 ) );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_PUTCARD:	// 스킬발동
								{
									str.Format( GAMERESOURCEMAN->GetGameText( 458 ) );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							case eDUTY_PVPJOIN:
							case eDUTY_PVPWIN:
							case eDUTY_PVPLOSE:
								{
									unsigned int pvpIndex = 0;
									switch( targetIdx )
									{
									case MAP_DEATHMATCH:		pvpIndex = 859;			break;
									case MAP_FLAG:				pvpIndex = 860;			break;
									}

									unsigned int textIndex = 0;
									switch( type )
									{
									case eDUTY_PVPJOIN:		textIndex = 465;			break;
									case eDUTY_PVPWIN:		textIndex = 466;			break;
									case eDUTY_PVPLOSE:		textIndex = 467;			break;
									}

									str.Format( GAMERESOURCEMAN->GetGameText( textIndex ), GAMERESOURCEMAN->GetGameText( pvpIndex ), count );

									/// 완료
									if( (check & end) == (define->mComplete & end) )
										str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
									/// 진행중
									else
									{
										__int64 cnt = check & end;
										for( unsigned int b = 0; b < i; ++b)
											cnt = cnt >> 8;

										cStringT str1;
										str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

										str += str1;
									}

									pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)str.Cstr(), textposX + addX, textPosY, mDefaultColor );
									textPosY += rowHeight;
								}
								break;
							}

							/// 완료 체크용
							end = end << 8;
						}
					} /// 상세표현 완료
				}/// 의무조건 표현 완료
			}
		}
	}

	///
	pKeeper->DrawAll();
}

void cQuestSimpleWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, false, false, false );
}

void cQuestSimpleWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, false );

	/// 이미지 영역 체크
	if( pos.mY < GetAbsoluteRect().mTop + START_GAPY )
		return;

	int leftX  = GetAbsoluteRect().mLeft + START_GAPX;
	int rightX = GetAbsoluteRect().mLeft + START_GAPX + mImageSize.mWidth;

	int rowHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );
	unsigned int selectRow = (pos.mY - (GetAbsoluteRect().mTop + START_GAPY) ) / rowHeight;
	unsigned int row = 0;

	/// 선택된 이미지 찾기
	typedef tPointerList<void*> cProgressList;

	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( !data )
		{
			assert(0);
			return;
		}
		
		/// 선택된 이미지 
		if( selectRow == row && data->mCloseImage )
		{
			data->mClose = !data->mClose;

			/// 맵 라인은 닫히거나 열리니까 재계산하고,
			/// 의무조건 라인은 닫히면 무조건 삭제므로 재계산 안한다
			UpdateSize();
			return;
		}

		row++;

		/// 펼쳐져 있을 경우
		if( data->mClose == false )
		{
			cProgressList::cIterator b = data->mQuestProgressList.Begin();
			cProgressList::cIterator bend = data->mQuestProgressList.End();
			for( ; b != bend; ++b )
			{
				sQuestProgress* progress = (sQuestProgress*)(*b);
				if( !progress )
				{
					assert(0);
					return;
				}

				/// 선택된 이미지 인지 검사
				if( selectRow == row && progress->mCloseImage )
				{
					// 체크 이미지 영역안에 있으면 발생
					if( pos.mX >= leftX && pos.mX <= rightX )
					{
						QUESTMAN->DeleteQuestSelect( progress->mIndex, true );
					}
					else
					{
						QUESTMAN->UpdateChaseQuest( progress->mIndex );
					}
					return;
				}

				row++;

				/// 의무조건 갯수 추가
				cQuestDefine* define = progress->mDefine;
				cQuestDuty* duty = define->mQuestDuty;
				cQuestLimit* limit = define->mQuestLimit;
				if( limit && limit->mTime > 0 )
				{
					if( selectRow == row )
					{
						QUESTMAN->UpdateChaseQuest( progress->mIndex );
						return;
					}

					row++;
				}

				if( duty )
				{
					for( unsigned int i = 0; i < MAX_DUTY; ++i )
					{
						eDutyType type = duty->mDuty[i].dutyType;
						if( type != eDUTY_NONE )
						{
							if( selectRow == row )
							{
								QUESTMAN->UpdateChaseQuest( progress->mIndex );
								return;
							}

							row++;
						}
					}
				}
			}
		}
	}
}

/// 퀘스트정보 삽입
void cQuestSimpleWindow::AddQuestInfo( unsigned long questIndex, __int64* check )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	/// 데이터가 없으면 새로생성
	sQuestHaveData* data = (sQuestHaveData*)mQuestMap.GetAt( define->mTakeMapIndex );
	if( !data )
	{
		data = new sQuestHaveData;
		data->mMapName = GAMERESOURCEMAN->GetMapName( define->mTakeMapIndex );
		data->mClose = false;
		data->mCloseImage = mpHighImage;

		if( mQuestMap.Insert( define->mTakeMapIndex, data ) == false )
		{
			assert(0);
			SAFE_DELETE( data );
			return;
		}
	}

	/// 퀘스트 갯수 갱신
	data->mTotalCount = QUESTMAN->GetCount( define->mTakeMapIndex );

	/// 상세 정보 세팅
	sQuestProgress* progress = new sQuestProgress;
	progress->mIndex = questIndex;
	progress->mDefine = define;
	progress->mCheck = check;
	progress->mCloseImage = mpLowImage;
	progress->mTime = 0;
		
	TCHAR temp[128]={0,};
	::_stprintf_s( temp, _T( "[%d] %s" ), define->mLevel, GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );
	progress->mQuestName = temp;
	
	data->mQuestProgressList.PushBack( progress );

	/// 윈도우 사이즈 재계산
	UpdateSize();

	///
	if( mQuestMap.GetSize() > 0 && mVisible == false )
	{
		cQuestWindow* win = GAMEUI->GetQuestWindow();
		if( win && win->IsShowInfo() == true )
		{
			/// npc 윈도우가 보이면 우선 예약
			if( HERO->IsOpenNpcTalk() == false )
				Show();
			else
				UIMAN->SetRestoreShow( this );
		}
	}

	//UpdateSelect();
}

/// 삭제
void cQuestSimpleWindow::DeleteQuestInfo( unsigned long questIndex )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	/// 데이터가 없으면 새로생성
	sQuestHaveData* data = (sQuestHaveData*)mQuestMap.GetAt( define->mTakeMapIndex );
	if( !data )
	{
		assert(0);
		return;
	}

	typedef tPointerList<void*> cProgressList;
	cProgressList::cIterator b = data->mQuestProgressList.Begin();
	cProgressList::cIterator bend = data->mQuestProgressList.End();
	for( ; b != bend; ++b )
	{
		sQuestProgress* progress = (sQuestProgress*)(*b);
		if( progress && progress->mIndex == questIndex )
		{
			SAFE_DELETE( progress );
			data->mQuestProgressList.Erase( b );
			break;
		}
	}

	/// 하위 데이터가 남아있지 않으면 맵 정보 삭제
	if( data->mQuestProgressList.GetSize() == 0 )
	{
		SAFE_DELETE(data);
		mQuestMap.Erase( define->mTakeMapIndex );
	}

	/// 윈도우 사이즈 재계산
	if( mQuestMap.GetSize() == 0 )
		Hide();
	else
		UpdateSize();

	//
	UpdateSelect();
}	

void cQuestSimpleWindow::UpdateSize()
{
	int totalLine = 0;
	int rowHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );

	typedef tPointerList<void*> cProgressList;

	/// 출력
	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( !data )
		{
			assert(0);
			return;
		}
	
		/// 1. 맵이름 출력 라인 추가
		totalLine++;

		/// 펼쳐져 있으면 퀘스트 리스트 출력
		if( data->mClose == false )
		{
			cProgressList::cIterator b = data->mQuestProgressList.Begin();
			cProgressList::cIterator bend = data->mQuestProgressList.End();
			for( ; b != bend; ++b )
			{
				sQuestProgress* progress = (sQuestProgress*)(*b);
				if( !progress )
				{
					assert(0);
					return;
				}	

				/// 1. 퀘스트 이름 출력 라인 추가
				totalLine++;

				/// 퀘스트 의무조건출력
				if( progress->mDefine )
				{
					/// 1. 시간 제한 출력
					cQuestDefine* define = progress->mDefine;
					cQuestLimit* limit = define->mQuestLimit;
					if( limit && limit->mTime > 0 )
					{
						long restTime = QUESTMAN->GetQuestRestTime( progress->mIndex );
						int time = (int)(restTime*0.001);
						if( time >= 0 )
						{
							/// 3. 의무조건 - 시간 출력 라인 추가
							totalLine++;
						}
					}

					cQuestDuty* duty = define->mQuestDuty;
					if( duty )
					{
						__int64 end = 0xFF;
						//__int64 check = *progress->mCheck;	/// 참조

						for( unsigned int i = 0; i < MAX_DUTY; ++i )
						{
							eDutyType type = duty->mDuty[i].dutyType;
							if( type == eDUTY_NONE )
								break;

							/// 4. 의무조건 출력 라인 추가
							switch( type )
							{
							case eDUTY_HUNT:	totalLine++; break;
							case eDUTY_COLLECT:	totalLine++; break;
							case eDUTY_MEET:	totalLine++; break;
							case eDUTY_ACTIVESKILL:	totalLine++; break;
							case eDUTY_ENHANCE:	totalLine++; break;
							case eDUTY_PUTCARD:	totalLine++; break;
							case eDUTY_PVPJOIN:	totalLine++; break;
							case eDUTY_PVPWIN:	totalLine++; break;
							case eDUTY_PVPLOSE:	totalLine++; break;
							}

							/// 완료 체크용
							end = end << 8;
						}
					}
				}
			}
		}
	}

	/// 윈도우 사이즈 설정
	int w = mInitWidth + (START_GAPX * 2);
	int h = totalLine * rowHeight + (START_GAPY * 2);
	SetRelativeSize( cUISize(w, h) );
	
	int x = GetAbsoluteRect().mLeft;
	int y = GetAbsoluteRect().mTop;

	/// 이미지 설정
	if( mpBackImage[eBACKIMAGE_TOP] )
	{
		int skinHeight = mpBackSkin[eBACKIMAGE_TOP]->mSkinInfo->mHeight;
		mpBackImage[eBACKIMAGE_TOP]->SetScreenRect( x, y, w, skinHeight );
	}
	if( mpBackImage[eBACKIMAGE_CENTER] )
	{
		int backHeight = totalLine * rowHeight;
		mpBackImage[eBACKIMAGE_CENTER]->SetScreenRect( x + START_GAPX, y + START_GAPY, mInitWidth, backHeight );
	}
	if( mpBackImage[eBACKIMAGE_BOTTOM] )
	{
		int backHeight = totalLine * rowHeight;
		int skinHeight = mpBackSkin[eBACKIMAGE_BOTTOM]->mSkinInfo->mHeight;
		mpBackImage[eBACKIMAGE_BOTTOM]->SetScreenRect( x, y + backHeight + START_GAPY, w, skinHeight );
	}
	if( mpBackImage[eBACKIMAGE_LEFT] )
	{
		int backHeight = totalLine * rowHeight;
		int skinWidth = mpBackSkin[eBACKIMAGE_LEFT]->mSkinInfo->mWidth;
		mpBackImage[eBACKIMAGE_LEFT]->SetScreenRect( x, y + START_GAPY, skinWidth, backHeight );
	}
	if( mpBackImage[eBACKIMAGE_RIGHT] )
	{
		int backHeight = totalLine * rowHeight;
		int skinWidth = mpBackSkin[eBACKIMAGE_RIGHT]->mSkinInfo->mWidth;
		mpBackImage[eBACKIMAGE_RIGHT]->SetScreenRect( x + START_GAPX + mInitWidth, y + START_GAPY, skinWidth, backHeight );
	}

	/// 전체 캡션 지정
	mCaptionRect.Set( 0, 0, w, h );
}

void cQuestSimpleWindow::UpdateSelect()
{
	unsigned long questIndex = QUESTMAN->GetChangeQuestIndex();
	if( questIndex == 0 )
	{
		mShowSelect = false;
		mShowOn = false;
		return;
	}

	int startRow = FindStartRow( questIndex );
	if( startRow == -1 )
	{
		mShowSelect = false;
		mShowOn = false;
		return;
	}

	// 선택한 퀘스트인덱스를 매니저에 알림
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		mShowSelect = false;
		mShowOn = false;
		return;
	}

	mShowOn = ( define->mQuestInfo ) ? true : false;

	int rowCount = 1;
	cQuestLimit* limit = define->mQuestLimit;
	if( limit && limit->mTime > 0 )
		rowCount++;

	cQuestDuty* duty = define->mQuestDuty;
	if( duty )
	{
		for( unsigned int i = 0; i < MAX_DUTY; ++i )
		{
			eDutyType type = duty->mDuty[i].dutyType;
			if( type != eDUTY_NONE )
				rowCount++;
		}
	}

	// 이미지 처리
	int rowHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );

	cUIRect rc;
	rc.mLeft = GetAbsoluteRect().mLeft;
	rc.mRight = rc.mLeft + GetAbsoluteRect().GetWidth();
	rc.mTop = GetAbsoluteRect().mTop + START_GAPY + ( rowHeight * startRow );
	rc.mBottom = rc.mTop + ( rowHeight * rowCount );

	if( mpSelectImage )
	{
		mShowSelect = true;
		mpSelectImage->SetScreenXY( (unsigned short)rc.mLeft, (unsigned short)rc.mTop );
		mpSelectImage->SetScreenWH( (unsigned short)rc.GetWidth(), (unsigned short)rc.GetHeight() );
	}

	// 추적 이미지 처리
	if( mpOnSkin )
	{
		int tw = (short)mpOnSkin->mSkinInfo->mTexWidth;
		//int th = (short)mpOnSkin->mSkinInfo->mTexHeight;
		mpOnImage->SetScreenXY( rc.mRight - tw, rc.mTop );
	}
}


int cQuestSimpleWindow::FindStartRow( unsigned long questIndex )
{
	if( questIndex == 0 )
		return -1;

	if( mQuestMap.GetSize() == 0 )
		return -1;

	//int leftX  = GetAbsoluteRect().mLeft + START_GAPX;
	//int rightX = GetAbsoluteRect().mLeft + START_GAPX + mImageSize.mWidth;
	int row = 0;

	typedef tPointerList<void*> cProgressList;

	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( !data )
		{
			assert(0);
			return -1;
		}

		row++;

		if( data->mClose == false )
		{
			// 펼쳐져 있으면
			cProgressList::cIterator b = data->mQuestProgressList.Begin();
			cProgressList::cIterator bend = data->mQuestProgressList.End();
			for( ; b != bend; ++b )
			{
				sQuestProgress* progress = (sQuestProgress*)(*b);
				if( !progress )
				{
					assert(0);
					return -1;
				}

				int startRow = row;

				/// 시작 줄 번호 찾아서 리턴
				if( progress->mIndex == questIndex )
					return startRow;

				row++;

				/// 의무조건 라인수 추가
				cQuestDefine* define = progress->mDefine;
				cQuestDuty* duty = define->mQuestDuty;
				cQuestLimit* limit = define->mQuestLimit;
				if( limit && limit->mTime > 0 )
					row++;
	
				if( duty )
				{
					for( unsigned int i = 0; i < MAX_DUTY; ++i )
					{
						eDutyType type = duty->mDuty[i].dutyType;
						if( type != eDUTY_NONE )
							row++;
					}
				}
			}
		}
	}

	return -1;
}

void cQuestSimpleWindow::GetHideMapCategorySet( tHashSet<unsigned int>& mapSet )
{
	/// 출력
	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( !data )
		{
			assert(0);
			continue;
		}

		/// 닫혀 있으면 해당 맵 인덱스 얻기
		if( data->mClose == true )
		{
			if( data->mQuestProgressList.GetSize() > 0 )
			{
				//cProgressList::cIterator b = data->mQuestProgressList.Begin();
				sQuestProgress* progress = (sQuestProgress*)( data->mQuestProgressList.Front() );
				if( !progress )
				{
					assert(0);
					return;
				}	
						
				/// 퀘스트 의무조건출력
				if( progress->mDefine )
				{
					mapSet.Insert( progress->mDefine->mTakeMapIndex );
				}
			}
		}
	}
}

void cQuestSimpleWindow::SetHideMapCategory( unsigned long mapIndex )
{
	/// 출력
	cQuestMap::cIterator i = mQuestMap.Begin();
	cQuestMap::cIterator end = mQuestMap.End();
	for( ; i != end; ++i )
	{
		sQuestHaveData* data = (sQuestHaveData*)(i->mSecond);
		if( !data )
		{
			assert(0);
			continue;
		}

		/// 닫혀 있으면 해당 맵 인덱스 얻기
		if( data->mClose == false )
		{
			if( data->mQuestProgressList.GetSize() > 0 )
			{
				//cProgressList::cIterator b = data->mQuestProgressList.Begin();
				sQuestProgress* progress = (sQuestProgress*)( data->mQuestProgressList.Front() );
				if( !progress )
				{
					assert(0);
					return;
				}	

				/// 퀘스트 의무조건출력
				if( progress->mDefine )
				{
					if( progress->mDefine->mTakeMapIndex == mapIndex )
					{
						data->mClose = true;
						UpdateSize();
						return;
					}
				}
			}
		}
	}
}
