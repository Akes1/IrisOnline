#include "stdafx.h"
#include "TerrainPaintDialog.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "TerrainEditor.h"
#include "Engine/Terrain.h"
#include "Engine/TerrainNode.h"

IMPLEMENT_DYNAMIC(cTerrainPaintDialog, cDialog)

BEGIN_MESSAGE_MAP(cTerrainPaintDialog, cDialog)
	ON_BN_CLICKED(IDC_CHECK_TPAINT_VISIBILITY, OnCheckedVisibility)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_PICK, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_PAINT_ALPHA, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_BLUR_ALPHA, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_SHARPEN_ALPHA, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_COLOR, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_BLUR_COLOR, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_LIGHTER, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_DARKER, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_BRUSH, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_SAVE_PALETTE, OnClickedSavePalette)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_LOAD_PALETTE, OnClickedLoadPalette)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_REFRESH_TEX_BLEND_LIST, OnClickedRefreshTexBlendList)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_SET_DETAIL_TEXTURES, OnClickedSetDetailTextures)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_SET_DETAIL_TEXTURES_TO_ALL, OnClickedSetDetailTexturesToAll)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_0, OnClickedUnLoadTexture0)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_1, OnClickedUnLoadTexture1)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_2, OnClickedUnLoadTexture2)
	ON_LBN_SELCHANGE(IDC_LIST_TEXTURE_BLEND, OnSelchangeTextureBlend)
	ON_WM_HSCROLL()

	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_0_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_1_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_2_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_3_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_4_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_5_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_6_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_7_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_8_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_9_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_10_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_11_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_12_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_13_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_14_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_15_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_16_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_17_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_18_0, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_19_0, OnClickedTexIcon)

	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_0_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_1_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_2_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_3_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_4_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_5_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_6_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_7_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_8_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_9_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_10_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_11_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_12_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_13_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_14_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_15_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_16_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_17_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_18_1, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_19_1, OnClickedTexIcon)

	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_0_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_1_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_2_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_3_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_4_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_5_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_6_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_7_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_8_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_9_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_10_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_11_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_12_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_13_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_14_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_15_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_16_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_17_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_18_2, OnClickedTexIcon)
	ON_BN_CLICKED(IDC_BUTTON_TPAINT_TEX_19_2, OnClickedTexIcon)
END_MESSAGE_MAP()

cTerrainPaintDialog::cTerrainPaintDialog()
: mAlpha0( 0.0f )
, mAlpha1( 0.0f )
, mAlpha2( 0.0f )
{
	mTexBlendArray.Reserve( 256 );
}

cTerrainPaintDialog::~cTerrainPaintDialog()
{
}

void cTerrainPaintDialog::DoDataExchange( CDataExchange* dx )
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_TPAINT_PAINT_ALPHA, mDetailButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_BLUR_ALPHA, mBlurAlphaButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_SHARPEN_ALPHA, mShapenAlphaButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLOR, mColorButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_BLUR_COLOR, mBlurColorButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_LIGHTER, mLighterButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_DARKER, mDarkerButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_BRUSH, mBrushButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_PICK, mPickButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_SAVE_PALETTE, mSavePaletteButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_LOAD_PALETTE, mLoadPaletteButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_REFRESH_TEX_BLEND_LIST, mRefreshTexBlendList );
	DDX_Control( dx, IDC_BUTTON_TPAINT_SET_DETAIL_TEXTURES, mSetDetailTexturesToTilesButton );
	DDX_Control( dx, IDC_BUTTON_TPAINT_SET_DETAIL_TEXTURES_TO_ALL, mSetDetailTexturesToAllButton );

	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_0, mColorBox[0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_1, mColorBox[1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_2, mColorBox[2] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_3, mColorBox[3] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_4, mColorBox[4] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_5, mColorBox[5] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_6, mColorBox[6] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_7, mColorBox[7] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_8, mColorBox[8] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_9, mColorBox[9] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_10, mColorBox[10] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_11, mColorBox[11] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_12, mColorBox[12] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_13, mColorBox[13] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_14, mColorBox[14] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_15, mColorBox[15] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_16, mColorBox[16] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_17, mColorBox[17] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_18, mColorBox[18] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_19, mColorBox[19] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_20, mColorBox[20] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_21, mColorBox[21] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_22, mColorBox[22] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_COLORBOX_23, mColorBox[23] );

	DDX_Control( dx, IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_0, mDeleteTextureButton[0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_1, mDeleteTextureButton[1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_UNLOAD_TEXTURE_2, mDeleteTextureButton[2] );

	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_0_0, mTexIcon[0][0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_1_0, mTexIcon[0][1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_2_0, mTexIcon[0][2] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_3_0, mTexIcon[0][3] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_4_0, mTexIcon[0][4] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_5_0, mTexIcon[0][5] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_6_0, mTexIcon[0][6] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_7_0, mTexIcon[0][7] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_8_0, mTexIcon[0][8] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_9_0, mTexIcon[0][9] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_10_0, mTexIcon[0][10] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_11_0, mTexIcon[0][11] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_12_0, mTexIcon[0][12] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_13_0, mTexIcon[0][13] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_14_0, mTexIcon[0][14] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_15_0, mTexIcon[0][15] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_16_0, mTexIcon[0][16] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_17_0, mTexIcon[0][17] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_18_0, mTexIcon[0][18] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_19_0, mTexIcon[0][19] );

	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_0_1, mTexIcon[1][0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_1_1, mTexIcon[1][1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_2_1, mTexIcon[1][2] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_3_1, mTexIcon[1][3] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_4_1, mTexIcon[1][4] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_5_1, mTexIcon[1][5] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_6_1, mTexIcon[1][6] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_7_1, mTexIcon[1][7] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_8_1, mTexIcon[1][8] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_9_1, mTexIcon[1][9] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_10_1, mTexIcon[1][10] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_11_1, mTexIcon[1][11] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_12_1, mTexIcon[1][12] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_13_1, mTexIcon[1][13] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_14_1, mTexIcon[1][14] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_15_1, mTexIcon[1][15] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_16_1, mTexIcon[1][16] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_17_1, mTexIcon[1][17] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_18_1, mTexIcon[1][18] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_19_1, mTexIcon[1][19] );

	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_0_2, mTexIcon[2][0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_1_2, mTexIcon[2][1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_2_2, mTexIcon[2][2] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_3_2, mTexIcon[2][3] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_4_2, mTexIcon[2][4] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_5_2, mTexIcon[2][5] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_6_2, mTexIcon[2][6] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_7_2, mTexIcon[2][7] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_8_2, mTexIcon[2][8] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_9_2, mTexIcon[2][9] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_10_2, mTexIcon[2][10] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_11_2, mTexIcon[2][11] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_12_2, mTexIcon[2][12] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_13_2, mTexIcon[2][13] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_14_2, mTexIcon[2][14] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_15_2, mTexIcon[2][15] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_16_2, mTexIcon[2][16] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_17_2, mTexIcon[2][17] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_18_2, mTexIcon[2][18] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_TEX_19_2, mTexIcon[2][19] );

	DDX_Control( dx, IDC_BUTTON_TPAINT_DETAIL_0, mDetailIcon[0] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_DETAIL_1, mDetailIcon[1] );
	DDX_Control( dx, IDC_BUTTON_TPAINT_DETAIL_2, mDetailIcon[2] );
}

void cTerrainPaintDialog::SetCheckedButton( int id )
{
	mGroup.SetCheckedButton( id );
}

void cTerrainPaintDialog::SelectTextureIcon( int i0, int i1, int i2 )
{
	if( i0 > 0 )
		mTexIcon[0][i0-1].SetChecked( true );
	else if( i0 == 0 )
		mTexIconGroup[0].SetCheckedButton( 0 );

	if( i1 > 0 )
		mTexIcon[1][i1-1].SetChecked( true );
	else if( i1 == 0 )
		mTexIconGroup[1].SetCheckedButton( 0 );

	if( i2 > 0 )
		mTexIcon[2][i2-1].SetChecked( true );
	else if( i2 == 0 )
		mTexIconGroup[2].SetCheckedButton( 0 );
}

void cTerrainPaintDialog::SelectTextureImage( int i0, int i1, int i2 )
{
	cTerrainTexture* tex0 = TERRAIN->GetTexture( i0 );
	cTerrainTexture* tex1 = TERRAIN->GetTexture( i1 );
	cTerrainTexture* tex2 = TERRAIN->GetTexture( i2 );

	if( i0 > 0 && tex0 )
		mDetailIcon[0].SetImage( tex0->GetBitmap(), tex0->GetWidth(), tex0->GetHeight() );
	else
		mDetailIcon[0].SetImage( 0, 0, 0 );

	if( i1 > 0 && tex1 )
		mDetailIcon[1].SetImage( tex1->GetBitmap(), tex1->GetWidth(), tex1->GetHeight() );
	else
		mDetailIcon[1].SetImage( 0, 0, 0 );

	if( i2 > 0 && tex2 )
		mDetailIcon[2].SetImage( tex2->GetBitmap(), tex2->GetWidth(), tex2->GetHeight() );
	else
		mDetailIcon[2].SetImage( 0, 0, 0 );
}

void cTerrainPaintDialog::SelectTextureName( int i0, int i1, int i2 )
{
	const char* texName0 = 0;
	const char* texName1 = 0;
	const char* texName2 = 0;
	cTerrainTexture* tex0 = TERRAIN->GetTexture( i0 );
	cTerrainTexture* tex1 = TERRAIN->GetTexture( i1 );
	cTerrainTexture* tex2 = TERRAIN->GetTexture( i2 );

	if( i0 > 0 && tex0 )
		texName0 = tex0->GetFileName();
	else
		texName0 = "?";

	if( i1 > 0 && tex1 )
		texName1 = tex1->GetFileName();
	else
		texName1 = "?";

	if( i2 > 0 && tex2 )
		texName2 = tex2->GetFileName();
	else
		texName2 = "?";

	SetDlgItemText( IDC_EDIT_TPAINT_DETAIL_NAME_0, texName0 );
	SetDlgItemText( IDC_EDIT_TPAINT_DETAIL_NAME_1, texName1 );
	SetDlgItemText( IDC_EDIT_TPAINT_DETAIL_NAME_2, texName2 );
}

void cTerrainPaintDialog::SelectTextureBlend( int i0, int i1, int i2 )
{
	bool found = false;

	for( unsigned int i = 0, iend = mTexBlendArray.GetSize(); i < iend; ++i )
	{
		cTexBlend& texBlend = mTexBlendArray[i];

		if( texBlend.mIndex0 == i0 && texBlend.mIndex1 == i1 && texBlend.mIndex2 == i2 )
		{
			found = true;
			CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TEXTURE_BLEND );
			list->SetCurSel( i );
			break;
		}
	}

	if( found == false )
	{
		CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TEXTURE_BLEND );
		list->SetCurSel( -1 );
	}
}

void cTerrainPaintDialog::UpdateInnerRadius( float radius )
{
	cString str;
	str.Format( "%.1f", radius );
	int innerRadius = int(radius * 10.0f);
	CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_INNER_RADIUS);
	CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_OUTER_RADIUS);

	if( innerRadius > outer->GetPos() )
	{
		outer->SetPos( innerRadius );
		SetDlgItemText( IDC_EDIT_TPAINT_OUTER_RADIUS, str.Cstr() );
	}
	inner->SetPos( innerRadius );
	SetDlgItemText( IDC_EDIT_TPAINT_INNER_RADIUS, str.Cstr() );
}

void cTerrainPaintDialog::UpdateOuterRadius( float radius )
{
	cString str;
	str.Format( "%.1f", radius );
	int outerRadius = int(radius * 10.0f);
	CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_INNER_RADIUS);
	CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_OUTER_RADIUS);

	if( outerRadius < inner->GetPos() )
	{
		inner->SetPos( outerRadius );
		SetDlgItemText( IDC_EDIT_TPAINT_INNER_RADIUS, str.Cstr() );
	}
	outer->SetPos( outerRadius );
	SetDlgItemText( IDC_EDIT_TPAINT_OUTER_RADIUS, str.Cstr() );
}

void cTerrainPaintDialog::UpdateTileGroup( cTileGroup* group )
{
	assert( group );

	int i0 = group->GetTextureIndex0();
	int i1 = group->GetTextureIndex1();
	int i2 = group->GetTextureIndex2();

	/// 텍스처 이미지를 선택
	SelectTextureImage( i0, i1, i2 );

	/// 텍스처 아이콘을 선택
	SelectTextureIcon( i0, i1, i2 );

	/// 텍스처 이름을 선택
	SelectTextureName( i0, i1, i2 );

	/// 텍스처 조합을 선택
	SelectTextureBlend( i0, i1, i2 );
}

void cTerrainPaintDialog::UpdateColor( unsigned char red, unsigned char green, unsigned char blue )
{
	int i = mColorBoxGroup.GetCheckedBoxIndex();
	assert( i >= 0 );

	mColorBox[i].SetColor( RGB(red, green, blue) );
}

void cTerrainPaintDialog::UpdateTextures()
{
	for( unsigned int i = 1; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		cTerrainTexture* tex = TERRAIN->GetTexture( i );

		if( tex )
		{
			mTexIcon[0][i-1].SetImage( tex->GetBitmap(), tex->GetWidth(), tex->GetHeight() );
			mTexIcon[1][i-1].SetImage( tex->GetBitmap(), tex->GetWidth(), tex->GetHeight() );
			mTexIcon[2][i-1].SetImage( tex->GetBitmap(), tex->GetWidth(), tex->GetHeight() );
		}
		else
		{
			mTexIcon[0][i-1].SetImage( 0, 0, 0 );
			mTexIcon[1][i-1].SetImage( 0, 0, 0 );
			mTexIcon[2][i-1].SetImage( 0, 0, 0 );
		}
	}

	/// 맵에 디테일 정보를 추가
	mTexBlendMap.Clear();
	mTexBlendArray.Clear();

	tArray<cTerrainLeafNode*>& nodeArray = TERRAIN->GetLeafNodeArray();

	for( unsigned int i = 0, iend = nodeArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = nodeArray[i];

		unsigned int value = n->GetTextureValue1();
		int i0 = n->GetTextureIndex0();
		int i1 = n->GetTextureIndex1();
		int i2 = n->GetTextureIndex2();

		mTexBlendMap.Insert( value, cTexBlend(i0, i1, i2) );
	}

	/// 리스트
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TEXTURE_BLEND );
	list->ResetContent();

	cString str;

	{
		cTexBlendMap::cIterator i = mTexBlendMap.Begin();
		cTexBlendMap::cIterator iend = mTexBlendMap.End();

		cTexBlend& tex = i->mSecond;
		++i;
		if( tex.mIndex0 != 0 || tex.mIndex1 != 0 || tex.mIndex2 != 0 )
		{
			str.Format( "tex %2d - tex %2d - tex %2d", tex.mIndex0, tex.mIndex1, tex.mIndex2 );
			list->AddString( str.Cstr() );

			mTexBlendArray.PushBack( cTexBlend(tex.mIndex0, tex.mIndex1, tex.mIndex2) );
		}

		for( ; i != iend; ++i )
		{
			cTexBlend& tex = i->mSecond;

			str.Format( "tex %2d - tex %2d - tex %2d", tex.mIndex0, tex.mIndex1, tex.mIndex2 );
			list->AddString( str.Cstr() );

			mTexBlendArray.PushBack( cTexBlend(tex.mIndex0, tex.mIndex1, tex.mIndex2) );
		}
	}

	///
	int i0 = mTexIconGroup[0].GetCheckedButtonIndex() + 1;
	int i1 = mTexIconGroup[1].GetCheckedButtonIndex() + 1;
	int i2 = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	/// 텍스처 이미지를 선택
	SelectTextureImage( i0, i1, i2 );

	/// 텍스처 이름을 선택
	SelectTextureName( i0, i1, i2 );

	/// 텍스처 조합을 선택
	SelectTextureBlend( i0, i1, i2 );
}

void cTerrainPaintDialog::IncreaseAlpha0()
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 );

	int pos = slider->GetPos();
	
	if( pos == 100 )
		return;
	if( (pos += 10) > 100 )
		pos = 100;

	slider->SetPos( pos );
	OnHScroll( 0, 0, (CScrollBar*)slider );
}

void cTerrainPaintDialog::IncreaseAlpha1()
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 );

	int pos = slider->GetPos();

	if( pos == 100 )
		return;
	if( (pos += 10) > 100 )
		pos = 100;

	slider->SetPos( pos );
	OnHScroll( 0, 0, (CScrollBar*)slider );
}

void cTerrainPaintDialog::IncreaseAlpha2()
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 );

	int pos = slider->GetPos();

	if( pos == 100 )
		return;
	if( (pos += 10) > 100 )
		pos = 100;

	slider->SetPos( pos );
	OnHScroll( 0, 0, (CScrollBar*)slider );
}

bool cTerrainPaintDialog::LoadPalette( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
		return false;

	for( unsigned int i = 0; i < 24; ++i )
	{
		COLORREF color = 0;

		loader.Read( &color, sizeof(COLORREF) );

		mColorBox[i].SetColor( color );
	}
	return true;
}

bool cTerrainPaintDialog::SavePalette( const cString& pathName )
{
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
		return false;

	for( unsigned int i = 0; i < 24; ++i )
	{
		COLORREF color = mColorBox[i].GetColor();

		saver.Write( &color, sizeof(COLORREF) );
	}

	cString fileName;
	GetFileName( &fileName, pathName );
	MAIN->SetPaletteFileName( fileName );
	return true;
}

BOOL cTerrainPaintDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mDetailButton.SetGroup( &mGroup );
	mBlurAlphaButton.SetGroup( &mGroup );
	mShapenAlphaButton.SetGroup( &mGroup );
	mColorButton.SetGroup( &mGroup );
	mBlurColorButton.SetGroup( &mGroup );
	mBrushButton.SetGroup( &mGroup );
	mLighterButton.SetGroup( &mGroup );
	mDarkerButton.SetGroup( &mGroup );
	mPickButton.SetGroup( &mGroup );

	for( unsigned int i = 0; i < 24; ++i )
	{
		mColorBox[i].SetGroup( &mColorBoxGroup );
	}
	mColorBox[0].SetChecked( true );

	for( unsigned int i = 0; i < 20; ++i )
	{
		mTexIcon[0][i].SetGroup( &(mTexIconGroup[0]) );
		mTexIcon[1][i].SetGroup( &(mTexIconGroup[1]) );
		mTexIcon[2][i].SetGroup( &(mTexIconGroup[2]) );
	}

	mDetailIcon[0].SetCheckEnabled( false );
	mDetailIcon[1].SetCheckEnabled( false );
	mDetailIcon[2].SetCheckEnabled( false );

	/// 불투명도
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_OPACITY );
	slider->SetRange( 1, 100 );
	slider->SetPos( 10 );
	SetDlgItemInt( IDC_EDIT_TPAINT_OPACITY, slider->GetPos() );

	/// 안쪽 반지름
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_INNER_RADIUS );
	slider->SetRange( 5, 3000 );
	slider->SetPos( 10 );
	SetDlgItemText( IDC_EDIT_TPAINT_INNER_RADIUS, "1.0" );

	/// 바깥쪽 반지름
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_OUTER_RADIUS );
	slider->SetRange( 5, 3000 );
	slider->SetPos( 30 );
	SetDlgItemText( IDC_EDIT_TPAINT_OUTER_RADIUS, "3.0" );

	/// 디테일 알파
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 );
	slider->SetRange( 0, 100 );
	slider->SetPos( 100 );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 );
	slider->SetRange( 0, 100 );
	slider->SetPos( 0 );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 );
	slider->SetRange( 0, 100 );
	slider->SetPos( 0 );

	mAlpha0 = 100.0f;
	mAlpha1 = 0.0f;
	mAlpha2 = 0.0f;

	/// 블러 비율
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_BLUR_RATIO );
	slider->SetRange( 10, 100 );
	slider->SetPos( 50 );
	SetDlgItemInt( IDC_EDIT_TPAINT_BLUR_RATIO, slider->GetPos() );
	return TRUE;
}

void cTerrainPaintDialog::OnCheckedVisibility()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_TPAINT_VISIBILITY );
	TERRAINEDIT->SetVisible( check->GetCheck() != 0 );
}

void cTerrainPaintDialog::OnClickedButton()
{
	TERRAINEDIT->SetPickPosChanged( true );
	VIEW->SetEditMode( eEDIT_TERRAIN_PAINT );
}

void cTerrainPaintDialog::OnClickedTexIcon()
{
	int i0 = mTexIconGroup[0].GetCheckedButtonIndex() + 1;
	int i1 = mTexIconGroup[1].GetCheckedButtonIndex() + 1;
	int i2 = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	/// 텍스처 이미지를 선택
	SelectTextureImage( i0, i1, i2 );

	/// 텍스처 이름을 선택
	SelectTextureName( i0, i1, i2 );

	/// 텍스처 조합을 선택
	SelectTextureBlend( i0, i1, i2 );
}

void cTerrainPaintDialog::OnClickedLoadPalette()
{
	CFileDialog dlg(
		TRUE, "Load Palette", "*.pal",
		OFN_HIDEREADONLY,
		"Palette Files (*.pal)| *.pal| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( LoadPalette( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Load Palette", "Palette" );
		}
	}
}

void cTerrainPaintDialog::OnClickedSavePalette()
{
	CFileDialog dlg(
		FALSE, "Save Palette", "*.pal",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Palette Files (*.pal)| *.pal| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( SavePalette( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Palette", "Palette" );
		}
	}
}

void cTerrainPaintDialog::OnClickedRefreshTexBlendList()
{
	UpdateTextures();
}

void cTerrainPaintDialog::OnClickedSetDetailTextures()
{
	int i0 = mTexIconGroup[0].GetCheckedButtonIndex() + 1;
	int i1 = mTexIconGroup[1].GetCheckedButtonIndex() + 1;
	int i2 = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	if( i0 <= 0 || i1 <= 0 || i2 <= 0 )
	{
		MessageBox( "Select Detail Textures~", "Terrain" );
		return;
	}

	TERRAINEDIT->SetDetailTextures( i0, i1, i2 );
}

void cTerrainPaintDialog::OnClickedSetDetailTexturesToAll()
{
	int i0 = mTexIconGroup[0].GetCheckedButtonIndex() + 1;
	int i1 = mTexIconGroup[1].GetCheckedButtonIndex() + 1;
	int i2 = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	if( i0 <= 0 || i1 <= 0 || i2 <= 0 )
	{
		MessageBox( "Select Detail Textures~", "Terrain" );
		return;
	}

	if( MessageBox( "Are you sure?", "Terrain", MB_YESNO ) != IDYES )
		return;

	TERRAINEDIT->SetDetailTexturesToAll( i0, i1, i2 );
}

void cTerrainPaintDialog::OnClickedUnLoadTexture0()
{
	int i = mTexIconGroup[0].GetCheckedButtonIndex() + 1;

	if( i <= 0 )
		return;

	if( MessageBox( "Are you sure?", "Terrain", MB_YESNO ) != IDYES )
		return;

	TERRAINEDIT->UnLoadTexture( i );
}

void cTerrainPaintDialog::OnClickedUnLoadTexture1()
{
	int i = mTexIconGroup[1].GetCheckedButtonIndex() + 1;

	if( i <= 0 )
		return;

	if( MessageBox( "Are you sure?", "Terrain", MB_YESNO ) != IDYES )
		return;

	TERRAINEDIT->UnLoadTexture( i );
}

void cTerrainPaintDialog::OnClickedUnLoadTexture2()
{
	int i = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	if( i <= 0 )
		return;

	if( MessageBox( "Are you sure?", "Terrain", MB_YESNO ) != IDYES )
		return;

	TERRAINEDIT->UnLoadTexture( i );
}

void cTerrainPaintDialog::OnSelchangeTextureBlend()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TEXTURE_BLEND );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		i = 0;

	assert( (unsigned int)i < mTexBlendArray.GetSize() );

	cTexBlend& texBlend = mTexBlendArray[i];

	int i0 = texBlend.mIndex0;
	int i1 = texBlend.mIndex1;
	int i2 = texBlend.mIndex2;

	/// 텍스처 이미지를 선택
	SelectTextureImage( i0, i1, i2 );

	/// 텍스처 아이콘을 선택
	SelectTextureIcon( i0, i1, i2 );

	/// 텍스처 이름을 선택
	SelectTextureName( i0, i1, i2 );
}

void cTerrainPaintDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_TPAINT_OPACITY:
		{
			int pos = slider->GetPos();
			SetDlgItemInt( IDC_EDIT_TPAINT_OPACITY, pos );
		}
		break;
	case IDC_SLIDER_TPAINT_INNER_RADIUS:
		{
			CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_OUTER_RADIUS);
			int innerRadius = slider->GetPos();
			int outerRadius = outer->GetPos();
			cString str;
			str.Format( "%.1f", innerRadius / 10.0f );

			if( innerRadius > outerRadius )
			{
				outer->SetPos( innerRadius );
				SetDlgItemText( IDC_EDIT_TPAINT_OUTER_RADIUS, str.Cstr() );
			}
			SetDlgItemText( IDC_EDIT_TPAINT_INNER_RADIUS, str.Cstr() );

			TERRAINEDIT->SetPickPosChanged( true );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_TPAINT_OUTER_RADIUS:
		{
			CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TPAINT_INNER_RADIUS);
			int innerRadius = inner->GetPos();
			int outerRadius = slider->GetPos();
			cString str;
			str.Format( "%.1f", outerRadius / 10.0f );

			if( outerRadius < innerRadius )
			{
				inner->SetPos( outerRadius );
				SetDlgItemText( IDC_EDIT_TPAINT_INNER_RADIUS, str.Cstr() );
			}
			SetDlgItemText( IDC_EDIT_TPAINT_OUTER_RADIUS, str.Cstr() );

			TERRAINEDIT->SetPickPosChanged( true );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_TPAINT_BLUR_RATIO:
		SetDlgItemInt( IDC_EDIT_TPAINT_BLUR_RATIO, slider->GetPos() );
		break;
	case IDC_SLIDER_TPAINT_ALPHA_0:
		{
			mAlpha0 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 ))->GetPos();
			mAlpha1 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 ))->GetPos();
			mAlpha2 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 ))->GetPos();

			float d = (100.0f - mAlpha0 - mAlpha1 - mAlpha2) / 2.0f;
			mAlpha1 += d;
			mAlpha2 += d;

			if( mAlpha1 < 0.0f )
			{
				mAlpha2 += mAlpha1;
				mAlpha1 = 0.0f;
			}
			if( mAlpha2 < 0.0f )
			{
				mAlpha1 += mAlpha2;
				mAlpha2 = 0.0f;
			}
			if( mAlpha0 >= 99.0f )
			{
				mAlpha0 = 100.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha1 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 100.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha2 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 100.0f;
			}

			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 ))->SetPos( (int)mAlpha1 );
			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 ))->SetPos( (int)mAlpha2 );

			assert( mAlpha0 >= 0.0f && mAlpha1 >= 0.0f && mAlpha2 >= 0.0f );
			assert( mAlpha0 + mAlpha1 + mAlpha2 > 99.9f );
		}
		break;
	case IDC_SLIDER_TPAINT_ALPHA_1:
		{
			mAlpha0 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 ))->GetPos();
			mAlpha1 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 ))->GetPos();
			mAlpha2 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 ))->GetPos();

			float d = (100.0f - mAlpha0 - mAlpha1 - mAlpha2) / 2.0f;
			mAlpha0 += d;
			mAlpha2 += d;

			if( mAlpha0 < 0.0f )
			{
				mAlpha2 += mAlpha0;
				mAlpha0 = 0.0f;
			}
			if( mAlpha2 < 0.0f )
			{
				mAlpha0 += mAlpha2;
				mAlpha2 = 0.0f;
			}
			if( mAlpha0 >= 99.0f )
			{
				mAlpha0 = 100.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha1 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 100.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha2 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 100.0f;
			}

			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 ))->SetPos( (int)mAlpha0 );
			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 ))->SetPos( (int)mAlpha2 );

			assert( mAlpha0 >= 0.0f && mAlpha1 >= 0.0f && mAlpha2 >= 0.0f );
			assert( mAlpha0 + mAlpha1 + mAlpha2 > 99.9f );
		}
		break;
	case IDC_SLIDER_TPAINT_ALPHA_2:
		{
			mAlpha0 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 ))->GetPos();
			mAlpha1 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 ))->GetPos();
			mAlpha2 = (float)((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_2 ))->GetPos();

			float d = (100.0f - mAlpha0 - mAlpha1 - mAlpha2) / 2.0f;
			mAlpha0 += d;
			mAlpha1 += d;

			if( mAlpha0 < 0.0f )
			{
				mAlpha1 += mAlpha0;
				mAlpha0 = 0.0f;
			}
			if( mAlpha1 < 0.0f )
			{
				mAlpha0 += mAlpha1;
				mAlpha1 = 0.0f;
			}
			if( mAlpha0 >= 99.0f )
			{
				mAlpha0 = 100.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha1 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 100.0f;
				mAlpha2 = 0.0f;
			}
			if( mAlpha2 >= 99.0f )
			{
				mAlpha0 = 0.0f;
				mAlpha1 = 0.0f;
				mAlpha2 = 100.0f;
			}

			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_0 ))->SetPos( (int)mAlpha0 );
			((CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_ALPHA_1 ))->SetPos( (int)mAlpha1 );

			assert( mAlpha0 >= 0.0f && mAlpha1 >= 0.0f && mAlpha2 >= 0.0f );
			assert( mAlpha0 + mAlpha1 + mAlpha2 > 99.9f );
		}
		break;
	}
}
