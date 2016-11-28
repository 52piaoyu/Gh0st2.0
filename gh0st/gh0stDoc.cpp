// gh0stDoc.cpp : implementation of the Cgh0stDoc class
//

#include "stdafx.h"
#include "gh0st.h"
#include "gh0stDoc.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cgh0stDoc

IMPLEMENT_DYNCREATE(Cgh0stDoc, CDocument)

BEGIN_MESSAGE_MAP(Cgh0stDoc, CDocument)
	//{{AFX_MSG_MAP(Cgh0stDoc)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cgh0stDoc construction/destruction

Cgh0stDoc::Cgh0stDoc()
{
	// TODO: add one-time construction code here
}

Cgh0stDoc::~Cgh0stDoc()
{
}

//DEL BOOL Cgh0stDoc::OnNewDocument()
//DEL {
//DEL 	if (!CDocument::OnNewDocument())
//DEL 		return FALSE;
//DEL
//DEL 	// TODO: add reinitialization code here
//DEL 	// (SDI documents will reuse this document)
//DEL 	return TRUE;
//DEL }

/////////////////////////////////////////////////////////////////////////////
// Cgh0stDoc serialization

void Cgh0stDoc::Serialize(CArchive& ar)
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

/////////////////////////////////////////////////////////////////////////////
// Cgh0stDoc diagnostics

#ifdef _DEBUG
void Cgh0stDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void Cgh0stDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Cgh0stDoc commands

void Cgh0stDoc::DeleteContents()
{
	//if(AfxGetApp()->m_pMainWnd) ((CMainFrame*)AfxGetApp()->m_pMainWnd)->DeleteContents();
	((CMainFrame*)AfxGetApp());
	CDocument::DeleteContents();
}