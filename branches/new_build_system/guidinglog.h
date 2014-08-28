/*
 *  guidinglog.h
 *  PHD Guiding
 *
 *  Created by Bret McKee
 *  Copyright (c) 2012-2013 Bret McKee
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
 *    Neither the name of Bret McKee, Dad Dog Development,
 *     Craig Stark, Stark Labs nor the names of its
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

#ifndef GUIDINGLOG_INCLUDED
#define GUIDINGLOG_INCLUDED

#include "logger.h"

class Mount;
class Guider;
struct LockPosShiftParams;

struct GuideStepInfo
{
    Mount *mount;
    double time;
    const PHD_Point *cameraOffset;
    const PHD_Point *mountOffset;
    double guideDistanceRA;
    double guideDistanceDec;
    int durationRA;
    int durationDec;
    // TODO: the following two members are GUIDE_DIRECTION, but we have circular
    // dependencies in our header files so cannot use GUIDE_DIRECTION here
    int directionRA;
    int directionDec;
    wxPoint aoPos;
    double starMass;
    double starSNR;
};

class GuidingLog : public Logger
{
    bool m_enabled;
    wxFFile m_file;
    wxString m_fileName;
    bool m_keepFile;

public:
    GuidingLog(bool active=false);
    ~GuidingLog(void);

    bool EnableLogging(void);
    bool EnableLogging(bool enabled);
    bool DisableLogging(void);
    bool IsEnabled(void);
    bool Flush(void);
    void Close(void);

    bool StartCalibration(Mount *pCalibrationMount);
    bool CalibrationFailed(Mount *pCalibrationMount, const wxString& msg);
    bool CalibrationStep(Mount *pCalibrationMount, const wxString& direction, int steps, double dx, double dy, const PHD_Point &xy, double dist);
    bool CalibrationDirectComplete(Mount *pCalibrationMount, const wxString& direction, double angle, double rate);
    bool CalibrationComplete(Mount *pCalibrationMount);

    bool StartGuiding();
    bool GuideStep(const GuideStepInfo& info);

    bool ServerCommand(Guider *guider, const wxString& cmd);
    bool NotifyGuidingDithered(Guider *guider, double dx, double dy);
    bool NotifySetLockPosition(Guider *guider);
    bool NotifyLockShiftParams(const LockPosShiftParams& shiftParams, const PHD_Point& cameraRate);

    bool SetGuidingParam(const wxString& name, double val);
    bool SetGuidingParam(const wxString& name, int val);
    bool SetGuidingParam(const wxString& name, const wxString& val);

    bool ChangeDirLog(const wxString& newdir);

protected:
    bool GuidingHeader(void);
};

#endif
