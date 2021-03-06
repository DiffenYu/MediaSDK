// Copyright (c) 2017 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "mfx_common.h"

#if defined (MFX_ENABLE_VPP)
#if defined (MFX_VA_LINUX)

#ifndef __MFX_VPP_VAAPI
#define __MFX_VPP_VAAPI

#include "umc_va_base.h"
#include "mfx_vpp_interface.h"
#include "mfx_platform_headers.h"

#include <va/va.h>
#include <va/va_vpp.h>

#include <assert.h>
#include <set>

#define MFX_CHECK_WITH_ASSERT(EXPR, ERR) {assert(EXPR); MFX_CHECK(EXPR,ERR); }

namespace MfxHwVideoProcessing
{
    class VAAPIVideoProcessing : public DriverVideoProcessing
    {
    public:

        VAAPIVideoProcessing();
        
        virtual ~VAAPIVideoProcessing();

        virtual mfxStatus CreateDevice(VideoCORE * core, mfxVideoParam *pParams, bool isTemporal = false);

        virtual mfxStatus ReconfigDevice(mfxU32 /*indx*/) { return MFX_ERR_NONE; }

        virtual mfxStatus DestroyDevice( void );

        virtual mfxStatus Register(mfxHDLPair* pSurfaces, 
                                   mfxU32 num, 
                                   BOOL bRegister);

        virtual mfxStatus QueryTaskStatus(mfxU32 taskIndex);

        virtual mfxStatus QueryCapabilities( mfxVppCaps& caps );

        virtual mfxStatus QueryVariance(
            mfxU32 frameIndex,
            std::vector<mfxU32> &variance) { return MFX_ERR_UNSUPPORTED; }

        virtual BOOL IsRunning() { return m_bRunning; }

        virtual mfxStatus Execute(mfxExecuteParams *pParams);

        virtual mfxStatus Execute_Composition(mfxExecuteParams *pParams);

        virtual mfxStatus Execute_Composition_VideoWall(mfxExecuteParams *pParams);

        virtual mfxStatus Execute_FakeOutput(mfxExecuteParams *pParams);

    private:


        typedef struct _compositionStreamElement
        {
            mfxU16 index;
            BOOL   active;
            mfxU16 x;
            mfxU16 y;
            _compositionStreamElement()
                : index(0)
                , active(false) 
                , x(0)
                , y(0)
            {};
        } compStreamElem;

        /* Small structure to keep video wall settings. */
        struct _videoWallPars
        {
            /* Width of the single stream in video wall.
             * All streams must have the same width */
            mfxU16 elemWidth;
            /* Height of the single stream in video wall.
             * All streams must have the same width */
            mfxU16 elemHeight;
            /* Number of the streams on X-axis */
            mfxU16 numX;
            /* Number of the streams on Y-axis */
            mfxU16 numY;

            /* Number of tiles needed. Each tile takes 8 streams at maximum
             * That's current driver limitation */
            mfxU16 tiles;
            std::map<mfxU16, compStreamElem> layout;
        } m_videoWallParams;

        BOOL    isVideoWall(mfxExecuteParams *pParams);

        BOOL m_bRunning;
        BOOL m_bVideoWallMode;

        VideoCORE* m_core;

        VADisplay   m_vaDisplay;
        VAConfigID  m_vaConfig;
        VAContextID m_vaContextVPP;

        VAProcFilterCap m_denoiseCaps;
        VAProcFilterCap m_detailCaps;

        VAProcPipelineCaps           m_pipelineCaps;
        VAProcFilterCapColorBalance  m_procampCaps[VAProcColorBalanceCount];
        VAProcFilterCapDeinterlacing m_deinterlacingCaps[VAProcDeinterlacingCount];
#ifdef MFX_ENABLE_VPP_FRC
        VAProcFilterCapFrameRateConversion m_frcCaps[2]; /* only two modes, 24p->60p and 30p->60p */
#endif

        VABufferID m_denoiseFilterID;
        VABufferID m_detailFilterID;
        VABufferID m_deintFilterID;
        VABufferID m_procampFilterID;
        VABufferID m_frcFilterID;
        mfxU32     m_refCountForADI;
        BOOL       m_bFakeOutputEnabled;
        VASurfaceID m_refForFRC[5];
        mfxU32 m_frcCyclicCounter;

        VABufferID m_filterBufs[VAProcFilterCount];
        mfxU32 m_numFilterBufs;

        std::vector<VAProcPipelineParameterBuffer> m_pipelineParam;
        std::vector<VABufferID> m_pipelineParamID;

        std::set<mfxU32> m_cachedReadyTaskIndex;

        typedef struct 
        {
            VASurfaceID surface;
            mfxU32 number;
        } ExtVASurface;

        VASurfaceID* m_primarySurface4Composition ;

        std::vector<ExtVASurface> m_feedbackCache; 

        UMC::Mutex m_guard;

        mfxStatus Init( _mfxPlatformAccelerationService* pVADisplay, mfxVideoParam *pParams);

        mfxStatus Close( void );

        mfxStatus RemoveBufferFromPipe(VABufferID);
    };

}; // namespace

#endif //__MFX_VPP_VAAPI
#endif // MFX_VA_LINUX
#endif // MFX_ENABLE_VPP

/* EOF */
