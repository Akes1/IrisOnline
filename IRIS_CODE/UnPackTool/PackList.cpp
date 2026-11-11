// PackList.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "UnPackTool.h"
#include "PackList.h"


// CPackList 대화 상자입니다.

IMPLEMENT_DYNAMIC(CPackList, CDialog)

CPackList::CPackList(CWnd* pParent /*=NULL*/)
	: CDialog(CPackList::IDD, pParent)
{

}

CPackList::~CPackList()
{
}

void CPackList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPackList, CDialog)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CPackList 메시지 처리기입니다.

void CPackList::OnDropFiles(HDROP hDropInfo)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

    char file[_MAX_PATH] = {0,};
    
    UINT count = DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );

    //int         nItem               = mPackList.GetItemCount();

    for( UINT i = 0; i < count; i++ )
    {
        DragQueryFile( hDropInfo, i, file, _MAX_PATH );
        CString path( file );
/*
        if( FindPackList( path ) )
            continue;

        sPackInfo* pack = new sPackInfo;

        /// 파일 경로
        strcpy_s( pack->path, path );

        /// 파일 이름
        int     sPos        = path.ReverseFind( '\\' );
        CString fileName    = path.Right( (path.GetLength() - sPos - 1) ); 
        strcpy_s( pack->name, fileName );

        mPackList.InsertItem( nItem, fileName );
        mPackList.SetItemData( nItem, (DWORD_PTR)pack );
        ++nItem;*/
    }

    ::DragFinish( hDropInfo );
    //CDialog::OnDropFiles(hDropInfo);
}

BOOL CPackList::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.

    return TRUE;  // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
