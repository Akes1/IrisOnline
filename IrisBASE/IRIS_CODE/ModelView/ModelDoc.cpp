// ModelDoc.cpp : implementation of the CModelDoc class
//

#include "stdafx.h"
#include "ModelApp.h"

#include "ModelDoc.h"
#include ".\modeldoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDemoDoc

IMPLEMENT_DYNCREATE(CModelDoc, CDocument)

BEGIN_MESSAGE_MAP(CModelDoc, CDocument)
END_MESSAGE_MAP()


// CModelDoc construction/destruction

CModelDoc::CModelDoc()
{
	// TODO: add one-time construction code here

}

CModelDoc::~CModelDoc()
{
}

BOOL CModelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CDemoDoc serialization

void CModelDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CModelDoc diagnostics

#ifdef _DEBUG
void CModelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CModelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

