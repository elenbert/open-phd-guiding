/*
 *  graph.cpp
 *  PHD Guiding
 *
 *  Created by Craig Stark.
 *  Copyright (c) 2008-2010 Craig Stark.
 *  All rights reserved.
 *
 *  Seriously modified by Bret McKee
 *  Copyright (c) 2013 Bret McKee
 *  All rights reserved.
 *
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Craig Stark, Stark Labs,
 *     Bret McKee, Dad Dog Development, Ltd, nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "phd.h"
#include <wx/dcbuffer.h>
#include <wx/utils.h>
#include <wx/colordlg.h>

static const int DefaultMinLength =  50;
static const int DefaultMaxLength = 400;
static const int DefaultMinHeight =  1;
static const int DefaultMaxHeight = 16;

BEGIN_EVENT_TABLE(GraphLogWindow, wxWindow)
EVT_PAINT(GraphLogWindow::OnPaint)
EVT_BUTTON(BUTTON_GRAPH_MODE,GraphLogWindow::OnButtonMode)
EVT_BUTTON(BUTTON_GRAPH_LENGTH,GraphLogWindow::OnButtonLength)
EVT_BUTTON(BUTTON_GRAPH_HEIGHT,GraphLogWindow::OnButtonHeight)
EVT_BUTTON(BUTTON_GRAPH_CLEAR,GraphLogWindow::OnButtonClear)
EVT_SPINCTRL(GRAPH_RAA,GraphLogWindow::OnUpdateSpinGuideParams)
EVT_SPINCTRL(GRAPH_RAH,GraphLogWindow::OnUpdateSpinGuideParams)

#if (wxMAJOR_VERSION > 2) || ((wxMAJOR_VERSION == 2) && (wxMINOR_VERSION > 8))
EVT_SPINCTRLDOUBLE(GRAPH_MM,GraphLogWindow::OnUpdateSpinDGuideParams)
#endif

EVT_SPINCTRL(GRAPH_MRAD,GraphLogWindow::OnUpdateSpinGuideParams)
EVT_SPINCTRL(GRAPH_MDD,GraphLogWindow::OnUpdateSpinGuideParams)
EVT_CHOICE(GRAPH_DM,GraphLogWindow::OnUpdateCommandGuideParams)
END_EVENT_TABLE()

GraphLogWindow::GraphLogWindow(wxWindow *parent):
//wxMiniFrame(parent,wxID_ANY,_("History"),wxDefaultPosition,wxSize(610,254),(wxCAPTION & ~wxSTAY_ON_TOP) | wxRESIZE_BORDER)
wxWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize, wxFULL_REPAINT_ON_RESIZE,_("Profile"))
{
    int width;
    wxCommandEvent dummy;

    //SetFont(wxFont(8,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));

    m_pParent = parent;

    wxBoxSizer *pMainSizer   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pClientSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *pControlSizer = new wxBoxSizer(wxHORIZONTAL);

    pMainSizer->Add(pButtonSizer, wxSizerFlags().Left().DoubleHorzBorder().Expand());
    pMainSizer->Add(pClientSizer, wxSizerFlags().Expand().Proportion(1));

    m_pClient = new GraphLogClientWindow(this);
    pClientSizer->Add(m_pClient, wxSizerFlags().Expand().Proportion(1));
    pClientSizer->Add(pControlSizer, wxSizerFlags().Expand().Center().Border(wxBOTTOM, 5));

    m_visible = false;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetBackgroundColour(*wxBLACK);

    m_pLengthButton = new wxButton(this,BUTTON_GRAPH_LENGTH,_T("foo"));
    m_pLengthButton->SetToolTip(_("# of frames of history to display"));
    OnButtonLength(dummy); // update the buttom label
    pButtonSizer->Add(m_pLengthButton, wxSizerFlags(0).Border(wxTOP, 5));

    m_pHeightButton = new wxButton(this,BUTTON_GRAPH_HEIGHT,_T("foo"));
    //m_pHeightButton->SetToolTip(_("# of pixels per Y division"));
    OnButtonHeight(dummy); // update the buttom label
    pButtonSizer->Add(m_pHeightButton);

    m_pModeButton = new wxButton(this,BUTTON_GRAPH_MODE,_T("RA/Dec"));
    m_pModeButton->SetToolTip(_("Toggle RA/Dec vs dx/dy.  Shift-click to change RA/dx color.  Ctrl-click to change Dec/dy color"));
    pButtonSizer->Add(m_pModeButton);

    m_pClearButton = new wxButton(this,BUTTON_GRAPH_CLEAR,_("Clear"));
    m_pClearButton->SetToolTip(_("Clear graph data"));
    pButtonSizer->Add(m_pClearButton);

    wxBoxSizer *pLabelSizer = new wxBoxSizer(wxHORIZONTAL);

    m_pLabel1 = new wxStaticText(this, wxID_ANY, _("RA"));
    m_pLabel1->SetForegroundColour(m_pClient->m_raOrDxColor);
    m_pLabel1->SetBackgroundColour(*wxBLACK);
    pLabelSizer->Add(m_pLabel1, wxSizerFlags().Left());

    m_pLabel2 = new wxStaticText(this, wxID_ANY, _("Dec"));
    m_pLabel2->SetForegroundColour(m_pClient->m_decOrDyColor);
    m_pLabel2->SetBackgroundColour(*wxBLACK);

    pLabelSizer->AddStretchSpacer();
    pLabelSizer->Add(m_pLabel2, wxSizerFlags().Right());

    pButtonSizer->Add(pLabelSizer, wxSizerFlags().Expand());

    m_pClient->m_pOscRMS = new wxStaticText(this, wxID_ANY, _("RMS: 0.00"));
    m_pClient->m_pOscRMS->SetForegroundColour(*wxLIGHT_GREY);
    m_pClient->m_pOscRMS->SetBackgroundColour(*wxBLACK);
    pButtonSizer->Add(m_pClient->m_pOscRMS);

    m_pClient->m_pOscIndex = new wxStaticText(this, wxID_ANY, _("Osc: 0.00"));
    m_pClient->m_pOscIndex->SetForegroundColour(*wxLIGHT_GREY);
    m_pClient->m_pOscIndex->SetBackgroundColour(*wxBLACK);
    pButtonSizer->Add(m_pClient->m_pOscIndex);
#ifdef __WINDOWS__
    int ctl_size = 45;
    int extra_offset = -5;
#else
    int ctl_size = 60;
    int extra_offset = 0;
#endif

    // Ra Aggression
    wxStaticText *raa_label = new wxStaticText(this,wxID_ANY,_T("RA agr"));
    raa_label->SetOwnForegroundColour(*wxWHITE);
#ifdef __WINDOWS__
    raa_label->SetOwnBackgroundColour(*wxBLACK);
#endif

    width = StringWidth(_T("0000"));

    RAA_Ctrl = new wxSpinCtrl(this,GRAPH_RAA, _T(""), wxDefaultPosition,
                                    wxSize(width+25, -1),wxSP_ARROW_KEYS | wxALIGN_RIGHT,
                                    0, 120, 0);

    pControlSizer->AddStretchSpacer();
    pControlSizer->Add(raa_label, wxSizerFlags().Right());
    pControlSizer->AddSpacer(5);
    pControlSizer->Add(RAA_Ctrl, wxSizerFlags().Left());
    pControlSizer->AddSpacer(20);

    // Hysteresis
    wxStaticText *rah_label = new wxStaticText(this,wxID_ANY,_T("RA hys"));
    rah_label->SetOwnForegroundColour(* wxWHITE);
#ifdef __WINDOWS__
    rah_label->SetOwnBackgroundColour(* wxBLACK);
#endif
    width = StringWidth(_T("0000"));

    RAH_Ctrl = new wxSpinCtrl(this,GRAPH_RAH, _T(""), wxDefaultPosition,
                                    wxSize(width+25,-1),wxSP_ARROW_KEYS | wxALIGN_RIGHT,
                                    0, 50, 0);

    pControlSizer->Add(rah_label, wxSizerFlags().Right());
    pControlSizer->AddSpacer(5);
    pControlSizer->Add(RAH_Ctrl, wxSizerFlags().Left());
    pControlSizer->AddSpacer(20);

// Min Motion
    wxStaticText *mm_pLabel = new wxStaticText(this,wxID_ANY,_T("Min mot"));
    mm_pLabel->SetOwnForegroundColour(* wxWHITE);
#ifdef __WINDOWS__
    mm_pLabel->SetOwnBackgroundColour(* wxBLACK);
#endif
    width = StringWidth(_T("00.00"));
    MM_Ctrl = new wxSpinCtrlDouble(this,GRAPH_MM, _T(""), wxDefaultPosition,
                                    wxSize(width+25,-1),wxSP_ARROW_KEYS | wxALIGN_RIGHT,
                                    0,5,0,0.05);

    pControlSizer->Add(mm_pLabel, wxSizerFlags().Right());
    pControlSizer->AddSpacer(5);
    pControlSizer->Add(MM_Ctrl, wxSizerFlags().Left());
    pControlSizer->AddStretchSpacer();

#ifdef BRET_TODO
    // Max RA Dur
    wxStaticText *mrad_label = new wxStaticText(this,wxID_ANY,_T("Mx RA"),wxPoint(315,210),wxSize(ctl_size+10,-1));
    mrad_label->SetOwnForegroundColour(* wxWHITE);
#ifdef __WINDOWS__
    mrad_label->SetOwnBackgroundColour(* wxBLACK);
#endif
    this->MRAD_Ctrl = new wxSpinCtrl(this,GRAPH_MRAD, _T(""),
                                    wxPoint(360,205),wxSize(ctl_size+10,-1),wxSP_ARROW_KEYS,
                                    0,2000,0);
    // Max Dec Dur
    wxStaticText *mdd_label = new wxStaticText(this,wxID_ANY,_T("Mx dec"),wxPoint(425,210),wxSize(ctl_size+10,-1));
    mdd_label->SetOwnForegroundColour(* wxWHITE);
#ifdef __WINDOWS__
    mdd_label->SetOwnBackgroundColour(* wxBLACK);
#endif
    double Max_Dec_Dur = 0;
    this->MDD_Ctrl = new wxSpinCtrl(this,GRAPH_MDD, _T(""),
                                    wxPoint(470,205),wxSize(ctl_size+10,-1),wxSP_ARROW_KEYS,
                                    0,2000, 0);
    // Dec Guide mode
    wxString dec_choices[] = {
        _("Off"),_("Auto"),_("North"),_("South")
    };
    this->DM_Ctrl= new wxChoice(this,GRAPH_DM,wxPoint(535,210+extra_offset),wxSize(ctl_size+15,-1),WXSIZEOF(dec_choices), dec_choices );
    //DM_Ctrl->SetSelection(pMount->GetDecGuideMode());
#endif

    SetSizer(pMainSizer);
    pMainSizer->SetSizeHints(this);
}

GraphLogWindow::~GraphLogWindow()
{
    delete m_pClient;
}

wxColor GraphLogWindow::GetRaOrDxColor(void)
{
    return m_pClient->m_raOrDxColor;
}

wxColor GraphLogWindow::GetDecOrDyColor(void)
{
    return m_pClient->m_decOrDyColor;
}

int GraphLogWindow::StringWidth(wxString string)
{
    int width, height;

    m_pParent->GetTextExtent(string, &width, &height);

    return width;
}

void GraphLogWindow::OnUpdateSpinGuideParams(wxSpinEvent& WXUNUSED(evt)) {
    if (pMount->GetXGuideAlgorithm() == GUIDE_ALGORITHM_HYSTERESIS)
    {
        GuideAlgorithmHysteresis *pHyst = (GuideAlgorithmHysteresis *)pFrame->pGuider;

        pHyst->SetAggression((float) this->RAA_Ctrl->GetValue() / 100.0);
        pHyst->SetHysteresis((float) this->RAH_Ctrl->GetValue() / 100.0);
    }
#ifdef BRET_TODO
    pMount->SetMaxDecDuration(this->MDD_Ctrl->GetValue());
    pMount->SetMaxRaDuration(this->MRAD_Ctrl->GetValue());
#endif
}

void GraphLogWindow::OnUpdateCommandGuideParams(wxCommandEvent& WXUNUSED(evt)) {
#ifdef BRET_TODO
    pMount->SetDecGuideMode(this->DM_Ctrl->GetSelection());
#endif
}

void GraphLogWindow::OnUpdateSpinDGuideParams(wxSpinDoubleEvent& WXUNUSED(evt)) {
    if (pMount->GetYGuideAlgorithm() == GUIDE_ALGORITHM_HYSTERESIS)
    {
        GuideAlgorithmHysteresis *pHyst = (GuideAlgorithmHysteresis *)pFrame->pGuider;

        pHyst->SetMinMove(this->MM_Ctrl->GetValue());
    }
}

void GraphLogWindow::OnButtonMode(wxCommandEvent& WXUNUSED(evt)) {
    wxMouseState mstate = wxGetMouseState();

    if (wxGetKeyState(WXK_SHIFT)) {
        wxColourData cdata;
        cdata.SetColour(m_pClient->m_raOrDxColor);
        wxColourDialog cdialog(this, &cdata);
        if (cdialog.ShowModal() == wxID_OK) {
            cdata = cdialog.GetColourData();
            m_pClient->m_raOrDxColor = cdata.GetColour();
        }
    }
    if (wxGetKeyState(WXK_CONTROL)) {
        wxColourData cdata;
        cdata.SetColour(m_pClient->m_decOrDyColor);
        wxColourDialog cdialog(this, &cdata);
        if (cdialog.ShowModal() == wxID_OK) {
            cdata = cdialog.GetColourData();
            m_pClient->m_decOrDyColor = cdata.GetColour();
        }
    }

    switch (m_pClient->m_mode)
    {
        case GraphLogClientWindow::MODE_RADEC:
            m_pClient->m_mode = GraphLogClientWindow::MODE_DXDY;
            m_pModeButton->SetLabel(_T("dx/dy"));
            break;
        case GraphLogClientWindow::MODE_DXDY:
            m_pClient->m_mode = GraphLogClientWindow::MODE_RADEC;
            m_pModeButton->SetLabel(_T("RA/Dec"));
            break;
    }

    Refresh();
}

void GraphLogWindow::OnButtonLength(wxCommandEvent& WXUNUSED(evt))
{
    m_pClient->m_length *= 2;

    if (m_pClient->m_length > m_pClient->m_maxLength)
    {
            m_pClient->m_length = m_pClient->m_minLength;
    }

    this->m_pLengthButton->SetLabel(wxString::Format(_T("x:%3d"), m_pClient->m_length));
    this->Refresh();
}

void GraphLogWindow::OnButtonHeight(wxCommandEvent& WXUNUSED(evt))
{
    m_pClient->m_height *= 2;

    if (m_pClient->m_height > m_pClient->m_maxHeight)
    {
            m_pClient->m_height = m_pClient->m_minHeight;
    }

    this->m_pHeightButton->SetLabel(wxString::Format(_T("y:+/-%d"), m_pClient->m_height));
    this->Refresh();
}

void GraphLogWindow::SetState(bool is_active) {
    this->m_visible = is_active;
    this->Show(is_active);
    if (is_active)
    {
        if (pMount->GetYGuideAlgorithm() == GUIDE_ALGORITHM_HYSTERESIS)
        {
            GuideAlgorithmHysteresis *pHyst = (GuideAlgorithmHysteresis *)pFrame->pGuider;
            this->RAA_Ctrl->SetValue((int) (pHyst->GetAggression() * 100));
            this->RAH_Ctrl->SetValue((int) (pHyst->GetHysteresis() * 100));
            this->MM_Ctrl->SetValue(pHyst->GetMinMove());
        }
#ifdef BRET_TODO
        this->MDD_Ctrl->SetValue(pMount->GetMaxDecDuration());
        this->MRAD_Ctrl->SetValue(pMount->GetMaxRaDuration());
        this->DM_Ctrl->SetSelection(pMount->GetDecGuideMode());
#endif
        Refresh();
    }
}

void GraphLogWindow::AppendData(float dx, float dy, float RA, float Dec)
{
    m_pClient->AppendData(dx, dy, RA, Dec);

    if (m_visible)
    {
        Refresh();
    }
}

void GraphLogWindow::OnButtonClear(wxCommandEvent& WXUNUSED(evt))
{
    m_pClient->m_nItems = 0;
    Refresh();
}

void GraphLogWindow::OnPaint(wxPaintEvent& WXUNUSED(evt))
{
    wxAutoBufferedPaintDC dc(this);

    dc.SetBackground(*wxBLACK_BRUSH);
    //dc.SetBackground(wxColour(10,0,0));
    dc.Clear();

    switch (m_pClient->m_mode)
    {
        case GraphLogClientWindow::MODE_RADEC:
            m_pLabel1->SetLabel(_("RA"));
            m_pLabel2->SetLabel(_("Dec"));
            break;
        case GraphLogClientWindow::MODE_DXDY:
            m_pLabel1->SetLabel(_("dx"));
            m_pLabel2->SetLabel(_("dy"));
            break;
    }

    if (pFrame->GetSampling() != 1)
    {
        this->m_pHeightButton->SetLabel(wxString::Format(_T("y:+/-%d''"), m_pClient->m_height));
        m_pHeightButton->SetToolTip(_("# of arc-sec per Y division"));
    }
    else
    {
        this->m_pHeightButton->SetLabel(wxString::Format(_T("y:+/-%d"), m_pClient->m_height));
        m_pHeightButton->SetToolTip(_("# of pixels per Y division"));
    }
}

BEGIN_EVENT_TABLE(GraphLogClientWindow, wxWindow)
EVT_PAINT(GraphLogClientWindow::OnPaint)
END_EVENT_TABLE()

GraphLogClientWindow::GraphLogClientWindow(wxWindow *parent) :
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(401,200), wxFULL_REPAINT_ON_RESIZE)
{
    m_nItems = 0;
    m_mode = MODE_RADEC;

    m_raOrDxColor  = wxColour(100,100,255);
    m_decOrDyColor = wxColour(255,0,0);

    int minLength = pConfig->GetInt("/graph/minLength", DefaultMinLength);
    SetMinLength(minLength);

    int maxLength = pConfig->GetInt("/graph/maxLength", DefaultMaxLength);
    SetMaxLength(maxLength);

    int minHeight = pConfig->GetInt("/graph/minHeight", DefaultMinHeight);
    SetMinHeight(minHeight);

    int maxHeight = pConfig->GetInt("/graph/maxHeight", DefaultMaxHeight);
    SetMaxHeight(maxHeight);

    m_length = m_minLength;
    m_height = m_maxHeight;

    m_pHistory = new S_HISTORY[m_maxLength];
}

GraphLogClientWindow::~GraphLogClientWindow(void)
{
    delete [] m_pHistory;
}

bool GraphLogClientWindow::SetMinLength(int minLength)
{
    bool bError = false;

    try
    {
        if (minLength < 1)
        {
            throw ERROR_INFO("minLength < 1");
        }
        m_minLength = minLength;
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
        bError = true;
        m_minLength = DefaultMinLength;
    }

    pConfig->SetInt("/graph/minLength", m_minLength);

    return bError;
}

bool GraphLogClientWindow::SetMaxLength(int maxLength)
{
    bool bError = false;

    try
    {
        if (maxLength <= m_minLength)
        {
            throw ERROR_INFO("maxLength < m_minLength");
        }
        m_maxLength = maxLength;
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
        bError = true;
        m_minLength = DefaultMinLength;
        m_maxLength = DefaultMaxLength;
    }

    pConfig->SetInt("/graph/maxLength", m_maxLength);

    return bError;
}

bool GraphLogClientWindow::SetMinHeight(int minHeight)
{
    bool bError = false;

    try
    {
        if (minHeight < 1)
        {
            throw ERROR_INFO("minHeight < 1");
        }
        m_minHeight = minHeight;
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
        bError = true;
        m_minHeight = DefaultMinHeight;
    }

    pConfig->SetInt("/graph/minHeight", m_minHeight);

    return bError;
}

bool GraphLogClientWindow::SetMaxHeight(int maxHeight)
{
    bool bError = false;

    try
    {
        if (maxHeight <= m_minHeight)
        {
            throw ERROR_INFO("maxHeight < m_minHeight");
        }
        m_maxHeight = maxHeight;
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
        bError = true;
        m_minHeight = DefaultMinHeight;
        m_maxHeight = DefaultMaxHeight;
    }

    pConfig->SetInt("/graph/maxHeight", m_maxHeight);

    return bError;
}

void GraphLogClientWindow::AppendData(float dx, float dy, float RA, float Dec)
{
    const int idx = m_maxLength - 1;

    memmove(m_pHistory, m_pHistory+1, sizeof(m_pHistory[0])*(m_maxLength-1));

    m_pHistory[idx].dx  = dx;
    m_pHistory[idx].dy  = dy;
    m_pHistory[idx].ra  = RA;
    m_pHistory[idx].dec = Dec;

    if (m_nItems < m_maxLength)
    {
        m_nItems++;
    }
}

void GraphLogClientWindow::OnPaint(wxPaintEvent& WXUNUSED(evt))
{
    //wxAutoBufferedPaintDC dc(this);
    wxPaintDC dc(this);

    wxSize size = GetClientSize();
    wxSize center(size.x/2, size.y/2);

    const int leftEdge = 0;
    const int rightEdge = size.x-5;

    const int topEdge = 5;
    const int bottomEdge = size.y-5;

    const int xorig = 0;
    const int yorig = size.y/2;

    int i;

    const int xDivisions = m_length/m_xSamplesPerDivision-1;
    const int xPixelsPerDivision = size.x/2/(xDivisions+1);
    const int yPixelsPerDivision = size.y/2/(m_yDivisions+1);

    const double sampling = pFrame->GetSampling();

    wxPoint *pRaOrDxLine  = NULL;
    wxPoint *pDecOrDyLine = NULL;

    dc.SetBackground(*wxBLACK_BRUSH);
    //dc.SetBackground(wxColour(10,0,0));
    dc.Clear();

    wxPen GreyDashPen;
    GreyDashPen = wxPen(wxColour(200,200,200),1, wxDOT);

    // Draw axes
    dc.SetPen(*wxGREY_PEN);
    dc.DrawLine(center.x,topEdge,center.x,bottomEdge);
    dc.DrawLine(leftEdge,center.y,rightEdge,center.y);

    // draw a box around the client area
    dc.SetPen(*wxGREY_PEN);
    dc.DrawLine(leftEdge, topEdge, rightEdge, topEdge);
    dc.DrawLine(rightEdge, topEdge, rightEdge, bottomEdge);
    dc.DrawLine(rightEdge, bottomEdge, leftEdge, bottomEdge);
    dc.DrawLine(leftEdge, bottomEdge, leftEdge, topEdge);

    // Draw horiz rule (scale is 1 pixel error per 25 pixels)
    dc.SetPen(GreyDashPen);
    for(i=1;i<=m_yDivisions;i++)
    {
        dc.DrawLine(leftEdge,center.y-i*yPixelsPerDivision, rightEdge, center.y-i*yPixelsPerDivision);
        dc.DrawLine(leftEdge,center.y+i*yPixelsPerDivision, rightEdge, center.y+i*yPixelsPerDivision);
    }

    for(i=1;i<=xDivisions;i++)
    {
        dc.DrawLine(center.x-i*xPixelsPerDivision, topEdge, center.x-i*xPixelsPerDivision, bottomEdge);
        dc.DrawLine(center.x+i*xPixelsPerDivision, topEdge, center.x+i*xPixelsPerDivision, bottomEdge);
    }

    // Draw data
    if (m_nItems > 0)
    {
        pRaOrDxLine  = new wxPoint[m_maxLength];
        pDecOrDyLine = new wxPoint[m_maxLength];

        int start_item = m_maxLength;

        if (m_nItems < m_length)
        {
            start_item -= m_nItems;
        }
        else
        {
            start_item -= m_length;
        }

        const double xmag = size.x / (double)m_length;
        const double ymag = yPixelsPerDivision*(double)(m_yDivisions + 1)/(double)m_height;

        for (i=start_item; i<m_maxLength; i++)
        {
            int j=i-start_item;
            S_HISTORY *pSrc = m_pHistory + i;

            switch (m_mode)
            {
            case MODE_RADEC:
                pRaOrDxLine[j] =wxPoint(xorig+(j*xmag),yorig + (int) (pSrc->ra * (double) ymag * sampling));
                pDecOrDyLine[j]=wxPoint(xorig+(j*xmag),yorig + (int) (pSrc->dec * (double) ymag * sampling));
                break;
            case MODE_DXDY:
                pRaOrDxLine[j]=wxPoint(xorig+(j*xmag),yorig + (int) (pSrc->dx * (double) ymag * sampling));
                pDecOrDyLine[j]=wxPoint(xorig+(j*xmag),yorig + (int) (pSrc->dy * (double) ymag * sampling));
                break;
            }
        }

        wxPen raOrDxPen(m_raOrDxColor);
        wxPen decOrDyPen(m_decOrDyColor);

        int plot_length = m_length;

        if (m_length > m_nItems)
        {
            plot_length = m_nItems;
        }
        dc.SetPen(raOrDxPen);
        dc.DrawLines(plot_length,pRaOrDxLine);
        dc.SetPen(decOrDyPen);
        dc.DrawLines(plot_length,pDecOrDyLine);

        // Figure oscillation score
        int same_sides = 0;
        double mean = 0.0;
        for (i = start_item + 1 ; i < m_maxLength ; i++)
        {
            if ( (m_pHistory[i].ra * m_pHistory[i-1].ra) > 0.0)
                same_sides++;
            mean = mean + m_pHistory[i].ra;
        }
        if (m_nItems != start_item)
            mean = mean / (double) (m_nItems - start_item);
        else
            mean = 0.0;
        double RMS = 0.0;
        for (i = start_item + 1; i < m_maxLength; i++)
        {
            double ra = m_pHistory[i].ra;
            RMS = RMS + (ra-mean)*(ra-mean);
        }
        if (m_nItems != start_item)
            RMS = sqrt(RMS/(double) (m_maxLength - start_item));
        else
            RMS = 0.0;

        if (sampling != 1)
            m_pOscRMS->SetLabel(wxString::Format("RMS: %4.2f (%.2f'')", RMS, RMS * sampling));
        else
            m_pOscRMS->SetLabel(wxString::Format("RMS: %4.2f", RMS));

        double osc_index = 0.0;
        if (m_nItems != start_item)
            osc_index= 1.0 - (double) same_sides / (double) (m_maxLength - start_item);

        if ((osc_index > 0.6) || (osc_index < 0.15))
        {
            m_pOscIndex->SetForegroundColour(wxColour(185,20,0));
        }
        else
        {
            m_pOscIndex->SetForegroundColour(*wxLIGHT_GREY);
        }

        if (sampling != 1)
            m_pOscIndex->SetLabel(wxString::Format("Osc: %4.2f (%.2f)", osc_index, osc_index * sampling));
        else
            m_pOscIndex->SetLabel(wxString::Format("Osc: %4.2f", osc_index));

        delete [] pRaOrDxLine;
        delete [] pDecOrDyLine;
    }
}
