///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder_SubPic.cpp,v 1.16 2005-02-17 09:31:48 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2003 Gabest
//  http://www.gabest.org
//
///////////////////////////////////////////////////////////////////////////////
//
//  This Program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//   
//  This Program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//   
//  You should have received a copy of the GNU General Public License
//  along with GNU Make; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
//  http://www.gnu.org/copyleft/gpl.html
//
///////////////////////////////////////////////////////////////////////////////
//
// This file was based on the mpeg2dec filter which is part of the MPC
// program see http://sf.net/projects/guliverkli/ for more details
//
// Changes made to files by John Adcock 06/02/04
//  - Removed use of MFC
//  - Replaced use of ATL with YACL
//  - Replaced Baseclasses with FilterLib
//  - Removed DeCSS
//  - Fixed various timestamp issues
//
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2004/12/06 18:05:00  adcockj
// Major improvements to deinterlacing
//
// Revision 1.14  2004/10/28 15:52:24  adcockj
// Moved video output pin code into new class
//
// Revision 1.13  2004/10/26 16:23:44  adcockj
// Improve subpicture performance
//
// Revision 1.12  2004/08/03 08:55:57  adcockj
// Fixes for seeking issues
//
// Revision 1.11  2004/07/28 16:32:34  adcockj
// Fixes Blight's problems from the forum
//
// Revision 1.10  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.9  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.8  2004/04/08 16:41:57  adcockj
// Tidy up subpicture support
//
// Revision 1.7  2004/04/06 16:46:12  adcockj
// DVD Test Annex Compatability fixes
//
// Revision 1.6  2004/03/11 16:52:22  adcockj
// Improved subpicture drawing with different video widths/heights
//
// Revision 1.5  2004/02/25 17:14:02  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.4  2004/02/16 17:25:02  adcockj
// Fix build errors, locking problems and DVD compatability
//
// Revision 1.3  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.2  2004/02/06 16:41:42  adcockj
// Added frame smoothing and forced subs parameters
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MpegDecoder.h"
#include "DSInputPin.h"
#include "DSVideoOutPin.h"

#define PTS2RT(pts) (10000i64*pts/90)
#define ComparePTSWithRt(rt, pts) (abs((int)(rt - 10000i64*pts/90)) <= 90000)

HRESULT CMpegDecoder::SetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData)
{
    bool fRefresh = false;

    switch(dwPropID)
    {
    case AM_PROPERTY_DVDSUBPIC_PALETTE:
        {
            CProtectCode WhileVarInScope(&m_SubPictureLock);
            AM_PROPERTY_SPPAL* pSPPAL = (AM_PROPERTY_SPPAL*)pPropertyData;
            memcpy(m_sppal, pSPPAL->sppal, sizeof(AM_PROPERTY_SPPAL));

            LOG(DBGLOG_FLOW,("new palette\n"));
        }
        break;
    case AM_PROPERTY_DVDSUBPIC_HLI:
        {
            CProtectCode WhileVarInScope(&m_SubPictureLock);
            AM_PROPERTY_SPHLI* pSPHLI = (AM_PROPERTY_SPHLI*)pPropertyData;


            std::list<CHighlight*>::iterator it = m_HighlightList.begin();
            while(it != m_HighlightList.end())
            {
                if((*it)->rtStart >= PTS2RT(pSPHLI->StartPTM) || pSPHLI->HLISS == 0 || pSPHLI->StartPTM == 0xFFFFFFFF)
                {
                    delete *it;
                    m_HighlightList.pop_front();
                    it = m_HighlightList.begin();
                }
                else
                {
                    it++;
                }
            }
                            
            if(pSPHLI->HLISS)
            {
                // switch off any previous highlights
                if(m_HighlightList.size() > 0)
                {
                    if(m_HighlightList.back()->rtStop > PTS2RT(pSPHLI->StartPTM))
                    {
                        m_HighlightList.back()->rtStop = PTS2RT(pSPHLI->StartPTM);
                        if(m_HighlightList.back()->rtStart >= m_HighlightList.back()->rtStop)
                        {
                            m_HighlightList.back()->rtStart = m_HighlightList.back()->rtStop - 1;
                        }
                    }
                }
                
                // add new highlight into list
                CHighlight* NewHighlight = new CHighlight();
                memcpy(&NewHighlight->m_Hi, pSPHLI, sizeof(AM_PROPERTY_SPHLI));
                if(pSPHLI->StartPTM != 0xFFFFFFFF)
                {
                    NewHighlight->rtStart = PTS2RT(pSPHLI->StartPTM);
                }
                else
                {
                    NewHighlight->rtStart = _I64_MIN;
                }
                if(pSPHLI->EndPTM != 0xFFFFFFFF)
                {
                    NewHighlight->rtStop = PTS2RT(pSPHLI->EndPTM);
                }
                else
                {
                    NewHighlight->rtStop = _I64_MAX;
                }

                m_HighlightList.push_back(NewHighlight);
                fRefresh = true;

                LOG(DBGLOG_ALL,("hli: %I64d - %I64d, (%d,%d) - (%d,%d) %d\n", 
                    NewHighlight->rtStart, NewHighlight->rtStop,
                    pSPHLI->StartX, pSPHLI->StartY, pSPHLI->StopX, pSPHLI->StopY, &NewHighlight->m_Hi));
            }
        }
        break;
    case AM_PROPERTY_DVDSUBPIC_COMPOSIT_ON:
        {
            CProtectCode WhileVarInScope(&m_SubPictureLock);
            AM_PROPERTY_COMPOSIT_ON* pCompositOn = (AM_PROPERTY_COMPOSIT_ON*)pPropertyData;
            m_spon = *pCompositOn;
            fRefresh = true;
        }
        break;
    default:
        return E_PROP_ID_UNSUPPORTED;
    }

    if(fRefresh && m_LastPictureWasStill)
    {
        ForceDelivery();
    }

    return S_OK;
}

void CMpegDecoder::ForceDelivery()
{
    CProtectCode WhileVarInScope(&m_DeliverLock);
    int Counter = 6;
    int DroppedFramesBefore;
    int DroppedFramesAfter;
    do
    {
        if(Counter != 6)
        {
            Sleep(0);
            LOG(DBGLOG_FLOW, ("Refresh Image %d\n", Counter));
        }
        else
        {
            LOG(DBGLOG_ALL, ("Refresh Image\n"));
        }
        DroppedFramesBefore = m_VideoOutPin->GetDroppedFrames();
        Deliver(true);
        DroppedFramesAfter = m_VideoOutPin->GetDroppedFrames();
    }
    while(--Counter && DroppedFramesAfter != DroppedFramesBefore);

    if(!Counter)
    {
        LOG(DBGLOG_FLOW, ("!!!!! Can't Refresh Image\n", Counter));
    }
}

HRESULT CMpegDecoder::GetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned)
{
    return E_NOTIMPL;
}

HRESULT CMpegDecoder::SupportPropSetSubPic(DWORD dwPropID, DWORD *pTypeSupport)
{
    switch(dwPropID)
    {
    case AM_PROPERTY_DVDSUBPIC_PALETTE:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    case AM_PROPERTY_DVDSUBPIC_HLI:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    case AM_PROPERTY_DVDSUBPIC_COMPOSIT_ON:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    default:
        return E_PROP_ID_UNSUPPORTED;
    }
    return S_OK;
}

HRESULT CMpegDecoder::ProcessSubPicSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties)
{
    BYTE* pDataIn = pSampleProperties->pbBuffer;
    long len = pSampleProperties->lActual;
    HRESULT hr = S_OK;

    if(*(DWORD*)pDataIn == 0xBA010000) // MEDIATYPE_*_PACK
    {
        len -= 14; 
        pDataIn += 14;
        if(int stuffing = (pDataIn[-1]&7))
        {
            len -= stuffing; 
            pDataIn += stuffing;
        }
    }

    if(len <= 0)
    {
        LOG(DBGLOG_FLOW,("Nothing after packing\n"));
        return S_OK;
    }

    if(*(DWORD*)pDataIn == 0xBD010000)
    {
        if(m_SubpictureInPin->GetMediaType()->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
        {
            len -= 8; 
            pDataIn += 8;
            len -= *pDataIn+1+1; 
            pDataIn += *pDataIn+1+1;
        }
    }

    if(len <= 0) 
    {
        LOG(DBGLOG_FLOW,("Nothing after subpicture\n"));
        return S_OK;
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        CProtectCode WhileVarInScope(&m_SubPictureLock);

        std::list<CSubPicture*>::iterator it = m_SubPicureList.end();
        while(it != m_SubPicureList.begin())
        {
            it--;
            CSubPicture* sp = *it;
            if(sp->rtStop == _I64_MAX && sp->rtStart < pSampleProperties->tStart)
            {
                sp->rtStop = pSampleProperties->tStart;
                break;
            }
        }

        CSubPicture* p = new CSubPicture();
        p->rtStart = pSampleProperties->tStart;
        p->rtStop = _I64_MAX;
        p->pData.resize(len);
        memcpy(&(p->pData[0]), pDataIn, len);

        m_SubPicureList.push_back(p);
    }
    else
    {
        CProtectCode WhileVarInScope(&m_SubPictureLock);
        if(m_SubPicureList.size() > 0)
        {
            CSubPicture* sp = m_SubPicureList.back();
            sp->pData.resize(sp->pData.size() + len);
            memcpy(&sp->pData[0] + sp->pData.size() - len, pDataIn, len);
        }
        else
        {
            LOG(DBGLOG_FLOW,("Hanging subpicture\n"));
        }
    }   

    if(m_SubPicureList.size() > 0)
    {
        AM_PROPERTY_SPHLI sphli;
        DWORD offset1;
        DWORD offset2;
        if(!DecodeSubpic(m_SubPicureList.back(), sphli, offset1, offset2))
        {
            CProtectCode WhileVarInScope(&m_DeliverLock);
            if(m_CurrentPicture && HasSubpicsToRender(m_CurrentPicture->m_rtStart) && m_LastPictureWasStill)
            {
                LOG(DBGLOG_ALL,("Refresh Image\n"));
                ForceDelivery();
            }
        }
    }

    return hr;
}

bool CMpegDecoder::DecodeSubpic(CSubPicture* sp, AM_PROPERTY_SPHLI& sphli, DWORD& offset1, DWORD& offset2)
{
    memset(&sphli, 0, sizeof(sphli));

    sp->fForced = false;

    BYTE* p = &sp->pData[0];

    WORD packetsize = (p[0]<<8)|p[1];
    WORD datasize = (p[2]<<8)|p[3];

    if(packetsize > sp->pData.size() || datasize > packetsize)
        return(false);

    int i, next = datasize;

    #define GetWORD (p[i]<<8)|p[i+1]; i += 2

    do
    {
        i = next;

        int pts = GetWORD;
        next = GetWORD;

        if(next > packetsize || next < datasize)
            return(false);

        for(bool fBreak = false; !fBreak; )
        {
            int len = 0;

            switch(p[i])
            {
                case 0x00: len = 0; break;
                case 0x01: len = 0; break;
                case 0x02: len = 0; break;
                case 0x03: len = 2; break;
                case 0x04: len = 2; break;
                case 0x05: len = 6; break;
                case 0x06: len = 4; break;
                default: len = 0; break;
            }

            if(i+len >= packetsize)
            {
                LOG(DBGLOG_FLOW,("Warning: Wrong subpicture parameter block ending\n"));
                break;
            }

            switch(p[i++])
            {
                case 0x00: // forced start displaying
                    //LOG(DBGLOG_FLOW,("forced start displaying\n"));
                    sp->fForced = true;
                    break;
                case 0x01: // normal start displaying
                    //LOG(DBGLOG_FLOW,("normal start displaying\n"));
                    sp->fForced = false;
                    break;
                case 0x02: // stop displaying
                    //LOG(DBGLOG_FLOW,("stop displaying\n"));
                    sp->rtStop = sp->rtStart + 1024*PTS2RT(pts);
                    break;
                case 0x03:
                    //LOG(DBGLOG_FLOW,("0x03\n"));
                    sphli.ColCon.emph2col = p[i]>>4;
                    sphli.ColCon.emph1col = p[i]&0xf;
                    sphli.ColCon.patcol = p[i+1]>>4;
                    sphli.ColCon.backcol = p[i+1]&0xf;
                    i += 2;
                    break;
                case 0x04:
                    //LOG(DBGLOG_FLOW,("0x04\n"));
                    sphli.ColCon.emph2con = p[i]>>4;
                    sphli.ColCon.emph1con = p[i]&0xf;
                    sphli.ColCon.patcon = p[i+1]>>4;
                    sphli.ColCon.backcon = p[i+1]&0xf;
                    i += 2;
                    break;
                case 0x05:
                    //LOG(DBGLOG_FLOW,("0x05\n"));
                    sphli.StartX = (p[i]<<4) + (p[i+1]>>4);
                    sphli.StopX = ((p[i+1]&0x0f)<<8) + p[i+2]+1;
                    sphli.StartY = (p[i+3]<<4) + (p[i+4]>>4);
                    sphli.StopY = ((p[i+4]&0x0f)<<8) + p[i+5]+1;
                    i += 6;
                    break;
                case 0x06:
                    //LOG(DBGLOG_FLOW,("0x06\n"));
                    offset1 = GetWORD;
                    offset2 = GetWORD;
                    break;
                case 0xff: // end of ctrlblk
                    //LOG(DBGLOG_FLOW,("end of ctrlblk\n"));
                    fBreak = true;
                    continue;
                default: // skip this ctrlblk
                    //LOG(DBGLOG_FLOW,("skip this ctrlblk\n"));
                    fBreak = true;
                    break;
            }
        }
    }
    while(i <= next && i < packetsize);

    return(true);
}

void CMpegDecoder::FlushSubPic()
{
    // clear out all the subpictures
    std::list<CSubPicture*>::iterator it = m_SubPicureList.begin();
    while(it != m_SubPicureList.end())
        delete *it++;
    m_SubPicureList.clear();

    std::list<CHighlight*>::iterator it2 = m_HighlightList.begin();
    while(it2 != m_HighlightList.end())
        delete *it2++;
    m_HighlightList.clear();
}

static __inline BYTE GetNibble(BYTE* p, DWORD* offset, int& nField, int& fAligned)
{
    BYTE ret = (p[offset[nField]] >> (fAligned << 2)) & 0x0f;
    offset[nField] += 1-fAligned;
    fAligned = !fAligned;
    return ret;
}

void CMpegDecoder::DrawPixel(BYTE** yuv, POINT pt, int pitch, BYTE color, BYTE contrast, AM_DVD_YUV* sppal)
{
    if(contrast == 0) return;

    if(m_MpegWidth == 352 && (m_OutputHeight == 240 || m_OutputHeight == 288))
    {
        if ((pt.x & 1) || (pt.y & 1))
        {
            return;
        }
        pt.y = pt.y /2;
        pt.x = max((pt.x - 8)/2, 0);
    }
    else if(m_MpegWidth == 352)
    {
        if ((pt.x & 1))
        {
            return;
        }
        pt.x = max((pt.x - 8)/2, 0);
    }
    else if(m_MpegWidth == 704)
    {
        pt.x = max(pt.x - 8, 0);
    }
    else if(m_PanAndScanDVD && m_OutputWidth == 540)
    {
        pt.x *= 540;
        pt.x /= m_MpegWidth;
        pt.x += (m_MpegWidth - m_OutputWidth) /2;
    }



    BYTE* p = &yuv[0][pt.y*pitch + pt.x];
//  *p = (*p*(15-contrast) + sppal[color].Y*contrast)>>4;
    *p -= (*p - sppal[color].Y) * contrast >> 4;

    if(pt.y&1) return; // since U/V is half res there is no need to overwrite the same line again

    pt.x = (pt.x + 1) / 2;
    pt.y = (pt.y /*+ 1*/) / 2; // only paint the upper field always, don't round it
    pitch /= 2;

    // U/V is exchanged? wierd but looks true when comparing the outputted colors from other decoders

    p = &yuv[1][pt.y*pitch + pt.x];
//  *p = (BYTE)(((((int)*p-0x80)*(15-contrast) + ((int)sppal[color].V-0x80)*contrast) >> 4) + 0x80);
    *p -= (*p - sppal[color].V) * contrast >> 4;

    p = &yuv[2][pt.y*pitch + pt.x];
//  *p = (BYTE)(((((int)*p-0x80)*(15-contrast) + ((int)sppal[color].U-0x80)*contrast) >> 4) + 0x80);
    *p -= (*p - sppal[color].U) * contrast >> 4;

    // Neighter of the blending formulas are accurate (">>4" should be "/15").
    // Even though the second one is a bit worse, since we are scaling the difference only,
    // the error is still not noticable.
}

void CMpegDecoder::DrawPixels(BYTE** yuv, POINT pt, int pitch, int len, BYTE color, 
                                AM_PROPERTY_SPHLI& sphli, RECT& rc,
                                AM_PROPERTY_SPHLI* sphli_hli, RECT& rchli,
                                AM_DVD_YUV* sppal)
{
    if(pt.y < rc.top || pt.y >= rc.bottom) return;
    if(pt.x < rc.left) {len -= rc.left - pt.x; pt.x = rc.left;}
    if(pt.x + len > rc.right) len = rc.right - pt.x;
    if(len <= 0 || pt.x >= rc.right) return;

    BYTE contrast = 0, color_hli, contrast_hli = 0;

    if(sphli_hli) switch(color)
    {
    case 0: color_hli = sphli_hli->ColCon.backcol; contrast_hli = sphli_hli->ColCon.backcon; break;
    case 1: color_hli = sphli_hli->ColCon.patcol; contrast_hli = sphli_hli->ColCon.patcon; break;
    case 2: color_hli = sphli_hli->ColCon.emph1col; contrast_hli = sphli_hli->ColCon.emph1con; break;
    case 3: color_hli = sphli_hli->ColCon.emph2col; contrast_hli = sphli_hli->ColCon.emph2con; break;
    default: ASSERT(0); return;
    }
    
    switch(color)
    {
    case 0: color = sphli.ColCon.backcol; contrast = sphli.ColCon.backcon; break;
    case 1: color = sphli.ColCon.patcol; contrast = sphli.ColCon.patcon; break;
    case 2: color = sphli.ColCon.emph1col; contrast = sphli.ColCon.emph1con; break;
    case 3: color = sphli.ColCon.emph2col; contrast = sphli.ColCon.emph2con; break;
    default: ASSERT(0); return;
    }

    if(contrast == 0)
    {
        if(contrast_hli == 0)
            return;

        if(IsRectEmpty(&rchli))
            return;

        if(pt.y < rchli.top || pt.y >= rchli.bottom 
        || pt.x+len < rchli.left || pt.x >= rchli.right)
            return;
    }

    while(len-- > 0)
    {
        bool hli = sphli_hli && PtInRect(&rchli, pt);
        DrawPixel(yuv, pt, pitch, hli ? color_hli : color, hli ? contrast_hli : contrast, sppal);
        pt.x++;
    }
}

void CMpegDecoder::RenderHighlight(BYTE** p, int w, int h, AM_PROPERTY_SPHLI* sphli_hli)
{
    POINT pt = {sphli_hli->StartX, sphli_hli->StartY};
    for(; pt.y <= sphli_hli->StopY;  pt.y++)
    {
        for(pt.x = sphli_hli->StartX; pt.x <= sphli_hli->StopX;  pt.x++)
        {
            DrawPixel(p, pt, w,sphli_hli->ColCon.patcol, sphli_hli->ColCon.patcon, m_sppal);
        }
    }
}

void CMpegDecoder::RenderSubpic(CSubPicture* sp, BYTE** p, int w, int h, AM_PROPERTY_SPHLI* sphli_hli)
{
    AM_PROPERTY_SPHLI sphli;
    DWORD offset[2];
    if(!DecodeSubpic(sp, sphli, offset[0], offset[1]))
    {
        LOG(DBGLOG_FLOW,("Decoder Error\n"));
        return;
    }

    BYTE* pData = &sp->pData[0];
    RECT rc = {sphli.StartX, sphli.StartY, sphli.StopX, sphli.StopY};
    RECT rchli = {0,0,0,0};

    if(sphli_hli)
    {
        RECT rc_hli = {sphli_hli->StartX, sphli_hli->StartY, sphli_hli->StopX, sphli_hli->StopY};
        IntersectRect(&rchli, &rc, &rc_hli);
    }

    int nField = 0;

    DWORD end[2] = {offset[1], (pData[2]<<8)|pData[3]};

    for(nField = 0; nField < 2; nField++)
    {
        POINT pt = {sphli.StartX, sphli.StartY + nField};
        int fAligned = 1;
        
        while(offset[nField] < end[nField] && pt.y <= sphli.StopY)
        {
            DWORD code;
            DWORD len = 0;

            code = GetNibble(pData, offset, nField, fAligned);
            if(code < 0x004)
            {
                code = (code << 4) | GetNibble(pData, offset, nField, fAligned);
                if(code < 0x0010)
                {
                    code = (code << 4) | GetNibble(pData, offset, nField, fAligned);
                    if(code < 0x0040)
                    {
                        code = (code << 4) | GetNibble(pData, offset, nField, fAligned);
                    }
                }
            }
            if(code != 0)
                code = code;
            len = code >> 2;
            if(len == 0)
            {
                len = rc.right - pt.x;
            }
            DrawPixels(p, pt, w, len, (BYTE)(code & 3), sphli, rc, sphli_hli, rchli, m_sppal);

            //DrawPixels(p, pt, w, rc.right - pt.x, (BYTE)(code & 3), sphli, rc, sphli_hli, rchli, m_sppal);


            pt.x += len;
            if(pt.x >= sphli.StopX)
            {
                if(!fAligned) 
                {
                    GetNibble(pData, offset, nField, fAligned); // align to byte
                }
                pt.y += 2;
                pt.x = sphli.StartX;
            }
        }
    }
}

bool CMpegDecoder::HasSubpicsToRender(REFERENCE_TIME rt)
{
    if(!m_SubpictureInPin->IsConnected()) return false;

    CProtectCode WhileVarInScope(&m_SubPictureLock);

    for(std::list<CSubPicture*>::iterator it = m_SubPicureList.begin(); it != m_SubPicureList.end(); it++)
    {
        CSubPicture* sp = *it;
        if(sp->rtStart <= rt && rt < sp->rtStop && (sp->fForced || m_spon))
        {
            return(true);
        }
    }
    for(std::list<CHighlight*>::iterator it2 = m_HighlightList.begin(); it2 != m_HighlightList.end(); it2++)
    {
        CHighlight* hi = *it2;
        if(hi->rtStart <= rt && rt < hi->rtStop )
        {
            return(true);
        }
    }

    return(false);
}

void CMpegDecoder::ClearOldSubpics(REFERENCE_TIME rt)
{
    if(!m_SubpictureInPin->IsConnected()) return;

    CProtectCode WhileVarInScope(&m_SubPictureLock);

    // remove no longer needed things first
    std::list<CSubPicture*>::iterator it = m_SubPicureList.begin();
    while(it != m_SubPicureList.end() && ((*it)->rtStop <= rt + 90000))
    {
        LOG(DBGLOG_ALL,("DeleteSubpic: %I64d - %I64d - %I64d\n", rt, (*it)->rtStart, (*it)->rtStop));
        delete *it;
        m_SubPicureList.pop_front();
        it = m_SubPicureList.begin();
    }

    std::list<CHighlight*>::iterator it2 = m_HighlightList.begin();
    while(it2 != m_HighlightList.end() && ((*it2)->rtStop <= rt + 90000))
    {
        LOG(DBGLOG_ALL,("DeleteHighlight: %I64d - %I64d - %I64d %d\n", rt, (*it2)->rtStart, (*it2)->rtStop));
        delete *it2;
        m_HighlightList.pop_front();
        it2 = m_HighlightList.begin();
    }
}

void CMpegDecoder::RenderSubpics(REFERENCE_TIME rt, BYTE** p, int w, int h)
{
    CProtectCode WhileVarInScope(&m_SubPictureLock);
    std::list<CSubPicture*>::iterator it = m_SubPicureList.begin();
    while(it != m_SubPicureList.end() && ((*it)->rtStop <= rt + 90000))
    {
        LOG(DBGLOG_ALL,("DeleteSubpic: %I64d - %I64d - %I64d\n", rt, (*it)->rtStart, (*it)->rtStop));
        delete *it;
        m_SubPicureList.pop_front();
        it = m_SubPicureList.begin();
    }

    std::list<CHighlight*>::iterator it2 = m_HighlightList.begin();
    while(it2 != m_HighlightList.end() && ((*it2)->rtStop <= rt + 90000))
    {
        LOG(DBGLOG_ALL,("DeleteHighlight: %I64d - %I64d - %I64d\n", rt, (*it2)->rtStart, (*it2)->rtStop));
        delete *it2;
        m_HighlightList.pop_front();
        it2 = m_HighlightList.begin();
    }

    AM_PROPERTY_SPHLI* sphli_hli = NULL;
    if(m_HighlightList.size() > 0)
    {
        CHighlight* Highlight = m_HighlightList.front();
        if(Highlight->rtStart <= rt + 90000 && rt < Highlight->rtStop)
        {
            sphli_hli = &Highlight->m_Hi;
        }
    }

    bool pDone = false;
    while(it != m_SubPicureList.end())
    {
        CSubPicture* sp = *it;
        if(sp->rtStart <= rt + 90000 && rt < sp->rtStop
        && (m_spon || sp->fForced && (GetParamBool(DISPLAYFORCEDSUBS) || sphli_hli)))
        {
            RenderSubpic(sp, p, w, h, sphli_hli);
            LOG(DBGLOG_ALL,("RenderSubpic: %I64d - %I64d - %I64d %d\n", rt, sp->rtStart, sp->rtStop, sphli_hli));
            pDone = true;
        }
        it++;
    }
    if(pDone == false && sphli_hli)
    {
        //RenderHighlight(p, w, h, sphli_hli);
        LOG(DBGLOG_ALL,("RenderHighlight commented out: %I64d\n", rt));
    }
}


CMpegDecoder::CSubPicture::CSubPicture()
{
    rtStart = _I64_MIN;
    rtStop = _I64_MAX;
    fForced = false;
}

CMpegDecoder::CSubPicture::~CSubPicture()
{
    pData.clear();
}

CMpegDecoder::CHighlight::CHighlight()
{
    rtStart = _I64_MIN;
    rtStop = _I64_MAX;
}

CMpegDecoder::CHighlight::~CHighlight()
{
}




