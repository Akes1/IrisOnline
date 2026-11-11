#include "StdAfx.h"
#include "UINodeData.h"

#include "UISkinLexer.h"
#include "UIManager.h"
#include "Parser.h"
#include "UIRect.h"


cUINodeSkin::cUINodeSkin( eUINodeType type )
: mType( type )
, mpTexture( 0 )
, mSkinInfo(0)
{
	mSkinInfo = new sSkinInfo;

	mColor = NiColorA::WHITE;

	mDefaultColor = COLOR_WHITE;
}

cUINodeSkin::~cUINodeSkin()
{
	SAFE_DELETE( mSkinInfo );
}

/// 주의사항 : 각 윈도우나 자식들의 속성값을 읽어들인다
bool cUINodeSkin::ParseLine( cParser& parser, cToken& token )
{
	switch( token.mType )
	{
	case eTOKEN_IMAGEINDEX:
		{
			int i = parser.ParseInt();
			mpTexture = UIMAN->GetSkin()->GetTexture( i );

			/// 해당 인덱스의 텍스쳐가 존재하지 않으면,
			if( !mpTexture )
			{
				assert( 0 && "error!! not exist image index" );
				return false;
			}
		}
		break;
	case eTOKEN_POS:
		{
			mSkinInfo->mX = parser.ParseInt();
			mSkinInfo->mY = parser.ParseInt();
		}
		break;
	case eTOKEN_SIZE:
		{
			mSkinInfo->mWidth = mSkinInfo->mTexWidth = parser.ParseInt();
			mSkinInfo->mHeight = mSkinInfo->mTexHeight = parser.ParseInt();
		}
		break;
	case eTOKEN_TEXPOS:
		{
			mSkinInfo->mTexX = parser.ParseInt();
			mSkinInfo->mTexY = parser.ParseInt();
		}
		break;
	case eTOKEN_TEXSIZE:
		{
			// 070424 현재는 필요없는 키워드. KKR
			// UIImage 자체가 확대, 축소기능이 없다.
			mSkinInfo->mTexWidth = parser.ParseInt();
			mSkinInfo->mTexHeight = parser.ParseInt();
		}
		break;
	case eTOKEN_HOVERTIPINDEX:
		{
			mSkinInfo->mHoverTipIndex = parser.ParseInt();
		}
		break;
	default:
		TCHAR buffer[ 80 ];
		_stprintf_s( buffer, _T( "%s 파일의 %d 번째 줄에서 오류" ), parser.GetFileName(), parser.GetLineNum() );
		
		assert( 0 && buffer );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUINodeSkin::IsValid() const
{
	/// 유효성 검사를 하나, 사이즈가 없을 경우에도
	/// 문제가 없이 돌리기 위해 그냥 트루 리턴
//	if( mSkinInfo->mX < 0 || mSkinInfo->mY < 0 )
//	{
//		assert( 0 && "error [x, y] value in script" );
//		return false;
//	}
	if( mSkinInfo->mTexY < 0 || mSkinInfo->mTexX < 0 )
	{
		assert( 0 && "error [texX, texY] value in script" );
		return false;
	}
	if( mSkinInfo->mWidth < 0 || mSkinInfo->mHeight < 0 )
	{
		assert( 0 && "error [size] value in script" );
		return false;
	}
	return true;
}
