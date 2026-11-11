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