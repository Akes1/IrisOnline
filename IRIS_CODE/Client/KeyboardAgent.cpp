#include "StdAfx.h"
#include "keyboardagent.h"

#include "ShortcutManager.h"

cKeyboardAgent::cKeyboardAgent()
: mpDidKey(NULL)
, mBufferedKeys(0)
, mPrevBuffer(0)
{
}

cKeyboardAgent::~cKeyboardAgent()
{
	Release();
}

// * ------------------------------------------------------------------
// * 함수이름 :	Create( HWND hWnd, LPDIRECTINPUT8 pdi )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
HRESULT cKeyboardAgent::Create( HWND hWnd, LPDIRECTINPUT8 pDi )
{
	HRESULT hr;
	unsigned long coopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY;

	hr = pDi->CreateDevice( GUID_SysKeyboard, &mpDidKey, NULL );
	hr |= mpDidKey->SetDataFormat( &c_dfDIKeyboard );
	hr |= mpDidKey->SetCooperativeLevel( hWnd, coopFlags );

	DIPROPDWORD dipdw;

	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = KEYBOARD_BUFFER;

	hr = mpDidKey->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );

	mpDidKey->Acquire();

	ClearKeyState();

	return hr;
}

// * ------------------------------------------------------------------
// * 함수이름 :	Release()
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
void cKeyboardAgent::Release()
{
	if( mpDidKey )
		mpDidKey->Unacquire();

	SAFE_RELEASE( mpDidKey );
}


// * ------------------------------------------------------------------
// * 함수이름 :	UpdateKeyboardState()
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
void cKeyboardAgent::UpdateKeyboardState()
{
	HRESULT hr = E_FAIL;

	mBufferedKeys = KEYBOARD_BUFFER;

	hr = mpDidKey->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), mDidodKey, &mBufferedKeys, 0 );
	if( FAILED( hr ) )
	{
		do 
		{
			hr = mpDidKey->Acquire();
		} 
		while( hr == DIERR_INPUTLOST );

		ClearKeyState();

		//hr = mpDidKey->Acquire();

		//while( hr == DIERR_INPUTLOST ) 
		//	hr = mpDidKey->Acquire();

		//if(SUCCEEDED(hr))
		//{
		//	ClearKeyState();
		//	return;
		//}
	}
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetNextKeyState()
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetNextKeyState()
{
	if(mBufferedKeys > KEYBOARD_BUFFER)
		return false;

	mKeyEvent = false;

	DWORD keyindex;
	unsigned long i;

	// 키보드 상태 처리
	for(i=0; i<mPrevBuffer; i++)
	{
		keyindex = mBufferKey[i];
		if(keyindex < 256)
		{
			switch(mKeyState[keyindex]) 
			{
			case _KEY_DOWN:
				mKeyState[keyindex] = _KEY_PRESS;
				mKeyEvent = true;
				break;
			case _KEY_UP:
				mKeyState[keyindex] = _KEY_NONE;
				break;
			}
		}
	}

	for(i=0; i<mBufferedKeys; i++)
	{
		keyindex = mDidodKey[i].dwOfs;
		if(keyindex < 256)
		{
			mBufferKey[i] = (unsigned char)keyindex;

			///PressKey
			if(mDidodKey[i].dwData & 0x80)
			{
				mKeyState[keyindex] = _KEY_DOWN;
				mKeyEvent = true;
			}
			else
			{
				mKeyState[keyindex] = _KEY_UP;
				mKeyEvent = true;
			}
		}
	}

	mPrevBuffer = mBufferedKeys;

	return true;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyDown( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyDown( unsigned long key )
{
	if( key < 256)
		return (mKeyState[key] == _KEY_DOWN);
	else
		return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyDown( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyDown( sKeyboardInfo* pKeyInfo )
{
	if( pKeyInfo == NULL )
		return false;

	unsigned long key = pKeyInfo->mKey;
	unsigned long mixKey = pKeyInfo->mMixKey;

	if( key < 256 && mixKey < 256 )
	{
 		if( mixKey == 0 )
 		{
 			if( mKeyState[key] == _KEY_DOWN )
 				return true;
 			
 		}
 		else
 		{
 			if( mKeyState[mixKey] == _KEY_PRESS && mKeyState[key] == _KEY_DOWN )
 				return true;
  		}
	}
	
	return false;
}


// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyUp( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyUp( unsigned long key )
{
	if( key < 256)
		return (mKeyState[key] == _KEY_UP);
	else
		return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyUp( sKeyboardInfo* pKeyInfo  )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyUp( sKeyboardInfo* pKeyInfo  )
{
	if( pKeyInfo == NULL )
		return false;

	unsigned long key = pKeyInfo->mKey;
	unsigned long mixKey = pKeyInfo->mMixKey;

	if( key < 256 && mixKey < 256 )
	{
		if( mixKey == 0 && mKeyState[key] == _KEY_UP )
			return true;
	}
	
	return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyPress( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyPress( unsigned long key )
{
	if( key < 256)
		return (mKeyState[key] == _KEY_PRESS);
	else
		return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyPressed( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyPressed( unsigned long key )
{
	if( key < 256)
		return (mKeyState[key] > _KEY_UP);
	else
		return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyPressed( sKeyboardInfo* pKeyInfo  )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::GetKeyPressed( sKeyboardInfo* pKeyInfo )
{
	if( pKeyInfo == NULL )
		return false;

	unsigned long key = pKeyInfo->mKey;
	unsigned long mixKey = pKeyInfo->mMixKey;

	if( key < 256 && mixKey < 256 )
	{
		if( mixKey == 0 && mKeyState[key] > _KEY_UP )
			return true;
	}

	return false;
}

// * ------------------------------------------------------------------
// * 함수이름 :	GetKeyState( unsigned long key )
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
int cKeyboardAgent::GetKeyState( unsigned long key )
{
	if( key < 256)
		return mKeyState[key];
	else
		return _KEY_NONE;
}

// * ------------------------------------------------------------------
// * 함수이름 :	ClearKeyState()
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
void cKeyboardAgent::ClearKeyState()
{
	mBufferedKeys = 0;
	mPrevBuffer = 0;
	memset( mBufferKey, 0, sizeof(unsigned char)*KEYBOARD_BUFFER );
	memset( mKeyState, 0, sizeof(unsigned char)*256 );
}

// * ------------------------------------------------------------------
// * 함수이름 :	IsEmptyKeyboardEvent()
// * 목    적 :	
// * 주의사항 :	
// * ------------------------------------------------------------------
bool cKeyboardAgent::IsEmptyKeyboardEvent()
{
	return mKeyEvent;
}

//#include "StdAfx.h"
//#include "keyboardagent.h"
//
//cKeyboardAgent::cKeyboardAgent()
//: mpDidKey(NULL)
//{
//	mDown = 0;
//
//	mBufferedKeys	= 0;
//	mBufferPos		= 1;
//	mDown = 0;
//
//	ZeroMemory( mKeyPress, sizeof( mKeyPress ) );
//}
//
//cKeyboardAgent::~cKeyboardAgent()
//{
//	Release();
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	Create( HWND hWnd, LPDIRECTINPUT8 pdi )
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//HRESULT cKeyboardAgent::Create( HWND hWnd, LPDIRECTINPUT8 pDi )
//{
//	HRESULT hr;
//	unsigned long coopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY;
//
//	hr = pDi->CreateDevice( GUID_SysKeyboard, &mpDidKey, NULL );
//	hr |= mpDidKey->SetDataFormat( &c_dfDIKeyboard );
//	hr |= mpDidKey->SetCooperativeLevel( hWnd, coopFlags );
//
//	DIPROPDWORD dipdw;
//
//	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
//	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
//	dipdw.diph.dwObj        = 0;
//	dipdw.diph.dwHow        = DIPH_DEVICE;
//	dipdw.dwData            = KEYBOARD_BUFFER;
//
//	hr = mpDidKey->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
//
//	mpDidKey->Acquire();
//
//	ClearKeyState();
//
//	return hr;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	Release()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//void cKeyboardAgent::Release()
//{
//	if( mpDidKey )
//		mpDidKey->Unacquire();
//
//	SAFE_RELEASE( mpDidKey );
//}
//
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	UpdateKeyboardState()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//void cKeyboardAgent::UpdateKeyboardState()
//{
//	mDown = 0;
//	HRESULT hr = E_FAIL;
//
//	mBufferedKeys = KEYBOARD_BUFFER;
//
//	hr = mpDidKey->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), mDidodKey, &mBufferedKeys, 0 );
//	if( FAILED( hr ) )
//	{
//		do
//		{
//			hr = mpDidKey->Acquire();
//		}
//		while( hr == DIERR_INPUTLOST );
//
//		ClearKeyState();
//	}
//	else
//	{
//		mBufferPos = mBufferedKeys;
//	}
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	StartGetKeyState()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//void cKeyboardAgent::StartGetKeyState()
//{
//	mBufferPos = 0;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	GetNextKeyState()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//bool cKeyboardAgent::GetNextKeyState()
//{
//	++mBufferPos;
//
//	if( mBufferPos > mBufferedKeys )
//	{
//		return false;
//	}
//
//	///PressKey
//	if( mDidodKey[mBufferPos-1].dwData & 0x80 )
//	{
//		mKeyPress[mDidodKey[mBufferPos-1].dwOfs] = true;
//		mDown = mDidodKey[mBufferPos-1].dwOfs;
//	}
//	else
//	{
//		mKeyPress[mDidodKey[mBufferPos-1].dwOfs] = false;
//		mDown = 0;
//	}
//
//	return true;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	GetKeyDown( unsigned long key )
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//bool cKeyboardAgent::GetKeyDown( unsigned long key )
//{
//	if( mBufferedKeys == 0 ) 
//	{
//		return false;
//	}
//
//	if( mBufferPos > mBufferedKeys || mBufferPos < 1 )
//	{
//		return false;
//	}
//
//	if( ( mDidodKey[mBufferPos-1].dwOfs == key ) && ( mDidodKey[mBufferPos-1].dwData & 0x80 ) )
//	{
//		return true;
//	}
//
//	return false;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	GetKeyUp( unsigned long key )
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//bool cKeyboardAgent::GetKeyUp( unsigned long key )
//{
//	if( mBufferedKeys == 0 ) 
//	{
//		return false;
//	}
//
//	if( mBufferPos > mBufferedKeys || mBufferPos < 1 )
//	{
//		return false;
//	}
//
//	assert( mBufferPos <= mBufferedKeys && mBufferPos > 0 );
//
//	if( ( mDidodKey[mBufferPos-1].dwOfs == key ) && !( mDidodKey[mBufferPos-1].dwData & 0x80 ) )
//	{
//		return true;
//	}
//
//	return false;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	GetKeyPressed( unsigned long key )
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//bool cKeyboardAgent::GetKeyPressed( unsigned long key )
//{
//	return mKeyPress[key]? true:false;
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	ClearKeyState()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//void cKeyboardAgent::ClearKeyState()
//{
//	mBufferedKeys	= 0;
//	mBufferPos		= 1;
//	mDown = 0;
//
//	ZeroMemory( mKeyPress, sizeof( mKeyPress ) );
//}
//
//// * ------------------------------------------------------------------
//// * 함수이름 :	IsEmptyKeyboardEvent()
//// * 목    적 :	
//// * 주의사항 :	
//// * ------------------------------------------------------------------
//bool cKeyboardAgent::IsEmptyKeyboardEvent()
//{
//	if( mBufferPos > mBufferedKeys )
//	{
//		return true;
//	}
//
//	return false;
//}
