#include "StdAfx.h"
#include "MakeSkillScript.h"

#include "FileLoader.h"
#include "Tokenizer.h"

cMakeSkillScript* cMakeSkillScript::mpMakeSkillScript = NULL;


cMakeSkillScript::cMakeSkillScript(void)
{
	/// 싱글톤
	if( !mpMakeSkillScript )
	{
		mpMakeSkillScript = this;
	}

}

cMakeSkillScript::~cMakeSkillScript(void)
{
	cPHashMap::cIterator b  = mGroupMap.Begin();
	cPHashMap::cIterator end  = mGroupMap.End();
	for( ; b != end; ++b )
	{
		sGroupInfo* info = (sGroupInfo*)(b->mSecond);
		if( info )
		{
			for( unsigned int i = 0; i < info->mRecipeArr.GetSize(); ++i )
			{
				sRecipeGroupInfo* groupInfo = (sRecipeGroupInfo*)info->mRecipeArr[i];
				SAFE_DELETE( groupInfo );
			}
			info->mRecipeArr.Clear();
		}
		SAFE_DELETE( info );
	}
	mGroupMap.Clear();

	b = mMakeSkillMap.Begin();
	end = mMakeSkillMap.End();
	for( ; b != end ; ++b )
	{
		sMakeSkillScript* pMake = (sMakeSkillScript*)(*b).mSecond;
		if( pMake )
		{
			cPAry* pAry = pMake->mpMaterial;
			if( pAry != NULL )
			{
				for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
				{
					sMaterialScript* pMaterial = (sMaterialScript*)(*pAry)[i];
					SAFE_DELETE( pMaterial );
				}
				pAry->Clear();
			}
			SAFE_DELETE( pAry );
		}
		SAFE_DELETE( pMake );
	}
	mMakeSkillMap.Clear();

#ifdef _CLIENT
	///
	cItemMixedTypeMap::cIterator i  = mItemMixedType.Begin();
	cItemMixedTypeMap::cIterator iend  = mItemMixedType.End();
	for( ; i != iend ; ++i )
	{
		cMixedTypeSet* pSet = (cMixedTypeSet*)(*i).mSecond;
		if( pSet )
		{
			pSet->Clear();
			delete pSet;
		}
	}
	mItemMixedType.Clear();
#endif
}

bool cMakeSkillScript::Init()
{
	/// 스킬 스크립트 로드
	if( !MakeSkillScriptLoad() )
	{
		assert(NULL);
		return false;
	}

	return true;

}


bool cMakeSkillScript::MakeSkillScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./script/resource/item_mixed.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_mixed.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	/// mix 확인
	if( tokenizer.GetNext( &str ) == false )
		return false;

	sMakeSkillScript*   pMakeScript     = NULL;
	sMaterialScript*    pMaterialScript = NULL;
	sGroupInfo*         pGroupInfo      = NULL;

	while( tokenizer.IsEnd() == false )
	{
		if( str.CompareNoCase( "mix" ) != 0 )
			return true;
		pMakeScript = new sMakeSkillScript;

		/// 전문기술
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMakeSkill = (unsigned char)str.ToInt();

		/// 단계
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMakeStep = (unsigned char)str.ToInt();
		
		/// 최소숙련도
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMinExp = (unsigned short)str.ToInt();

		/// 기본 최소숙련도
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMinDefaultExp = (unsigned short)str.ToInt();

		/// 기본 최대숙련도
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMaxDefaultExp = (unsigned short)str.ToInt();

		/// 최대 업 가능 숙련도
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMaxExp = (unsigned short)str.ToInt();

		/// 레시피
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mRecipeIdx = str.ToInt();

		/// 분류타입
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mType = str.ToInt();

		/// 레시피그룹
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mGroupIdx = (unsigned char)str.ToInt();

		/// 습득 그룹을 기준으로 관련된 레시피 정보들을 기록해두는 참조hash map
		if( pMakeScript->mGroupIdx != 0 )
		{
			pGroupInfo = (sGroupInfo*)mGroupMap.GetAt( pMakeScript->mGroupIdx );
			if( pGroupInfo == NULL )
			{
                pGroupInfo = new sGroupInfo;
                pGroupInfo->mStep       = pMakeScript->mMakeStep;
                pGroupInfo->mExp        = pMakeScript->mMinExp;

                if( mGroupMap.Insert( pMakeScript->mGroupIdx, pGroupInfo ) == false )
				{
					assert(0);
					SAFE_DELETE( pGroupInfo );
					return false;
				}
            }
            else
            {
                /// 그룹 중 최대 단계
                if( pGroupInfo->mStep < pMakeScript->mMakeStep )
                    pGroupInfo->mStep = pMakeScript->mMakeStep;

                /// 그룹 중 큰 값의 최소 숙련도
                if( pGroupInfo->mExp < pMakeScript->mMinExp )
                    pGroupInfo->mExp = pMakeScript->mMinExp;
            }

			sRecipeGroupInfo* pRecipeInfo = new sRecipeGroupInfo;
			pRecipeInfo->mRecipeIdx = pMakeScript->mRecipeIdx;
			pRecipeInfo->mMakeSkill = pMakeScript->mMakeSkill;

			pGroupInfo->mRecipeArr.PushBack( pRecipeInfo );
		}

		for( int i = 0 ; i < MAKESKILL_MAKE_ITEM_CNT ; ++i )
		{
			/// 완성된 아이템
			if( tokenizer.GetNext( &str ) == false )
				return false;
			pMakeScript->mMakeItemIdx[i] = str.ToInt();

			/// 완성된 수량
			if( tokenizer.GetNext( &str ) == false )
				return false;
			pMakeScript->mMakeCnt[i] = (unsigned char)str.ToInt();

			/// 선택 확률(비율)
			if( tokenizer.GetNext( &str ) == false )
				return false;
			pMakeScript->mMakeRate[i] = (unsigned char)str.ToInt();
		}

		/// 조합시간
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mMakeTime = str.ToInt();

		/// 재조합시간
		if( tokenizer.GetNext( &str ) == false )
			return false;
		pMakeScript->mNextMakeTime = str.ToInt();

		if( pMakeScript->mpMaterial != NULL )
			return false;

		cPAry* pMaterialAry = new cPAry;
		pMakeScript->mpMaterial = pMaterialAry;

		bool materialTrue = true;
		while( materialTrue )
		{
			/// mix 확인
			if( tokenizer.GetNext( &str ) == false )
				break;

			if( str.CompareNoCase( "mix" ) == 0 )
				break;
			
			pMaterialScript = new sMaterialScript;

			/// 재료 아이템
			pMaterialScript->mItemIdx = str.ToInt();

			/// 보유수량
			if( tokenizer.GetNext( &str ) == false )
				return false;
			pMaterialScript->mItemCnt = (unsigned char)str.ToInt();

			pMaterialAry->PushBack( pMaterialScript );

#ifdef _CLIENT
			///
			cMixedTypeSet* pSet = (cMixedTypeSet*)mItemMixedType.GetAt( pMaterialScript->mItemIdx );
			if( pSet == 0 )
			{
				pSet = new cMixedTypeSet;
				mItemMixedType.Insert( pMaterialScript->mItemIdx, pSet );
			}

			if( pSet )
				pSet->Insert( pMakeScript->mMakeSkill );
#endif
		}
	    
		if( mMakeSkillMap.Insert( pMakeScript->mRecipeIdx, pMakeScript ) == false )
			return false;
	}

	return true;
}


sMakeSkillScript* cMakeSkillScript::GetMakeSkill( unsigned long recipeIdx )
{
	return (sMakeSkillScript*)mMakeSkillMap.GetAt( recipeIdx );
}


sGroupInfo* cMakeSkillScript::GetGroupInfo( unsigned long groupIdx )
{
	return (sGroupInfo*)mGroupMap.GetAt( groupIdx );
}
