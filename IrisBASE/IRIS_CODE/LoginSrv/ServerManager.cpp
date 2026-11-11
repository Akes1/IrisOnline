// Include
#include "loginsrv.h"
//#include <time.h>

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }

// Global data

// cServerManager Constructor
cServerManager::cServerManager( )
{
	memset( mServerTable, 0, sizeof(mServerTable) );
	// Pool Usage Pointer
	mNonPagedPoolUsage      = NULL;

	// 메모리 카운터.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
}

// ~cServerManager Destructor.
cServerManager::~cServerManager(void)
{
	// cServerManager를 해제.
	Shutdown();
}

// Shutdown Method
void cServerManager::Shutdown()
{
	PerChannel* temp;

	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreeChannel( &temp );
	}
}

// AllocChannel Method
PerChannel* cServerManager::AllocChannel()
{
	PerChannel* pch = (PerChannel*)GlobalAlloc( GPTR, sizeof(PerChannel) );

	if ( pch != NULL )
	{
		mWorkingSetSize++;	// mWorkingSetSize를 증가.

		pch->cid        = 0;					// Connection Index
		pch->status     = _E_STATUS_CLOSED_;	// Status
		pch->timeToLive = STATUS_SYNC_INIT;		// Time To Live

		pch->prev       = NULL;	// 선형리스트의 포인터 - 이전
		pch->next       = NULL;	// 선형리스트의 포인터 - 다음
		pch->parent     = NULL;	// BST 포인터 - 상위
		pch->left       = NULL;	// BST 포인터 - 왼쪽
		pch->right      = NULL;	// BST 포인터 - 오른쪽
	}

	return pch;
}

// FreeChannel Method.
void cServerManager::FreeChannel(PerChannel** pch)
{
	if ( (*pch) != NULL )
	{
		GlobalFree( (*pch) );
		(*pch) = NULL;

		// mWorkingSetSize를 감소.
		mWorkingSetSize--;
	}
}

// CompareCID Method
int cServerManager::CompareCID(PerChannel* pch1, PerChannel* pch2)
{
	return (pch1->cid - pch2->cid);
}
int cServerManager::CompareCID(long cid1, long cid2)
{
	return (cid1 - cid2);
}

// AttachPool Method
void cServerManager::AttachPool(PerChannel** pool, PerChannel* pch)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = pch;

		pch->prev = NULL;
		pch->next = (*pool);
	}
	(*pool) = pch;
}

// DetachPool Method
void cServerManager::DetachPool(PerChannel** pool, PerChannel* pch)
{
	PerChannel* prev = pch->prev;
	PerChannel* next = pch->next;

	if ( prev == NULL && next == NULL )
	{
		(*pool) = NULL;
	}
	else if ( prev == NULL && next != NULL )
	{
		next->prev = NULL;
		(*pool) = next;
	}
	else if ( prev != NULL && next == NULL )
	{
		prev->next = NULL;
	}
	else if ( prev != NULL && next != NULL )
	{
		prev->next = next;
		next->prev = prev;
	}

	pch->prev = NULL;
	pch->next = NULL;
}

// AttachBst Method
bool cServerManager::AttachBst(PerChannel** root, PerChannel* pch)
{
	PerChannel* parent = NULL;
	PerChannel* child  = (*root);
	int          result;

	while ( child != NULL )
	{
		parent = child;
		result = CompareCID( child->cid, pch->cid );

		if ( result == 0 ) return false;
		else if ( result > 0 ) child = child->left;
		else if ( result < 0 ) child = child->right;
	}

	if ( parent == NULL )
	{
		(*root) = pch;
	}
	else if ( CompareCID( parent, pch ) > 0 )
	{
		SetLeft( parent, pch );
	}
	else // if ( CompareCID( parent, pch ) < 0 )
	{
		SetRight( parent, pch );
	}
	return true;
}

// DetachBst Method
bool cServerManager::DetachBst(PerChannel** root, PerChannel* pch)
{
	PerChannel* parent = pch->parent;
	PerChannel* left   = pch->left;
	PerChannel* right  = pch->right;
	PerChannel* child  = NULL;

	if ( left == NULL )
	{
		child = right;
	}
	else if ( right == NULL )
	{
		child = left;
	}
	else
	{
		child = right;
		while ( child->left != NULL ) child = child->left;

		if ( child->parent != pch )
		{
			SetLeft( child->parent, child->right );
			SetRight( child, right );
		}
		child->parent = parent;
		SetLeft( child, left );
	}

	if ( (*root) == pch )
	{
		(*root) = child;
		if ( (*root) != NULL ) (*root)->parent = NULL;
	}
	else if ( pch == parent->left )
	{
		SetLeft( parent, child );
	}
	else
	{
		SetRight( parent, child );
	}

	pch->parent = NULL;
	pch->left   = NULL;
	pch->right  = NULL;

	return true;
}

// GetPool Method
PerChannel* cServerManager::GetPool(ServerTable* root, long cid)
{
	PerChannel* pch = mNonPagedPoolUsage;

	if ( pch != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, pch );
		mQuotaNonPagedPoolUsage--;
	}
	else
	{
		// 페이지가 없을 경우 새로 생성한다.
		pch = AllocChannel( );
	}

	if ( pch != NULL )
	{
		pch->cid = cid;
		// BST - 연결시도 / 페이지 된 풀 사용량을 증감.
		if ( AttachBst( &root->root, pch ) == true )
		{
			AttachPool( &root->pool, pch );
			mQuotaPagedPoolUsage++;
		}
		// BST - 연결실패 / 페이지 안된 풀 사용량을 증감.
		else
		{
			AttachPool( &mNonPagedPoolUsage, pch );
			mQuotaNonPagedPoolUsage++;
			return NULL;
		}
	}

	return pch;
}

// ReleasePool Method
void cServerManager::ReleasePool(ServerTable* root, PerChannel* ch)
{
	// BST - 연결종료.
	DetachBst( &root->root, ch );

	// 페이지 된 풀 사용량을 차감.
	DetachPool( &root->pool, ch );
	mQuotaPagedPoolUsage--;

	// 페이지 안된 풀 사용량을 증감.
	AttachPool( &mNonPagedPoolUsage, ch );
	mQuotaNonPagedPoolUsage++;
}

// GetServerTable Method
ServerTable* cServerManager::GetServerTable(long cid)
{
	WORD idx = (HIWORD(cid) - 1);
	return (idx < MAX_SERVER) ? (mServerTable + idx) : NULL;
}

// SearchChannel Method
PerChannel* cServerManager::SearchChannel(ServerTable* serverTable, long cid)
{
	PerChannel* ch = serverTable->root;
	int         result;

	while ( ch != NULL )
	{
		result = CompareCID( ch->cid, cid );
		if ( result == 0 )
			return ch;
		else if ( result > 0 )
			ch = ch->left;
		else if ( result < 0 )
			ch = ch->right;
	}

	return NULL;
}

// FindChannel Method
long cServerManager::FindChannel(long cid)
{
	ServerTable* serverTable = GetServerTable( cid );
	PerChannel*  ch;
	int          result;

	if ( serverTable != NULL )
	{
		ch = serverTable->root;

		while ( ch != NULL )
		{
			result = CompareCID( ch->cid, cid );
			if ( result == 0 )
				return ch->cid;
			else if ( result > 0 )
				ch = ch->left;
			else if ( result < 0 )
				ch = ch->right;
		}

		ch = serverTable->pool;
		if ( ch != NULL )
			return ch->cid;
	}

	return 0;
}

// GetChannel Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseChannel와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerChannel* cServerManager::GetChannel(ServerTable* serverTable, long cid)
{
	PerChannel* ch = GetPool( serverTable, cid );
	if ( ch != NULL )
	{
		serverTable->channelCounter++;
	}
	return ch;
}

// ReleaseChannel Method - 선형리스트의 HEAD로 회수한다.
// GetChannel와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
bool cServerManager::ReleaseChannel(long cid)
{
	ServerTable* serverTable = GetServerTable( cid );
	PerChannel*  ch          = NULL;

	if ( serverTable != NULL )
	{
		ch = SearchChannel( serverTable, cid );
		if ( ch != NULL )
		{
			LOGINPROCESS->ServerDown( HIWORD(cid), LOWORD(cid) );

			serverTable->channelCounter--;

			ch->Status( _E_STATUS_CLOSED_ );
			ch->TimeToLive( STATUS_SYNC_INIT );

			g_loginSrv->GetSender( )->PostChStatus( ch->cid, ch->status );

			ReleasePool( serverTable, ch );
		}
		return true;
	}
	return false;
}

// UpdateProcess Method
void cServerManager::UpdateProcess(DWORD currentTick)
{
	ServerTable* serverTable = mServerTable;
	PerChannel*  ch          = NULL;
	PerChannel*  next        = NULL;
	DWORD        ttl         = currentTick + STATUS_SYNC_WAIT;

	for ( int i = 0; i < MAX_SERVER; i++, serverTable++ )
	{
		bool update = false;

		ch = serverTable->pool;	// 채널 값 설정
		while ( ch != NULL )
		{
			WORD sNO = HIWORD(ch->cid);
			WORD cNO = LOWORD(ch->cid);

			next = ch->next;	// 채널 값 백업

			if ( ch->timeToLive < currentTick )
			{
				char buffer[MAX_PATH];
				bool sms=false;

				switch ( ch->status )
				{
				case _E_STATUS_RUNABLE_:
				case _E_STATUS_RUNNING_S1_:
				case _E_STATUS_RUNNING_S2_:
				case _E_STATUS_RUNNING_S3_:
				case _E_STATUS_RUNNING_S4_:
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_FIN_WAIT1_" );

					ch->Status( _E_STATUS_FIN_WAIT1_ );
					ch->TimeToLive( ttl );
					g_loginSrv->GetSender( )->PostChStatus( ch->cid, ch->status );
					break;

				case _E_STATUS_FIN_WAIT1_:
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_FIN_WAIT2_" );

					ch->Status( _E_STATUS_FIN_WAIT2_ );
					ch->TimeToLive( ttl );
					g_loginSrv->GetSender( )->PostChStatus( ch->cid, ch->status );

					sms = true;
					break;

				case _E_STATUS_FIN_WAIT2_:
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_LAST_ACK_" );

					ch->Status( _E_STATUS_LAST_ACK_ );
					ch->TimeToLive( ttl );
					g_loginSrv->GetSender( )->PostChStatus( ch->cid, ch->status );
					break;

				case _E_STATUS_LAST_ACK_:
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_DELETE_" );

					ch->Status( _E_STATUS_DELETE_ );
					g_loginSrv->GetSender( )->PostChStatus( ch->cid, ch->status );
					ReleaseChannel( ch->cid );

					sms = true;
					break;

				default:
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_UNKNOWN_" );
					ch->TimeToLive( ttl );

					sms = true;
					break;
				}

				if ( serverTable->sms < MAX_SMS && sms == true )
				{
					LOGINPROCESS->WebSMS( buffer );
					serverTable->sms++;
				}
				LOGINPROCESS->PostServerEvent( "cServerManager::UpdateProcess:%s", buffer );
			}
			else
			{
				if ( ch->status >= _E_STATUS_RUNNING_S1_ && ch->status <= _E_STATUS_RUNNING_S4_ )
					update = true;
			}

			ch = next;			// 채널 값 복원
		}

		if ( update == true )
		{
			if ( serverTable->concurrentUser >= SERVER_CCU_S3 )
			{
				serverTable->status = _E_STATUS_RUNNING_S3_;
			}
			else if ( serverTable->concurrentUser >= SERVER_CCU_S2 )
			{
				serverTable->status = _E_STATUS_RUNNING_S2_;
			}
			else /*if ( serverTable->concurrentUser >= SERVER_CCU_S1 )*/
			{
				serverTable->status = _E_STATUS_RUNNING_S1_;
			}
		}
		else
			serverTable->status = _E_STATUS_CLOSED_;
	}
}
