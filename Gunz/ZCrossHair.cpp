#include "stdafx.h"

#include "ZCrossHair.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZIDLResource.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZConfiguration.h"
#include "MComboBox.h"
#include "ZCanvas.h"

ZCrossHair::ZCrossHair()
{
	for (int i = 0; i < CH_MAX; i++)
	{
		m_pBitmaps[i] = NULL;
		m_pPickBitmaps[i] = NULL;
	}
	m_nStatus = ZCS_NORMAL;

	m_bVisible = false;
}

ZCrossHair::~ZCrossHair()
{
	Destroy();
}

bool ZCrossHair::Create()
{
	m_nStatus = ZCS_NORMAL;

	return true;
}

void ZCrossHair::Destroy()
{
	for (int i = 0; i < CH_MAX; i++)
	{
		m_pBitmaps[i] = NULL;
		m_pPickBitmaps[i] = NULL;
	}
}

void ZCrossHair::GetBitmaps(MBitmap** ppoutBitmap, MBitmap** ppoutPickBitmap, ZCrossHairPreset nPreset)
{
	char szBar[CH_MAX][256], szPick[CH_MAX][256];
	for (int i = 0; i < CH_MAX; i++)
	{
		szBar[i][0] = 0;
		szPick[i][0] = 0;
	}

	sprintf(szBar[CH_CENTER], "interface/custom/Crosshair/%d/%s%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TAILER);
	sprintf(szBar[CH_TOP], "interface/custom/Crosshair/0%d/%s%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TOP, FN_CROSSHAIR_TAILER);
	sprintf(szBar[CH_BOTTOM], "interface/custom/Crosshair/0%d/%s%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_BOTTOM, FN_CROSSHAIR_TAILER);
	sprintf(szBar[CH_LEFT], "interface/custom/Crosshair/0%d/%s%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_LEFT, FN_CROSSHAIR_TAILER);
	sprintf(szBar[CH_RIGHT], "interface/custom/Crosshair/0%d/%s%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_RIGHT, FN_CROSSHAIR_TAILER);

	sprintf(szPick[CH_CENTER], "interface/custom/Crosshair/%d/%s_pick%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TAILER);
	sprintf(szPick[CH_TOP], "interface/custom/Crosshair/0%d/%s_pick%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TOP, FN_CROSSHAIR_PICK, FN_CROSSHAIR_TAILER);
	sprintf(szPick[CH_BOTTOM], "interface/custom/Crosshair/0%d/%s_pick%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_BOTTOM, FN_CROSSHAIR_PICK, FN_CROSSHAIR_TAILER);
	sprintf(szPick[CH_LEFT], "interface/custom/Crosshair/0%d/%s_pick%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_LEFT, FN_CROSSHAIR_PICK, FN_CROSSHAIR_TAILER);
	sprintf(szPick[CH_RIGHT], "interface/custom/Crosshair/0%d/%s_pick%s", (int)nPreset + 1, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_RIGHT, FN_CROSSHAIR_PICK, FN_CROSSHAIR_TAILER);

	for (int i = 0; i < CH_MAX; i++)
	{
		if (ppoutBitmap != NULL)
			ppoutBitmap[i] = MBitmapManager::Get(szBar[i]);

		if (ppoutPickBitmap != NULL)
			ppoutPickBitmap[i] = MBitmapManager::Get(szPick[i]);
	}
}

void ZCrossHair::Change(ZCrossHairPreset nPreset)
{
	GetBitmaps(m_pBitmaps, m_pPickBitmaps, nPreset);
}

void ZCrossHair::ChangeFromOption()
{
	MComboBox* pComboBox = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CrossHairComboBox");
	if (pComboBox)
	{
		int nSelIndex = Z_ETC_CROSSHAIR;
		if ((nSelIndex >= 0) && (nSelIndex <= ZCSP_CUSTOM))
		{
			Change(ZCrossHairPreset(nSelIndex));
		}
	}

}

void ZCrossHair::DrawCrossHair(MDrawContext* pDC, MBitmap** ppBitmaps, MPOINT& center,
	float fSizeFactor, float fCAFactor)
{
	int sizex = 32;
	int sizey = 32;

	if (ppBitmaps[CH_CENTER])
	{
		sizex = ppBitmaps[CH_CENTER]->GetWidth() * fSizeFactor;
		sizey = ppBitmaps[CH_CENTER]->GetHeight() * fSizeFactor;
		pDC->SetBitmap(ppBitmaps[CH_CENTER]);
		pDC->Draw(center.x - sizex / 2, center.y - sizey / 2, sizex, sizey);
	}
	sizey = sizex = 32 * fSizeFactor;


	if (ppBitmaps[CH_LEFT]) {
		int bar3sizex = ppBitmaps[CH_LEFT]->GetWidth() * fSizeFactor;
		int bar3sizey = ppBitmaps[CH_LEFT]->GetHeight() * fSizeFactor;

		pDC->SetBitmap(ppBitmaps[CH_LEFT]);
		pDC->Draw(center.x - bar3sizex / 2 - (float)sizex*fCAFactor, center.y - bar3sizey / 2, bar3sizex, bar3sizey);
	}

	if (ppBitmaps[CH_RIGHT])
	{
		int bar1sizex = ppBitmaps[CH_RIGHT]->GetWidth() * fSizeFactor;
		int bar1sizey = ppBitmaps[CH_RIGHT]->GetHeight() * fSizeFactor;

		pDC->SetBitmap(ppBitmaps[CH_RIGHT]);
		pDC->Draw(center.x - bar1sizex / 2 + (float)sizex*fCAFactor, center.y - bar1sizey / 2, bar1sizex, bar1sizey);
	}

	if (ppBitmaps[CH_TOP])
	{
		int bar0sizex = ppBitmaps[CH_TOP]->GetWidth() * fSizeFactor;
		int bar0sizey = ppBitmaps[CH_TOP]->GetHeight() * fSizeFactor;

		pDC->SetBitmap(ppBitmaps[CH_TOP]);
		pDC->Draw(center.x - bar0sizex / 2, center.y - bar0sizey / 2 - (float)sizex*fCAFactor, bar0sizex, bar0sizey);
	}

	if (ppBitmaps[CH_BOTTOM])
	{
		int bar2sizex = ppBitmaps[CH_BOTTOM]->GetWidth() * fSizeFactor;
		int bar2sizey = ppBitmaps[CH_BOTTOM]->GetHeight() * fSizeFactor;

		pDC->SetBitmap(ppBitmaps[CH_BOTTOM]);
		pDC->Draw(center.x - bar2sizex / 2, center.y - bar2sizey / 2 + (float)sizex*fCAFactor, bar2sizex, bar2sizey);
	}
}


void ZCrossHair::Draw(MDrawContext* pDC)
{
	if (!m_bVisible) return;
	if (ZGetGame()->m_pMyCharacter == NULL) return;

	const float sizefactor = (float)MGetWorkspaceWidth() / (float)800 * 1.f;

	MPOINT center(MGetWorkspaceWidth() / 2, MGetWorkspaceHeight() / 2);

	float fFactor = ZGetGame()->m_pMyCharacter->GetCAFactor();
	fFactor = fFactor + 0.2f;

	switch (m_nStatus)
	{
	case ZCS_NORMAL:
	{
		DrawCrossHair(pDC, m_pBitmaps, center, sizefactor, fFactor);
	}
		break;
	case ZCS_PICKENEMY:
	{
		if (m_pPickBitmaps[CH_CENTER] != NULL)
			DrawCrossHair(pDC, m_pPickBitmaps, center, sizefactor, fFactor);
		else DrawCrossHair(pDC, m_pBitmaps, center, sizefactor, fFactor);
	}
		break;
	}
}


void ZCrossHair::OnDrawOptionCrossHairPreview(void* pCanvas, MDrawContext *pDC)
{
	MBitmap* pBitmaps[CH_MAX] = { NULL, };

	int nSelIndex = 0;
	MComboBox* pComboBox = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CrossHairComboBox");
	if (pComboBox)
	{
		nSelIndex = pComboBox->GetSelIndex();
	}

	int width = 64, height = 64;
	if (pCanvas != NULL)
	{
		ZCanvas* pcanvas = (ZCanvas*)pCanvas;
		width = pcanvas->GetClientRect().w;
		height = pcanvas->GetClientRect().h;
	}

	GetBitmaps(pBitmaps, NULL, ZCrossHairPreset(nSelIndex));

	pDC->SetColor(0, 0, 0);
	pDC->FillRectangle(0, 0, width, height);


	float sizefactor = 1.0f;
	MPOINT center(width / 2, height / 2);
	float fFactor = 0.5f + 0.2f;

	DrawCrossHair(pDC, pBitmaps, center, sizefactor, fFactor);

	pDC->SetColor(128, 128, 128);
	pDC->Rectangle(0, 0, width, height);
}

int ZCrossHair::GetHeight()
{
	if (m_pBitmaps[CH_CENTER]) return m_pBitmaps[CH_CENTER]->GetHeight();
	return 0;
}
int ZCrossHair::GetWidth()
{
	if (m_pBitmaps[CH_CENTER]) return m_pBitmaps[CH_CENTER]->GetWidth();
	return 0;
}

