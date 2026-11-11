#include "Stdafx.h"
#include "SpinCtrl.h"

cSpinCtrl::cSpinCtrl( eUINodeType type )
: cUIWindow( type )
{

}

cSpinCtrl::~cSpinCtrl()
{
	Clear();
}

/// 주의사항 :
void cSpinCtrl::AddData( LPCTSTR str, void* pExData )
{
	sSpinData* pData = new sSpinData;
	pData->mLabel = str;
	pData->mpExData = pExData;

	mSpinArray.PushBack( pData );
}

/// 주의사항 :
LPCTSTR cSpinCtrl::GetLabelData( int index ) const
{
	if( mSpinArray.GetSize() <= 0 )
		return 0;

	if( index >= (int)mSpinArray.GetSize() )
	{
		assert( 0 && "out of range" );
		return 0;
	}

	sSpinData* pData = (sSpinData*)mSpinArray[index];
	return pData->mLabel.Cstr();
}

void* cSpinCtrl::GetExtraData( int index ) const
{
	if( mSpinArray.GetSize() <= 0 )
		return 0 ;

	if( index >= (int)mSpinArray.GetSize() )
	{
		assert(0 && _T("out of range") );
		return 0;
	}

	sSpinData* pData = (sSpinData*)mSpinArray[index];
	return pData->mpExData;
}

void cSpinCtrl::Clear()
{
	for( unsigned int i=0; i<mSpinArray.GetSize(); ++i )
	{
		sSpinData* pData = (sSpinData*)mSpinArray[i];
		if( pData )
			delete pData;
	}
	mSpinArray.Clear();
}