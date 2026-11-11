#include "stdafx.h"
#include "NaviFieldEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "NaviFieldBuildDialog.h"
#include "NaviFieldPaintDialog.h"
#include "NaviFieldInfoDialog.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/Ray.h"
#include "Engine/NaviField.h"
#include "Engine/PathFinder.h"

#include "Doing/DoingManager.h"

cNaviFieldEditor* cNaviFieldEditor::mSingleton = 0;

cNaviFieldEditor::cNaviFieldEditor()
: mPaintDialog( 0 )
, mInfoDialog( 0 )
, mDrawing( false )
, mPickPos( NiPoint3::ZERO )
, mPickOldPos( NiPoint3::ZERO )
, mPickPosChanged( false )
, mOuterLines( NiColorA(1.0f, 1.0f, 0.0f), NiColorA(0.0f, 0.0f, 0.0f) )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	/// 네비필드를 생성
	mNaviField = new cNaviField;

	/// 길찾기를 생성
	mPathFinder = new cPathFinder( 4096 );

	///
	mPickedArray.Reserve( 1024 );
}

cNaviFieldEditor::~cNaviFieldEditor()
{
	delete mNaviField;
	delete mPathFinder;

	mSingleton = 0;
}

void cNaviFieldEditor::Clear()
{
	mDrawing = false;
	mPickOldPos = mPickPos = NiPoint3::ZERO;
	mPickPosChanged = false;
	mPathName.Clear();
}

void cNaviFieldEditor::Process()
{
	mNaviField->Process();

	if( mPickPos != NiPoint3::ZERO )
	{
		if( mPickPosChanged )
		{
			mPickPosChanged = false;

			float or = float(mPaintDialog->GetRadius() * mNaviField->GetUnitsPerMeter());
			mOuterLines.Process( mPickPos, or, mNaviField );
		}
	}
}

void cNaviFieldEditor::Render()
{
	mNaviField->Render();
	mOuterLines.Render();
}

void cNaviFieldEditor::Init( unsigned int resolution, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();
	mNaviField->Init( resolution, metersPerVertex, unitsPerMeter );
	mPathFinder->Init( mNaviField );

	/// 실행 기록을 지움
	DOINGMAN->ClearNaviField();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	MAIN->SetNaviFieldModified( false );

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->UpdateNaviField( resolution, mNaviField->GetLeafGridSize() - 1, metersPerVertex, unitsPerMeter );
}

bool cNaviFieldEditor::Load( const cString& pathName )
{
	Clear();

	/// 실행 기록을 지움
	DOINGMAN->ClearNaviField();

	if( mNaviField == 0 )
	{
		assert( 0 && "null navifield" );
		return false;
	}
	
	/// 네비필드를 로딩
	cString str;

	switch( mNaviField->Load( pathName ) )
	{
	case NAVIFIELD_LOAD_ERROR_OPEN:
		str = "Failed to open navifield file.";
		break;
	case NAVIFIELD_LOAD_ERROR_FILE_HEADER:
		str = "Failed to load navifield file header.";
		break;
	case NAVIFIELD_LOAD_ERROR_FILE_TYPE:
		str = "Invalid navifield file type.";
		break;
	case NAVIFIELD_LOAD_ERROR_FILE_VERSION:
		str = "Invalid navifield file version.";
		break;
	case NAVIFIELD_LOAD_ERROR_GRID_SIZE:
		str = "Invalid navifield grid size.";
		break;
	case NAVIFIELD_LOAD_ERROR_TEXTURE_NAME:
		str = "Failed to load navifield texture name.";
		break;
	case NAVIFIELD_LOAD_ERROR_TEXTURE:
		str = "Failed to load navifield texture.";
		break;
	case NAVIFIELD_LOAD_ERROR_NODE:
		str = "Failed to load navifield node.";
		break;
	}

	if( str.IsEmpty() == false )
	{
		AfxMessageBox( str.Cstr() );
		return false;
	}

	/// 길찾기 맵을 갱신
	mPathFinder->Init( mNaviField );

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetNaviFieldModified( name, false );

	/// 뷰를 갱신
	VIEW->Update();

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNaviField( mNaviField->GetCellCount(), mNaviField->GetLeafGridSize() - 1, mNaviField->GetMetersPerVertex(), mNaviField->GetUnitsPerMeter() );
	return true;
}

bool cNaviFieldEditor::Save()
{
	if( mPathName.IsEmpty() )
		return false;
	else
		return SaveAs( mPathName );
}

bool cNaviFieldEditor::SaveAs( const cString& pathName )
{
	if( mNaviField == 0 )
	{
		assert( 0 && "null navifield" );
		return false;
	}
	
	if( mNaviField->Save( pathName ) )
	{
		/// 경로 이름을 설정
		mPathName = pathName;

		cString name;
		::GetFileName( &name, pathName );
		MAIN->SetNaviFieldModified( name, false );
		return true;
	}
	return false;
}

void cNaviFieldEditor::SyncAllToNaviMesh()
{
	/// 실행 기록을 지움
	DOINGMAN->ClearNaviField();

	/// 지형과 동기화
	mNaviField->SyncAllToNaviMesh();

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetNaviFieldModified( true );
}

bool cNaviFieldEditor::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius )
{
	float x = pos.x;
	float y = pos.y;
	float unitsPerVertex = mNaviField->GetUnitsPerVertex();
	unsigned int gridSize = mNaviField->GetCellCount();

	int xb = (int)((x - radius) / unitsPerVertex);
	int yb = (int)((y - radius) / unitsPerVertex);
	int xe = (int)((x + radius) / unitsPerVertex);
	int ye = (int)((y + radius) / unitsPerVertex);

	--xb;
	--yb;
	++xe;
	++ye;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe > (int)gridSize )
		xe = gridSize;
	if( ye > (int)gridSize )
		ye = gridSize;

	if( xb >= xe || yb >= ye )
	{
		return false;
	}

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
	return true;
}

unsigned int cNaviFieldEditor::GetResolution() const
{
	return mNaviField->GetCellCount();
}

float cNaviFieldEditor::GetMetersPerVertex() const
{
	return mNaviField->GetMetersPerVertex();
}

void cNaviFieldEditor::OnTimer_Paint()
{
	if( mDrawing || mPickOldPos != mPickPos )
		mPickPosChanged = true;

	if( mDrawing && mPickPos != NiPoint3::ZERO )
	{
		switch( mPaintDialog->GetCheckedButton() )
		{
		case 0:
			return;
		case IDC_BUTTON_NPAINT_COLOR:
			{
				float radius = mPaintDialog->GetRadius() * 100.0f;
				COLORREF color = mPaintDialog->GetColor();
				float red = GetRValue( color ) / 255.0f;
				float green = GetGValue( color ) / 255.0f;
				float blue = GetBValue( color ) / 255.0f;
				unsigned char value = mPaintDialog->GetValue();

				mNaviField->Color( mPickPos, radius, red, green, blue, value );
			}
			break;
		case IDC_BUTTON_NPAINT_PICK:
			break;
		default:
			assert( 0 && "invalid navifield paint type" );
			break;
		}

		/// 변경 여부를 설정
		MAIN->SetNaviFieldModified( mNaviField->IsModified() );
	}
}

void cNaviFieldEditor::OnMouseMove( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_R) )
		{
			float r = mPaintDialog->GetRadius();
			r += float((point.x - mMousePos.x) - (point.y - mMousePos.y)) / 10.0f;

			if( r < 0.5f )
				r = 0.5f;
			else if( r > 50.0f )
				r = 50.0f;

			mPickPosChanged = true;
			mMousePos = point;
			mPaintDialog->UpdateRadius( r );
			VIEW->Update();
			return;
		}
	}

	NiPoint3 pos;

	if( mNaviField->Pick( &pos, point.x, point.y ))
	{
		mPickOldPos = mPickPos;
		mPickPos = pos;
	}
	else
	{
		mDrawing = false;
		mPickOldPos = mPickPos = NiPoint3::ZERO;
	}

	mMousePos = point;
}

void cNaviFieldEditor::OnLButtonDown_Paint( CPoint point )
{
	if( KEY->IsDown(KEY_R) )
	{
		mMousePos = point;
		return;
	}

	NiPoint3 pos;

	if( mNaviField->Pick( &pos, point.x, point.y ) )
	{
		mDrawing = true;
		mPickOldPos = mPickPos;
		mPickPos = pos;

		switch( mPaintDialog->GetCheckedButton() )
		{
		case IDC_BUTTON_NPAINT_PICK:
			{
				/// 해당 지점의 지형 색과 플랙값을 픽킹
				unsigned char value;

				if( mNaviField->GetValue( &value, pos ) )
				{
					mPaintDialog->SetValue( value );
				}
			}
			break;
		}
	}
	else
	{
		mDrawing = false;
		mPickPos = NiPoint3::ZERO;
	}
}

void cNaviFieldEditor::OnLButtonUp()
{
	mDrawing = false;
}
