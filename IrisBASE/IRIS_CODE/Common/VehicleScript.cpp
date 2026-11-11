#include "StdAfx.h"
#include "vehiclescript.h"

#include "vehicle_common.h"

#include "Tokenizer.h"


cVehicleScript* cVehicleScript::mpVehicleScript = NULL;


cVehicleScript::cVehicleScript()
{
	mpVehicleScript = this;
}

cVehicleScript::~cVehicleScript()
{
	sVehicleList* pInfo = NULL;
	cPointHashMap::cIterator iList = mVehicleMap.Begin();
	cPointHashMap::cIterator eList = mVehicleMap.End();

	for( ; iList != eList; ++iList )
	{
		pInfo = (sVehicleList*)((*iList).mSecond);
		SAFE_DELETE( pInfo );
	}
	mVehicleMap.Clear();
}

bool cVehicleScript::Init()
{
	if( LoadVehicleList() == false )
	{
		assert(0);
		return false;
	}

	return true;
}

bool cVehicleScript::LoadVehicleList()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/vehiclelist.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load GatheringList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sVehicleList* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sVehicleList;

		/// 종류 Index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mVehicleIdx = str.ToInt();

		/// fix Size
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mObjectFixSize = str.ToFloat();

		/// 모델
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFileName = str;

		/// 텍스쳐
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mTextureName = str;

		/// player 애니
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mPlayerIdle1 = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mPlayerIdle2 = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mPlayerRun = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mPlayerJump = str.ToInt();

		/// 해쉬에 기록
		if( mVehicleMap.Insert( pList->mVehicleIdx, pList ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pList;
	return false;
}

sVehicleList* cVehicleScript::GetVehicleInfo( unsigned long vehicleIdx )
{
	return (sVehicleList*)mVehicleMap.GetAt( vehicleIdx );
}
