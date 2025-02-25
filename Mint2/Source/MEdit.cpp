#include "stdafx.h"
#include "MEdit.h"
#include "MColorTable.h"
#include <mmsystem.h>
#include <stdio.h>
#include <memory.h>
#include "Mint.h"
#include "MDebug.h"

#define MEDIT_DEFAULT_WIDTH				100			// Default MEdit Widget Width, Height는 Font Height에 의해 초기 결정된다.

IMPLEMENT_LOOK(MEdit, MEditLook)

bool InsertString(char* szTarget, const char* szInsert, int nPos, int nMaxTargetLen=-1)
{
	int nTargetLen = strlen(szTarget);
	int nInsertLen = strlen(szInsert);
	if(nPos>nTargetLen) return false;
	if(nMaxTargetLen>0 && nTargetLen+nInsertLen>=nMaxTargetLen) return false;

	char* temp = new char[nTargetLen-nPos+2];
	strcpy(temp, szTarget+nPos);
	strcpy(szTarget+nPos, szInsert);
	strcpy(szTarget+nPos+nInsertLen, temp);
	delete[] temp;

	return true;
}

int DeleteChar(char* szTarget, int nPos)
{
	int nTargetLen = strlen(szTarget);
	if(nPos>=nTargetLen || nPos<0) return 0;

	int nCount = (IsHangul(szTarget[nPos])==true)?2:1;
	if(nPos+nCount>nTargetLen) nCount = 1;

	char* temp = new char[nTargetLen+2];
	strcpy(temp, szTarget);
	strcpy(szTarget+nPos, temp+nPos+nCount);
	delete[] temp;

	return nCount;
}

int NextPos(const char* szText, int nPos)
{
	int nLen = strlen(szText);

	if(nPos>=nLen) return nLen;

	if(IsHangul(szText[nPos])==true && nPos<nLen) return (nPos+2);

	return (nPos+1);
}

int PrevPos(char* szText, int nPos)
{
	int nLen = strlen(szText);
	if(nPos<=1) return 0;

	/*
	int nNewPos = nPos-1;
	if(nNewPos>0 && IsHangul(szText[nNewPos-1])==true) nNewPos--;

	return nNewPos;
	*/

	int nCurPos=0;
	while(1)
	{
		int nNext = nCurPos + (IsHangul(szText[nCurPos]) ? 2 : 1);
		if(nNext>=nPos) return nCurPos;
		nCurPos=nNext;
	}
}

void MEdit::OnHide(void)
{
	if (GetTabHandler()) {
		GetTabHandler()->Show(false);
	}
}

static bool g_bFocus=false;

void MEdit::OnSetFocus(void)
{
	Mint::GetInstance()->EnableIME(true);
//	g_bFocus=true;
//	MEvent::ForceSetIME(Mint::GetInstance()->m_dwIMEConvMode, IME_SMODE_AUTOMATIC);
}

void MEdit::OnReleaseFocus(void)
{
	Mint::GetInstance()->EnableIME(false);
//	g_bFocus=false;
//	MEvent::ForceSetIME(IME_CMODE_ALPHANUMERIC, IME_SMODE_AUTOMATIC);

	MListener* pListener = GetListener();
	if(pListener!=NULL) pListener->OnCommand(this, MEDIT_KILL_FOCUS);
}

bool MEdit::OnEvent(MEvent* pEvent, MListener* pListener)
{
	static bool bMyEvent = false;

	if(MWidget::m_pFocusedWidget!=this) return false;
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		{
			bool ret= InputFilterKey(pEvent->nKey);

			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_KEYDOWN_MSG);

			return ret;
		}
		break;
		//InputFilterKey(pEvent->nKey);
		//return true;
	case MWM_CHAR:
		{
			bool ret = false;
			if(InputFilterChar(pEvent->nKey)==false)
			{
				int nLen = strlen(m_pBuffer);
				if(nLen<m_nMaxLength-1)
				{
					char temp[2] = {(char)pEvent->nKey, 0};
					if(InsertString(m_pBuffer, temp, m_nCaretPos, m_nMaxLength)==true){
						m_nCaretPos++;
						OnChangedText();
					}
					_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
					/*
					if(m_nCaretPos==nLen){
						m_pBuffer[nLen] = (char)pEvent->nKey;
						m_pBuffer[nLen+1] = 0;
					}
					else{
						char* temp = new char[nLen];
						strcpy(temp, m_pBuffer+m_nCaretPos);
						m_pBuffer[m_nCaretPos] = (char)pEvent->nKey;
						m_nCaretPos++;
						strcpy(m_pBuffer+m_nCaretPos, temp);
						delete temp;
					}
					*/
				}
				ret=true;
			}
			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_CHAR_MSG);

			return ret;
		}
		break;
	case MWM_IMECONVERSION:
		{
//			OutputDebugString("MWM_IMECONVERSION\n");
			// IsFocus() 를 사용하면 ReleaseFocus때에 바뀔때에도 true이기 때문에...
			if(g_bFocus) OnSetFocus();
		}return false;
	case MWM_IMECOMPOSE:
		/*
		OutputDebugString("MWM_IMECOMPOSE\n");
		OutputDebugString(m_szIMECompositionString);
		*/
		strcpy(m_szIMECompositionString, pEvent->szIMECompositionString);
		if(pEvent->szIMECompositionResultString[0]!=NULL){
			if(InsertString(m_pBuffer, pEvent->szIMECompositionResultString, m_nCaretPos, m_nMaxLength)==true){
				m_nCaretPos += strlen(pEvent->szIMECompositionResultString);
				_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
				OnChangedText();
			}
			/*
			int nLen = strlen(m_pBuffer);
			if(nLen<int(m_nMaxLength-strlen(pEvent->szIMECompositionResultString))){
				wsprintf(m_pBuffer, "%s%s", m_pBuffer, pEvent->szIMECompositionResultString);
			}
			*/
			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_CHAR_MSG);

		}
		return true;
	case MWM_LBUTTONDOWN:
		// 마우스 클릭으로 캐럿 포지셔닝
		{
			MRECT r = GetClientRect();
			if(r.InPoint(pEvent->Pos)==true){
				int nPos = GetPosByScreen(pEvent->Pos.x);
				if(nPos<0) return false;
				m_nCaretPos = nPos;
				bMyEvent = true;
				return true;
			}
			return false;
		}

	case MWM_MOUSEMOVE:
		// 현재 캐럿의 위치와 다른 위치일 경우 셀렉트로 인정
		{			
			if( !bMyEvent ) return false;
		
			m_nSelectionRange = 0;
			int nCaretPos = 0;
			MRECT r = GetClientRect();

			if(r.InPoint(pEvent->Pos)==true){
				int nPos = GetPosByScreen(pEvent->Pos.x);
				if(m_nCaretPos==nPos) return false;
				m_nSelectionRange = nPos - m_nCaretPos;
			}
			return true;
		}
	case MWM_LBUTTONUP:
		{
			if(!bMyEvent) return false;
            bMyEvent = false;
			return true;
		}
	}
	return false;
}

void MEdit::OnChangedText()
{
	MListener* pListener = GetListener();
	if(pListener!=NULL) pListener->OnCommand(this, MEDIT_TEXT_CHANGED);
}

/*
bool MEdit::OnEvent(MEvent* pEvent, MListener* pListener)
{
	switch(pEvent->nMessage){
	case MWM_CHAR:
		if(InputFilter(pEvent->nKey)==false){
			int nLen = strlen(m_pBuffer);
			if(nLen<m_nMaxLength-1){
				m_pBuffer[nLen] = (char)pEvent->nKey;
				m_pBuffer[nLen+1] = 0;
			}
		}
		return true;
	case MWM_IMECOMPOSE:
		OutputDebugString("MWM_IMECOMPOSE\n");
		strcpy(m_szIMECompositionString, pEvent->szIMECompositionString);
		OutputDebugString(m_szIMECompositionString);
		if(pEvent->szIMECompositionResultString[0]!=NULL){
			int nLen = strlen(m_pBuffer);
			if(nLen<int(m_nMaxLength-strlen(pEvent->szIMECompositionResultString))){
				wsprintf(m_pBuffer, "%s%s", m_pBuffer, pEvent->szIMECompositionResultString);
			}
		}
		return true;
	}
	return false;
}
*/

bool MEdit::InputFilterKey(int nKey)
{
	if(nKey==VK_DELETE){
		int nCount = DeleteChar(m_pBuffer, m_nCaretPos);
		OnChangedText();
		//m_nCaretPos += nCount;
		return true;
	}
	else if(nKey==VK_LEFT){
		MoveCaretPrev();
		return true;
	}
	else if(nKey==VK_RIGHT){
		MoveCaretNext();
		return true;
	}
	else if(nKey==VK_UP){
		if(m_bSupportHistory==false) return true;

		if(m_nCurrentHistory==m_History.end()){
			m_nCurrentHistory = m_History.end();
			m_nCurrentHistory--;
		}
		else if(m_nCurrentHistory!=m_History.begin()){
			m_nCurrentHistory--;
		}

		if(m_nCurrentHistory!=m_History.end())
			SetText(*m_nCurrentHistory);

		return true;
	}
	else if(nKey==VK_DOWN){
		if(m_bSupportHistory==false) return true;

		if(m_nCurrentHistory!=m_History.end()){
			m_nCurrentHistory++;
		}

		if(m_nCurrentHistory!=m_History.end())
			SetText(*m_nCurrentHistory);
		else
			SetText("");

		return true;
	}
	else if(nKey==VK_HOME){
		MoveCaretHome();
		return true;
	}
	else if(nKey==VK_END){
		MoveCaretEnd();
		return true;
	}
	else if(nKey==VK_RETURN){
		//ReleaseFocus();
		if(m_bSupportHistory==true) AddHistory(GetText());
		MListener* pListener = GetListener();
		if(pListener!=NULL) return pListener->OnCommand(this, MEDIT_ENTER_VALUE);
		return false;
	}
	else if(nKey==VK_TAB){
		if (GetTabHandler()) {
			if (GetTabHandler()->IsVisible())
				GetTabHandler()->Show(false);
			else
				GetTabHandler()->Show(true,true);
			return true;
		} else {
			return false;
		}			
	}

	return false;
}

bool MEdit::InputFilterChar(int nKey)
{
	if(nKey==VK_BACK){
		int nCaretPos = PrevPos(m_pBuffer, m_nCaretPos);
		if(nCaretPos!=m_nCaretPos){
			int nCount = DeleteChar(m_pBuffer, nCaretPos);
			m_nCaretPos = nCaretPos;
			_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
			OnChangedText();
		}
		/*
		int nLen = strlen(m_pBuffer);
		if(nLen!=0){
			if(nLen>1 && (unsigned char)m_pBuffer[nLen-1]>127)	//한글
				m_pBuffer[nLen-2] = 0;
			else	// 영문
				m_pBuffer[nLen-1] = 0;
		}
		*/
		// 캐럿이 문자열의 시작지점보다 앞쪽이면, 보이게끔 해준다.
		if(m_nCaretPos<m_nStartPos) m_nStartPos = m_nCaretPos;

		return true;
	}
	else if(nKey==VK_ESCAPE){
		//ReleaseFocus();
		MListener* pListener = GetListener();
		if(pListener!=NULL) pListener->OnCommand(this, MEDIT_ESC_VALUE);
		return true;
	}
	else if(nKey==22){	// Ctrl+'V'
		char* temp = new char[m_nMaxLength];
		memset(temp, 0, m_nMaxLength);
		if ( GetClipboard(temp, m_nMaxLength)==true)
		{
			// 줄바꿈이 발견되면 잘라낸다.
			char *pFirst = strstr(temp,"\r");
			if(pFirst!=NULL)
			{
				*pFirst = 0;
				*(pFirst+1) = 0;
			}
			pFirst = strstr(temp,"\n");
			if(pFirst!=NULL)
			{
				*pFirst = 0;
				*(pFirst+1) = 0;
			}
			AddText(temp);
		}
		delete temp;
		return true;
	}
	else if(nKey==3){	// Ctrl+'C'
		SetClipboard(GetText());
		return true;
	}

	switch(nKey){
	case VK_TAB:
	case VK_RETURN:
	case VK_NONCONVERT:		// ctrl+] 를 하면 들어가는 문자, 무엇인지는 모름.
		return true;
	}

	// ctrl+a ~ z
	if(nKey<27) return true;

	if(IsNumberField()){
		if (nKey < '0' || '9' < nKey)
			return true;
	}

	return false;
}

void MEdit::Initialize(int nMaxLength, const char* szName)
{
	LOOK_IN_CONSTRUCTOR()

	m_nMaxLength = nMaxLength+2;
	m_bMouseOver = false;

	MFont* pFont = GetFont();
	int w = MEDIT_DEFAULT_WIDTH;
	int h = pFont->GetHeight()+2;
	//SetTextOffset(MPOINT(1, 1));

	//m_TextColor = MCOLOR(0, 255, 0); 

	m_pBuffer = new char[m_nMaxLength];
	m_pBuffer[0] = NULL;
	if(szName!=NULL) SetText(szName);

	m_szIMECompositionString[0] = NULL;

	m_bPassword = false;
	m_bNumberOnly = false;

	m_nCaretPos = 0;
	m_nStartPos = 0;
	m_nSelectionRange = 0;

	m_nCurrentHistory = m_History.end();
	m_bSupportHistory = true;

	SetFocusEnable(true);

	SetTabHandler(NULL);
}

MEdit::MEdit(int nMaxLength, const char* szName, MWidget* pParent, MListener* pListener )
 : MWidget(szName, pParent, pListener) //, m_TextOffset(0, 0)
{
	Initialize(nMaxLength, szName);
}

MEdit::MEdit(const char* szName, MWidget* pParent, MListener* pListener)
 : MWidget(szName, pParent, pListener) //, m_TextOffset(0, 0)
{
	Initialize(200, szName);
}

MEdit::~MEdit()
{
	if(m_pBuffer){
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}

	while(m_History.empty()==false){
		char* szText = *m_History.begin();
		delete szText;
		m_History.erase(m_History.begin());
	}
}

/*
void MEdit::SetTextOffset(MPOINT p)
{
	m_TextOffset = p;
}
*/
/*
void MEdit::SetTextColor(MCOLOR color)
{
	m_TextColor = color;
}

MCOLOR MEdit::GetTextColor(void)
{
	return m_TextColor;
}
*/

void MEdit::SetText(const char* szText)
{
	if(m_pBuffer) {
		strncpy(m_pBuffer, szText, m_nMaxLength-1);
		m_pBuffer[m_nMaxLength-1]=0;
		OnChangedText();
	}

	MoveCaretEnd();
	if(m_nStartPos>=m_nCaretPos)
		m_nStartPos=m_nCaretPos;
}

const char* MEdit::GetText(void)
{
	return m_pBuffer;
}


void MEdit::AddText(const char* szText)
{
	char		temp[ 1024 ]	= {0,};
	const int	nInputSize		= strlen( szText );
	const int	nCurSize		= strlen( GetText() );

	if( 1024 > (nInputSize, nCurSize) )
	{
		sprintf(temp, "%s%s", GetText(), szText);
		SetText(temp);
	}
	else
	{
		char szInputText[ 1024 ] = {0,};
		strncpy( szInputText, szText, 1024 - nCurSize );
		sprintf( temp, "%s%s", GetText(), szInputText );
	}
}

int MEdit::MoveCaretHome(void)
{
	m_nCaretPos = 0;
	return m_nCaretPos;
}

int MEdit::MoveCaretEnd(void)
{
	m_nCaretPos = strlen(m_pBuffer);
	return m_nCaretPos;
}

int MEdit::MoveCaretPrev(void)
{
	m_nCaretPos = PrevPos(m_pBuffer, m_nCaretPos);
	if(m_nCaretPos<m_nStartPos)
	{
		m_nStartPos=m_nCaretPos;
	}
	_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
	return m_nCaretPos;
}

int MEdit::MoveCaretNext(void)
{
	m_nCaretPos = NextPos(m_pBuffer, m_nCaretPos);
	_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
	return m_nCaretPos;
}

int MEdit::GetMaxLength(void)
{
	return m_nMaxLength;
}

const char* MEdit::GetCompositionString(void)
{
	return m_szIMECompositionString;
}

int MEdit::GetCarretPos(void)
{
	return m_nCaretPos;
}

bool MEdit::SetStartPos(int nStartPos)
{
	int nTextLen = strlen(GetText());
	if (nTextLen <= 0) {
		m_nStartPos = 0;
		return false;
	}
	if(nStartPos<0 || nStartPos>=nTextLen) return false;
	m_nStartPos = nStartPos;

	return true;
}

int MEdit::GetPosByScreen(int x)
{
	int nLen = strlen(m_pBuffer+GetStartPos());
	if(nLen==0) return -1;

	MFont* pFont = GetFont();
	MRECT r = GetClientRect();
	int i = 0;
	for (i = nLen; i>0; i = PrevPos(m_pBuffer + GetStartPos(), i)){
		int nWidth = MMGetWidth(pFont, m_pBuffer + GetStartPos(), i);
		if (x>r.x + nWidth){
			return i + GetStartPos();
		}
	}
	if(i==0) return GetStartPos();

	return -1;
}

void MEdit::SetPasswordField(bool bPassword)
{
	m_bPassword = bPassword;
}

bool MEdit::IsPasswordField(void)
{
	return m_bPassword;
}

void MEdit::SetNumberField(bool b)
{
	m_bNumberOnly = b;
}

bool MEdit::IsNumberField()
{
	return m_bNumberOnly;
}

bool MEdit::GetClipboard(char* szText, int nSize)
{
	if ( m_bPassword)
		return false;

	if(OpenClipboard(NULL)==FALSE)
		return false;

	/* get text from the clipboard */
	HANDLE hClipData = GetClipboardData(CF_TEXT);
	if(hClipData==NULL){
		CloseClipboard();
		return false;
	}
	
	HANDLE hText = GlobalAlloc(GMEM_MOVEABLE, GlobalSize(hClipData));
	if(hText==NULL){
		CloseClipboard();
		return false;
	}
	char* lpClipData = (char *)GlobalLock(hClipData);
	if(lpClipData==NULL){
		CloseClipboard();
		return false;
	}

	if((int)strlen(lpClipData)>(nSize-2)){
		memcpy(szText, lpClipData, nSize-1);
		szText[nSize-1] = 0;
	}
	else{
		strcpy(szText, lpClipData);
	}

	GlobalUnlock(hClipData);

	CloseClipboard();

	GlobalUnlock(hText);

	return true;
}

bool MEdit::SetClipboard(const char* szText)
{
	HANDLE hData = GlobalAlloc(GMEM_DDESHARE, strlen(szText)+2);
    if(hData==NULL){
        return false;
    }

	char* lpData = (char*)GlobalLock(hData);
    if(lpData==NULL){
		return false;
    }

	strcpy(lpData, szText);

    GlobalUnlock(hData);

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hData);
        CloseClipboard();
    }
	return true;
}

void MEdit::AddHistory(const char* szText)
{
	if(szText[0]==0) return;
	char* szNew = new char[strlen(szText)+2];
	strcpy(szNew, szText);
	m_History.insert(m_History.end(), szNew);
	m_nCurrentHistory = m_History.end();
}

void MEdit::SetMaxLength(int nMaxLength)
{
	nMaxLength += 1;
	char *pNewBuffer = new char[nMaxLength];
	ZeroMemory(pNewBuffer,nMaxLength);

	if (m_pBuffer != NULL) {
		strncpy(pNewBuffer,m_pBuffer,min(nMaxLength,m_nMaxLength));
		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
		OnChangedText();
	}

	m_nMaxLength = nMaxLength;

	MoveCaretEnd();
	if(m_nStartPos>=m_nCaretPos)
		m_nStartPos=m_nCaretPos;
}

void MEditLook::OnFrameDraw(MEdit* pEdit, MDrawContext* pDC)
{
	MRECT r = pEdit->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_OUTLINE));
	pDC->Rectangle(r);
}

void MEditLook::OnTextDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab )
{
#define BUFFERSIZE	1024

	char szBuffer[BUFFERSIZE];
	_ASSERT(sizeof(szBuffer)>pEdit->GetMaxLength()+2);
	if(pEdit->GetMaxLength()+2>BUFFERSIZE) return;

	if(pEdit->IsPasswordField()==false){
		wsprintf(szBuffer, "%s", pEdit->GetText());
	}
	else{
		memset(szBuffer, '*', strlen(pEdit->GetText()));
		szBuffer[strlen(pEdit->GetText())] = '\0';
	}

	// IME Composition String
	if(pEdit->IsFocus()==true && pEdit->GetCompositionString()[0]!=NULL){
		InsertString(szBuffer, pEdit->GetCompositionString(), pEdit->GetCarretPos());
	}

	/*
	MFDTextStyle nTextStyle=MFDTS_NORMAL;
	if(MWidget::m_pFocusedWindow==this) nTextStyle=MFDTS_ACTIVE;
	else if(m_bMouseOver==true) nTextStyle=MFDTS_NORMAL;
	else nTextStyle=MFDTS_NORMAL;
	*/
	//pFD->Text(pDC, m_Rect, temp, MAM_EDIT, nTextStyle, false);
	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_TEXT));

	MFont* pFont = pDC->GetFont();
	MRECT r = pEdit->GetClientRect();
	r.x-=2;
	MRECT scr=MClientToScreen(pEdit,r);
	pDC->SetClipRect(scr);

	if( bShowLanguageTab )
		r.w-=pFont->GetHeight();
//	r.w-=10;

	if(pEdit->GetCarretPos()<pEdit->GetStartPos())
		pEdit->SetStartPos(pEdit->GetCarretPos());

	_ASSERT(pEdit->GetCarretPos()>=pEdit->GetStartPos());

	int nCompositionStringLength = strlen(pEdit->GetCompositionString());


	// 텍스트의 시작 문자열 위치 지정
	char* szTemp = NULL;
	szTemp = szBuffer+pEdit->GetStartPos();
	int nTestLen = pEdit->GetCarretPos()-pEdit->GetStartPos()+nCompositionStringLength;	// pEdit->GetCompositionString()의 길이도 포함해야 StartPos 세팅에 컴포지션을 고려된다.

	// Composition 문자열이 Edit 너비를 넘어설때는 StartPos이 Composition 문자열 안으로 들어와야 한다.
	// 이런 경우 ESC 등으로 StartPos이 Composition 문자열을 제외한 원래 문자열로 돌아오는 문제점등이 생기므로,
	// StartPos의 범위는 Composition 문자열을 제외한 문자열로 제한한다.
#define INDICATOR_WIDTH	10
	while(szTemp[0]!=NULL && r.w-INDICATOR_WIDTH < pFont->GetWidth(szTemp, nTestLen)){	// -INDICATOR_WIDTH는 IME Indicator 위치 보정
		int nStartPos = NextPos(pEdit->GetText(), pEdit->GetStartPos());
		if(pEdit->SetStartPos(nStartPos)==false) break;
		szTemp = szBuffer+pEdit->GetStartPos();
		nTestLen = pEdit->GetCarretPos()-pEdit->GetStartPos()+nCompositionStringLength;
	}


	char* szStartText = szBuffer+pEdit->GetStartPos();	// 현재 창에 표시되는 문자열의 시작

	if(pEdit->IsFocus()==true){
		int nFontHeight = pFont->GetHeight();
		Mint* pMint = Mint::GetInstance();

		int nInsertPosInWidget = pFont->GetWidth(szStartText, pEdit->GetCarretPos()-pEdit->GetStartPos());
		int nCaretPosInWidget = pFont->GetWidth(szStartText, pEdit->GetCarretPos()+pMint->m_nCompositionCaretPosition-pEdit->GetStartPos());

		// 출력될 Candidate List 위치 지정하기
		MRECT r = pEdit->GetClientRect();
		//int nTextWidth = pFont->GetWidth(pEdit->GetText());
		MPOINT cp = MClientToScreen(pEdit, MPOINT(r.x+nInsertPosInWidget, r.y));
		pMint->SetCandidateListPosition(cp, r.h);

		// Caret
		int nSelStartPos=0;
		int nSelEndPos=0;
		//if(pEdit->m_nSelectionRange!=0)
		//{
		//	// Selection
		//	nSelStartPos = pEdit->GetCarretPos();
		//	nSelEndPos = nSelStartPos+pEdit->m_nSelectionRange;
		//	if(nSelStartPos>nSelEndPos)
		//	{
		//		int nTemp = nSelStartPos;
		//		nSelStartPos = nSelEndPos;
		//		nSelEndPos = nSelStartPos;
		//	}
		//	if(nSelStartPos<pEdit->GetStartPos()) nSelStartPos = pEdit->GetStartPos();

		//	const char* szSelText = szBuffer + nSelStartPos;
		//	int nWidth = pFont->GetWidth(szSelText, nSelEndPos-nSelStartPos);
		//	MRECT r = pEdit->GetClientRect();
		//	pDC->SetColor(109,207,246,255);
		//	pDC->FillRectangle( r.x+nCaretPosInWidget, r.y, nWidth, r.h );
		//}
		//else
		{
			// Caret
			long nCurrTime = timeGetTime();
			if((nCurrTime%(MEDIT_BLINK_TIME*2))>MEDIT_BLINK_TIME){
				//pDC->Text(r.x+nWidth-1, r.y, "|");
//				r.x+=nCaretPosInWidget-(int)((float)pFont->GetHeight()*0.3f);
				r.x+=nCaretPosInWidget;//-(int)((float)pFont->GetHeight());
				pDC->Text(r, "|", MAM_LEFT);
			}
		}


		r = pEdit->GetClientRect();

		// Composition 언더바 출력
		MPOINT p;
		pDC->GetPositionOfAlignment(&p, r, szStartText, MAM_LEFT);
		p.x += nInsertPosInWidget;
		pMint->DrawCompositionAttributes(pDC, p, pEdit->GetCompositionString());
	}


	r = pEdit->GetClientRect();
	//if(pEdit->m_nSelectionRange != 0 )
	//{
	//	if( nSelStartPos != 0 ) 
	//	{
	//		strncpy( szBuffer, szStartText, nSelStartPos );
	//		szBuffer[nSelStartPos+1]=0;
	//		pDC->Text( );
	//	}
	//	if( )
	//	{

	//	}

	//	pDC->Text(r, szBuffer, MAM_LEFT);
	//}
	//else
	pDC->Text(r, szStartText, MAM_LEFT);


	// 인디케이터 표기, 지전분하게 텍스트와 겹쳐서 보이지 않게 앞쪽에 그려준다.
	if(pEdit->IsFocus()==true){
		if(bShowLanguageTab){
			Mint* pMint = Mint::GetInstance();
			pMint->DrawIndicator(pDC, pEdit->GetClientRect());
		}
	}
}

void MEditLook::OnDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab)
{
	OnFrameDraw(pEdit, pDC);
	OnTextDraw(pEdit, pDC, bShowLanguageTab);
}

MRECT MEditLook::GetClientRect(MEdit* pEdit, MRECT& r)
{
	return MRECT(r.x, r.y, r.w, r.h);
	//return MRECT(r.x, r.y, r.w-10, r.h);	// IME Indicator 여유분
}
