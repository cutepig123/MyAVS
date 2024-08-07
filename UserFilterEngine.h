#pragma once


void Eng_Create();

void Eng_Draw(CDC *pDC);

void Eng_OnLButtonDown(CWnd* pWnd, CPoint point);

void Eng_OnLButtonDblClk(CWnd* pWnd, CPoint point);

void Eng_OnButtonRun();
void Eng_AddBlock(const char* name);
void Eng_ContextMenu(CWnd* pWnd, CPoint const& point);
void Eng_OnMouseMove(CWnd* pWnd, CPoint point);
