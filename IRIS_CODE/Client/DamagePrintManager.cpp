#include "StdAfx.h"
#include "damageprintmanager.h"

#include "EffectSceneNode.h"
#include "player.h"

#include "Player_Common.h"

cDamagePrintManager* cDamagePrintManager::mpDamagePrintManager = NULL;

cDamagePrintManager::cDamagePrintManager()
{
	mpDamagePrintManager = this;

	float fwidth = 512.0f;
	float fheight = 512.0f;
	float sizeW = 46.0f;//51.2f;
	float sizeH = 46.0f;//51.2f;

	/// 간격 설정
	for( unsigned int i = 0; i<eDAMAGEPRINT_MAX; i++ )
	{
		NiRect<float>* pR = mTextureRect[i];
		float top = (sizeH*(float)i + 0.5f) / fheight;
		float bottom = sizeH*(float)(i+1) / fheight;
		for( unsigned int j = 1; j < 11; j++ )
		{
			pR[j%10].m_left = (sizeW*(float)(j-1) + 0.5f)/ fwidth;
			pR[j%10].m_top = top;
			pR[j%10].m_right = sizeW*(float)j / fwidth;
			pR[j%10].m_bottom = bottom;
		}
	}
}

cDamagePrintManager::~cDamagePrintManager()
{
}

bool cDamagePrintManager::PrintDamage( cBaseObject* pobj, unsigned int damage, eDAMAGEPRINT_TYPE type, bool ownerHero )
{
	if( pobj == 0 ) 
		return false;

	/// 연결할 파일명 결정
	cString str;
	switch( type )
	{
	case eDAMAGEPRINT_NORMAL_OUR:
	case eDAMAGEPRINT_NORMAL_ENEMY:
	case eDAMAGEPRINT_HEAL_OUR:
	case eDAMAGEPRINT_HEAL_ENEMY:
	case eDAMAGEPRINT_MP_HEAL:
	case eDAMAGEPRINT_MP_NORMAL:
		str = "./Data/Effect/Eff_Bagic_Dam_01.nif";
		break;
	case eDAMAGEPRINT_CRITICAL_OUR:
	case eDAMAGEPRINT_CRITICAL_ENEMY:
	case eDAMAGEPRINT_MP_CRITICAL:
		str = "./Data/Effect/Eff_Bagic_DamC_01.nif";
		break;
	case eDAMAGEPRINT_MISS:
		str = "./Data/Effect/Eff_Basic_Miss_01.nif";
		break;
	default:
		assert(0);
		return false;
	}

	/// head2 더미 위치에 연결할 이펙트 노드 생성
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 100.0f );

	cEffectSceneNode* pnode;
	if( type == eDAMAGEPRINT_MISS )
	{
		pnode = pobj->SetLinkdDamage( eLINK_HEAD2, str.Cstr(), &trans, true, ownerHero );
		if( pnode == 0 )
			return false;

		return true;
	}
	else
	{
		pnode = pobj->SetLinkdDamage( eLINK_HEAD2, str.Cstr(), &trans, false, ownerHero );

		if( pnode == 0 )
			return false;
	}

	/// 데미지를 자리수별로 나눈다..
	unsigned int count = 0;
	unsigned int number[4] = {0,};
	unsigned int d = damage;
	for(; count<4 ;count++ )
	{
		number[3-count] = d%10;

		d = d/10;
		if( d == 0 )
			break;
	}
	/// 자리수
	count = count +1;
	if( count > 4 )
		count = 4;

	/// 여기에 4개의 배열을 연결
	NiMesh* dGeom[4] = {0,};
	if( pnode->GetNiNode() == 0 )
	{
		pnode->Remove();
		return false;
	}

	if( FindPrintTextureInfo( pnode->GetNiNode(), dGeom ) == false )
	{
		pnode->Remove();
		return false;
	}

	/// uv 셋팅 변경
	unsigned int startIdx = 4;
	switch( count )
	{
	case 1:
	case 2:
		startIdx = 1;
		break;
	case 3:
	case 4:
		startIdx = 0;
		break;
	}
	if( startIdx + count > 4 )
		startIdx = 0;

	for( unsigned int i = 0; i<count; i++ )
	{
		dGeom[startIdx+i]->SetAppCulled( false );

		NiRect<float> r = mTextureRect[type][number[4-count+i]];

		NiDataStreamElementLock kLock( dGeom[startIdx+i], NiCommonSemantics::TEXCOORD(), 0,
			NiDataStreamElement::F_FLOAT32_2, 
			NiDataStream::LOCK_READ | NiDataStream::LOCK_WRITE );
		assert( kLock.IsLocked() );

		NiTStridedRandomAccessIterator<NiPoint2> kBegin = kLock.begin<NiPoint2>();
		NiTStridedRandomAccessIterator<NiPoint2> kEnd = kLock.end<NiPoint2>();

		if( kBegin != kEnd )
		{
			kBegin[0].x = r.m_left;
			kBegin[0].y = r.m_top;
			kBegin[1].x = r.m_left;
			kBegin[1].y = r.m_bottom;
			kBegin[3].x = r.m_right;
			kBegin[3].y = r.m_bottom;
			kBegin[2].x = r.m_right;
			kBegin[2].y = r.m_top;
		}

		kLock.Unlock();
	}

	return true;
}

bool cDamagePrintManager::FindPrintTextureInfo( NiNode* pNode, NiMesh** pArray )
{
	if( pNode == 0 || pArray == 0 )
		return false;

	cString str;
	for( unsigned int i = 0; i<4; i++ )
	{
		str.Format("Dam%d", i+1);
		NiAVObject* pobj = pNode->GetObjectByName( str.Cstr() );
		if( pobj == 0 )
		{
			assert(0);
			return false;
		}

		NiMesh* pgeom = cSceneNode::GetGeom( pobj );
		if( pgeom == 0 )
		{
			assert(0);
			return false;
		}

		NiDataStreamRef* r = pgeom->FindStreamRef( NiCommonSemantics::TEXCOORD() );
		if( r )
		{
			NiDX9DataStream* stream = NiDynamicCast( NiDX9DataStream, r->GetDataStream() );
			if( stream )
			{
				NiUInt8 mask = stream->GetAccessMask();
				mask &= ~NiDataStream::ACCESS_CPU_WRITE_STATIC_INITIALIZED;
				mask |= NiDataStream::ACCESS_CPU_WRITE_MUTABLE | NiDataStream::ACCESS_CPU_WRITE_STATIC;

				stream->SetAccessMask( mask );
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			assert(0);
		}

		/// ch_2.6
		if( pgeom->GetVertexCount() != 4 )
			return false;
		if( pgeom->GetSemanticCount( NiCommonSemantics::TEXCOORD() ) != 1 )
			return false;

		pgeom->SetAppCulled( true );
		pArray[i] = pgeom;
	}
	return true;
}
