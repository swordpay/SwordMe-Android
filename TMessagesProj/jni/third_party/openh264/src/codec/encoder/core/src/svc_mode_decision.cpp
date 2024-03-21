/*!
 * \copy
 *     Copyright (c)  2009-2013, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * \file    svc_mode_decision.c
 *
 * \brief Algorithmetic MD for:
 * - multi-spatial Enhancement Layer MD;
 * - Scrolling PSkip Decision for screen content
 *
 * \date    2009.7.29
 *

 **************************************************************************************
 */
#include "mv_pred.h"
#include "ls_defines.h"
#include "svc_base_layer_md.h"
#include "svc_mode_decision.h"

namespace WelsEnc {

// MD for enhancement layers
//////////////
void WelsMdSpatialelInterMbIlfmdNoilp (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* pSlice,
                                       SMB* pCurMb, const Mb_Type kuiRefMbType) {
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;
  SMbCache* pMbCache = &pSlice->sMbCacheInfo;

  const uint32_t kuiNeighborAvail = pCurMb->uiNeighborAvail;
  const int32_t kiMbWidth = pCurDqLayer->iMbWidth;
  const  SMB* kpTopMb = pCurMb - kiMbWidth;
  const bool kbMbLeftAvailPskip = ((kuiNeighborAvail & LEFT_MB_POS) ? IS_SKIP ((pCurMb - 1)->uiMbType) : false);
  const bool kbMbTopAvailPskip  = ((kuiNeighborAvail & TOP_MB_POS) ? IS_SKIP (kpTopMb->uiMbType) : false);
  const bool kbMbTopLeftAvailPskip  = ((kuiNeighborAvail & TOPLEFT_MB_POS) ? IS_SKIP ((kpTopMb - 1)->uiMbType) : false);
  const bool kbMbTopRightAvailPskip = ((kuiNeighborAvail & TOPRIGHT_MB_POS) ? IS_SKIP ((kpTopMb + 1)->uiMbType) : false);

  bool bTrySkip  = kbMbLeftAvailPskip | kbMbTopAvailPskip | kbMbTopLeftAvailPskip | kbMbTopRightAvailPskip;
  bool bKeepSkip = kbMbLeftAvailPskip & kbMbTopAvailPskip & kbMbTopRightAvailPskip;
  bool bSkip = false;

  if (pEncCtx->pFuncList->pfInterMdBackgroundDecision (pEncCtx, pWelsMd, pSlice, pCurMb, pMbCache, &bKeepSkip)) {
    return;
  }

  bSkip = WelsMdInterJudgePskip (pEncCtx, pWelsMd, pSlice, pCurMb, pMbCache, bTrySkip);

  if (bSkip && bKeepSkip) {
    WelsMdInterDecidedPskip (pEncCtx,  pSlice,  pCurMb, pMbCache);
    return;
  }

  if (! IS_SVC_INTRA (kuiRefMbType)) {
    if (!bSkip) {
      PredictSad (pMbCache->sMvComponents.iRefIndexCache, pMbCache->iSadCost, 0, &pWelsMd->iSadPredMb);

      pWelsMd->iCostLuma = WelsMdP16x16 (pEncCtx->pFuncList, pCurDqLayer, pWelsMd, pSlice, pCurMb);
      pCurMb->uiMbType = MB_TYPE_16x16;
    }

    WelsMdInterSecondaryModesEnc (pEncCtx, pWelsMd, pSlice, pCurMb, pMbCache, bSkip);
  } else { //BLMODE == SVC_INTRA

    const int32_t kiCostI16x16 = WelsMdI16x16 (pEncCtx->pFuncList, pEncCtx->pCurDqLayer, pMbCache, pWelsMd->iLambda);
    if (bSkip && (pWelsMd->iCostLuma <= kiCostI16x16)) {
      WelsMdInterDecidedPskip (pEncCtx,  pSlice,  pCurMb, pMbCache);
    } else {
      pWelsMd->iCostLuma = kiCostI16x16;
      pCurMb->uiMbType = MB_TYPE_INTRA16x16;

      WelsMdIntraSecondaryModesEnc (pEncCtx, pWelsMd, pCurMb, pMbCache);
    }
  }
}



void WelsMdInterMbEnhancelayer (sWelsEncCtx* pEncCtx, SWelsMD* pMd, SSlice* pSlice, SMB* pCurMb, SMbCache* pMbCache) {
  SDqLayer* pCurLayer                   = pEncCtx->pCurDqLayer;
  SWelsMD* pWelsMd                      = (SWelsMD*)pMd;
  const SMB* kpInterLayerRefMb          = GetRefMb (pCurLayer, pCurMb);
  const Mb_Type kuiInterLayerRefMbType  = kpInterLayerRefMb->uiMbType;

  SetMvBaseEnhancelayer (pWelsMd, pCurMb,
                         kpInterLayerRefMb); // initial sMvBase here only when pRef mb type is inter, if not sMvBase will be not used!

  WelsMdSpatialelInterMbIlfmdNoilp (pEncCtx, pWelsMd, pSlice, pCurMb, kuiInterLayerRefMbType); //MD process
}

SMB* GetRefMb (SDqLayer* pCurLayer, SMB* pCurMb) {
  const SDqLayer*  kpRefLayer = pCurLayer->pRefLayer;
  const int32_t  kiRefMbIdx = (pCurMb->iMbY >> 1) * kpRefLayer->iMbWidth + (pCurMb->iMbX >>
                              1); //because current lower layer is half size on both vertical and horizontal
  return (&kpRefLayer->sMbDataP[kiRefMbIdx]);
}

void SetMvBaseEnhancelayer (SWelsMD* pMd, SMB* pCurMb, const SMB* kpRefMb) {
  const Mb_Type kuiRefMbType = kpRefMb->uiMbType;

  if (! IS_SVC_INTRA (kuiRefMbType)) {
    SMVUnitXY sMv;
    int32_t iRefMbPartIdx = ((pCurMb->iMbY & 0x01) << 1) + (pCurMb->iMbX & 0x01); //may be need modified
    int32_t iScan4RefPartIdx = g_kuiMbCountScan4Idx[ (iRefMbPartIdx << 2)];
    sMv.iMvX = kpRefMb->sMv[iScan4RefPartIdx].iMvX * (1 << 1);
    sMv.iMvY = kpRefMb->sMv[iScan4RefPartIdx].iMvY * (1 << 1);

    pMd->sMe.sMe16x16.sMvBase = sMv;

    pMd->sMe.sMe8x8[0].sMvBase =
      pMd->sMe.sMe8x8[1].sMvBase =
        pMd->sMe.sMe8x8[2].sMvBase =
          pMd->sMe.sMe8x8[3].sMvBase = sMv;

    pMd->sMe.sMe16x8[0].sMvBase =
      pMd->sMe.sMe16x8[1].sMvBase =
        pMd->sMe.sMe8x16[0].sMvBase =
          pMd->sMe.sMe8x16[1].sMvBase = sMv;
  }
}

// MD for Background decision
//////////////
//////
//  try the BGD Pskip
//////
inline int32_t GetChromaCost (PSampleSadSatdCostFunc* pCalculateFunc,
                              uint8_t* pSrcChroma, int32_t iSrcStride, uint8_t* pRefChroma, int32_t iRefStride) {
  return pCalculateFunc[BLOCK_8x8] (pSrcChroma, iSrcStride, pRefChroma, iRefStride);
}
inline bool IsCostLessEqualSkipCost (int32_t iCurCost, const int32_t iPredPskipSad, const int32_t iRefMbType,
                                     const SPicture* pRef, const int32_t iMbXy,  const int32_t iSmallestInvisibleTh) {
  return ((iPredPskipSad > iSmallestInvisibleTh && iCurCost >= iPredPskipSad)  ||
          (pRef->iPictureType == P_SLICE     &&
           iRefMbType == MB_TYPE_SKIP    &&
           pRef->pMbSkipSad[iMbXy] > iSmallestInvisibleTh &&
           iCurCost >= (pRef->pMbSkipSad[iMbXy])));
}
bool CheckChromaCost (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SMbCache* pMbCache, const int32_t iCurMbXy) {
#define KNOWN_CHROMA_TOO_LARGE 640
#define SMALLEST_INVISIBLE 128 //2*64, 2 in pixel maybe the smallest not visible for luma

  PSampleSadSatdCostFunc* pSad = pEncCtx->pFuncList->sSampleDealingFuncs.pfSampleSad;
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;

  uint8_t* pCbEnc = pMbCache->SPicData.pEncMb[1];
  uint8_t* pCrEnc = pMbCache->SPicData.pEncMb[2];
  uint8_t* pCbRef = pMbCache->SPicData.pRefMb[1];
  uint8_t* pCrRef = pMbCache->SPicData.pRefMb[2];

  const int32_t iCbEncStride         = pCurDqLayer->iEncStride[1];
  const int32_t iCrEncStride         = pCurDqLayer->iEncStride[2];
  const int32_t iChromaRefStride     = pCurDqLayer->pRefPic->iLineSize[1];

  const int32_t iCbSad = GetChromaCost (pSad, pCbEnc, iCbEncStride, pCbRef, iChromaRefStride);
  const int32_t iCrSad = GetChromaCost (pSad, pCrEnc, iCrEncStride, pCrRef, iChromaRefStride);












  const bool bChromaTooLarge = (iCbSad > KNOWN_CHROMA_TOO_LARGE || iCrSad > KNOWN_CHROMA_TOO_LARGE);

  const int32_t iChromaSad = iCbSad + iCrSad;
  PredictSadSkip (pMbCache->sMvComponents.iRefIndexCache, pMbCache->bMbTypeSkip, pMbCache->iSadCostSkip, 0,
                  & (pWelsMd->iSadPredSkip));
  const bool bChromaCostCannotSkip = IsCostLessEqualSkipCost (iChromaSad, pWelsMd->iSadPredSkip, pMbCache->uiRefMbType,
                                     pCurDqLayer->pRefPic, iCurMbXy, SMALLEST_INVISIBLE);

  return (!bChromaCostCannotSkip && !bChromaTooLarge);
}

bool WelsMdInterJudgeBGDPskip (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* pSlice, SMB* pCurMb, SMbCache* pMbCache,
                               bool* bKeepSkip) {
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;

  const int32_t kiRefMbQp = pCurDqLayer->pRefPic->pRefMbQp[pCurMb->iMbXY];
  const int32_t kiCurMbQp = pCurMb->uiLumaQp;// unsigned -> signed
  int8_t* pVaaBgMbFlag    = pEncCtx->pVaa->pVaaBackgroundMbFlag + pCurMb->iMbXY;

  const int32_t kiMbWidth = pCurDqLayer->iMbWidth;

  *bKeepSkip = (*bKeepSkip) &&
               ((!pVaaBgMbFlag[-1]) &&
                (!pVaaBgMbFlag[-kiMbWidth]) &&
                (!pVaaBgMbFlag[-kiMbWidth + 1]));

  if (
    *pVaaBgMbFlag
    && !IS_INTRA (pMbCache->uiRefMbType)
    && (kiRefMbQp - kiCurMbQp <= DELTA_QP_BGD_THD || kiRefMbQp <= 26)
  ) {










    if (CheckChromaCost (pEncCtx, pWelsMd, pMbCache, pCurMb->iMbXY)) {
      SMVUnitXY sVaaPredSkipMv = { 0 };
      PredSkipMv (pMbCache, &sVaaPredSkipMv);
      WelsMdBackgroundMbEnc (pEncCtx, pWelsMd, pCurMb, pMbCache, pSlice, (LD32 (&sVaaPredSkipMv) == 0));
      return true;
    }
  }

  return false;
}

bool WelsMdInterJudgeBGDPskipFalse (sWelsEncCtx* pCtx, SWelsMD* pMd, SSlice* pSlice, SMB* pCurMb, SMbCache* pMbCache,
                                    bool* bKeepSkip) {
  return false;
}

//  update BGD related info
//////
void WelsMdUpdateBGDInfo (SDqLayer* pCurLayer,  SMB* pCurMb, const bool bCollocatedPredFlag,
                          const int32_t iRefPictureType) {
  uint8_t* pTargetRefMbQpList = (pCurLayer->pDecPic->pRefMbQp);
  const int32_t kiMbXY = pCurMb->iMbXY;

  if (pCurMb->uiCbp || I_SLICE == iRefPictureType || 0 == bCollocatedPredFlag) {
    pTargetRefMbQpList[kiMbXY] = pCurMb->uiLumaQp;
  } else { //unchange, do not need to evaluation?
    uint8_t* pRefPicRefMbQpList = (pCurLayer->pRefPic->pRefMbQp);
    pTargetRefMbQpList[kiMbXY] = pRefPicRefMbQpList[kiMbXY];
  }

  if (pCurMb->uiMbType == MB_TYPE_BACKGROUND) {
    pCurMb->uiMbType = MB_TYPE_SKIP;
  }
}

void WelsMdUpdateBGDInfoNULL (SDqLayer* pCurLayer, SMB* pCurMb, const bool bCollocatedPredFlag,
                              const int32_t iRefPictureType) {
  WelsMdUpdateBGDInfo (pCurLayer, pCurMb, bCollocatedPredFlag, iRefPictureType);
}

// MD for screen contents
//////////////
inline bool IsMbStatic (int32_t* pBlockType, EStaticBlockIdc eType) {
  return (pBlockType != NULL &&
          eType == pBlockType[0] &&
          eType == pBlockType[1] &&
          eType == pBlockType[2] &&
          eType == pBlockType[3]);
}
inline bool IsMbCollocatedStatic (int32_t* pBlockType) {
  return IsMbStatic (pBlockType, COLLOCATED_STATIC);
}

inline bool IsMbScrolledStatic (int32_t* pBlockType) {
  return IsMbStatic (pBlockType, SCROLLED_STATIC);
}

inline int32_t CalUVSadCost (SWelsFuncPtrList* pFunc, uint8_t* pEncOri, int32_t iStrideUV, uint8_t* pRefOri,
                             int32_t iRefLineSize) {
  return pFunc->sSampleDealingFuncs.pfSampleSad[BLOCK_8x8] (pEncOri, iStrideUV, pRefOri, iRefLineSize);
}

inline bool CheckBorder (int32_t iMbX, int32_t iMbY, int32_t iScrollMvX, int32_t iScrollMvY, int32_t iMbWidth,
                         int32_t iMbHeight) {
  return ((iMbX << 4) + iScrollMvX < 0 ||
          (iMbX << 4) + iScrollMvX > (iMbWidth - 1) << 4 ||
          (iMbY << 4) + iScrollMvY < 0 ||
          (iMbY << 4) + iScrollMvY > (iMbHeight - 1) << 4
         ); //border check for safety
}


bool JudgeStaticSkip (sWelsEncCtx* pEncCtx, SMB* pCurMb, SMbCache* pMbCache, SWelsMD* pWelsMd) {
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;
  const int32_t kiMbX = pCurMb->iMbX;
  const int32_t kiMbY = pCurMb->iMbY;

  bool bTryStaticSkip = IsMbCollocatedStatic (pWelsMd->iBlock8x8StaticIdc);
  if (bTryStaticSkip) {
    int32_t iStrideUV, iOffsetUV;
    SWelsFuncPtrList* pFunc = pEncCtx->pFuncList;
    SPicture* pRefOri = pCurDqLayer->pRefOri[0];
    if (pRefOri != NULL) {
      iStrideUV = pCurDqLayer->iEncStride[1];
      iOffsetUV = (kiMbX + kiMbY * iStrideUV) << 3;

      int32_t iSadCostCb = CalUVSadCost (pFunc, pMbCache->SPicData.pEncMb[1], iStrideUV, pRefOri->pData[1] + iOffsetUV,
                                         pRefOri->iLineSize[1]);
      if (iSadCostCb == 0) {
        int32_t iSadCostCr = CalUVSadCost (pFunc, pMbCache->SPicData.pEncMb[2], iStrideUV, pRefOri->pData[2] + iOffsetUV,
                                           pRefOri->iLineSize[1]);
        bTryStaticSkip = (0 == iSadCostCr);
      } else bTryStaticSkip = false;
    } else {
      bTryStaticSkip = false;
    }
  }
  return bTryStaticSkip;
}

bool JudgeScrollSkip (sWelsEncCtx* pEncCtx, SMB* pCurMb, SMbCache* pMbCache, SWelsMD* pWelsMd) {
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;
  const int32_t kiMbX = pCurMb->iMbX;
  const int32_t kiMbY = pCurMb->iMbY;
  const int32_t kiMbWidth = pCurDqLayer->iMbWidth;
  const int32_t kiMbHeight = pCurDqLayer->iMbHeight;

  SVAAFrameInfoExt_t* pVaaExt = static_cast<SVAAFrameInfoExt_t*> (pEncCtx->pVaa);

  bool bTryScrollSkip = false;

  if (pVaaExt->sScrollDetectInfo.bScrollDetectFlag)
    bTryScrollSkip = IsMbScrolledStatic (pWelsMd->iBlock8x8StaticIdc);
  else return 0;

  if (bTryScrollSkip) {
    int32_t iStrideUV, iOffsetUV;
    SWelsFuncPtrList* pFunc = pEncCtx->pFuncList;
    SPicture* pRefOri = pCurDqLayer->pRefOri[0];
    if (pRefOri != NULL) {
      int32_t iScrollMvX = pVaaExt->sScrollDetectInfo.iScrollMvX;
      int32_t iScrollMvY = pVaaExt->sScrollDetectInfo.iScrollMvY;
      if (CheckBorder (kiMbX, kiMbY, iScrollMvX, iScrollMvY, kiMbWidth, kiMbHeight)) {
        bTryScrollSkip =  false;
      } else {
        iStrideUV = pCurDqLayer->iEncStride[1];
        iOffsetUV = (kiMbX << 3) + (iScrollMvX >> 1) + ((kiMbY << 3) + (iScrollMvY >> 1)) * iStrideUV;

        int32_t iSadCostCb = CalUVSadCost (pFunc, pMbCache->SPicData.pEncMb[1], iStrideUV, pRefOri->pData[1] + iOffsetUV,
                                           pRefOri->iLineSize[1]);
        if (iSadCostCb == 0) {
          int32_t iSadCostCr = CalUVSadCost (pFunc, pMbCache->SPicData.pEncMb[2], iStrideUV, pRefOri->pData[2] + iOffsetUV,
                                             pRefOri->iLineSize[1]);
          bTryScrollSkip = (0 == iSadCostCr);
        } else bTryScrollSkip = false;
      }
    }
  }
  return bTryScrollSkip;
}

void SvcMdSCDMbEnc (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SMB* pCurMb, SMbCache* pMbCache, SSlice* pSlice,
                    bool bQpSimilarFlag,
                    bool bMbSkipFlag, SMVUnitXY sCurMbMv[], ESkipModes eSkipMode) {
  SDqLayer* pCurDqLayer         = pEncCtx->pCurDqLayer;
  SWelsFuncPtrList* pFunc       = pEncCtx->pFuncList;
  SMVUnitXY sMvp = { 0};
  ST16 (&sMvp.iMvX, sCurMbMv[eSkipMode].iMvX);
  ST16 (&sMvp.iMvY, sCurMbMv[eSkipMode].iMvY);
  uint8_t* pRefLuma             = pMbCache->SPicData.pRefMb[0];
  uint8_t* pRefCb               = pMbCache->SPicData.pRefMb[1];
  uint8_t* pRefCr               = pMbCache->SPicData.pRefMb[2];
  int32_t iLineSizeY            = pCurDqLayer->pRefPic->iLineSize[0];
  int32_t iLineSizeUV           = pCurDqLayer->pRefPic->iLineSize[1];
  uint8_t* pDstLuma             = pMbCache->pSkipMb;
  uint8_t* pDstCb               = pMbCache->pSkipMb + 256;
  uint8_t* pDstCr               = pMbCache->pSkipMb + 256 + 64;

  const int32_t iOffsetY  = (sCurMbMv[eSkipMode].iMvX >> 2) + (sCurMbMv[eSkipMode].iMvY >> 2) * iLineSizeY;
  const int32_t iOffsetUV = (sCurMbMv[eSkipMode].iMvX >> 3) + (sCurMbMv[eSkipMode].iMvY >> 3) * iLineSizeUV;

  if (!bQpSimilarFlag || !bMbSkipFlag) {
    pDstLuma = pMbCache->pMemPredLuma;
    pDstCb   = pMbCache->pMemPredChroma;
    pDstCr   = pMbCache->pMemPredChroma + 64;
  }

  pFunc->sMcFuncs.pMcLumaFunc (pRefLuma + iOffsetY, iLineSizeY, pDstLuma, 16, 0, 0, 16, 16);
  pFunc->sMcFuncs.pMcChromaFunc (pRefCb + iOffsetUV, iLineSizeUV, pDstCb, 8, sMvp.iMvX, sMvp.iMvY, 8, 8);
  pFunc->sMcFuncs.pMcChromaFunc (pRefCr + iOffsetUV, iLineSizeUV, pDstCr, 8, sMvp.iMvX, sMvp.iMvY, 8, 8);

  pCurMb->uiCbp = 0;
  pWelsMd->iCostLuma = 0;
  pCurMb->pSadCost[0] = pFunc->sSampleDealingFuncs.pfSampleSad[BLOCK_16x16] (pMbCache->SPicData.pEncMb[0],
                        pCurDqLayer->iEncStride[0], pRefLuma + iOffsetY, iLineSizeY);

  pWelsMd->iCostSkipMb = pCurMb->pSadCost[0];

  ST16 (& (pCurMb->sP16x16Mv.iMvX), sCurMbMv[eSkipMode].iMvX);
  ST16 (& (pCurMb->sP16x16Mv.iMvY), sCurMbMv[eSkipMode].iMvY);

  ST16 (& (pCurDqLayer->pDecPic->sMvList[pCurMb->iMbXY].iMvX), sCurMbMv[eSkipMode].iMvX);
  ST16 (& (pCurDqLayer->pDecPic->sMvList[pCurMb->iMbXY].iMvY), sCurMbMv[eSkipMode].iMvY);

  if (bQpSimilarFlag && bMbSkipFlag) {

    ST32 (pCurMb->pRefIndex, 0);
    pFunc->pfUpdateMbMv (pCurMb->sMv, sMvp);
    pCurMb->uiMbType = MB_TYPE_SKIP;
    WelsRecPskip (pCurDqLayer, pEncCtx->pFuncList, pCurMb, pMbCache);
    WelsMdInterUpdatePskip (pCurDqLayer, pSlice, pCurMb, pMbCache);
    return;
  }

  pCurMb->uiMbType = MB_TYPE_16x16;

  pWelsMd->sMe.sMe16x16.sMv.iMvX = sCurMbMv[eSkipMode].iMvX;
  pWelsMd->sMe.sMe16x16.sMv.iMvY = sCurMbMv[eSkipMode].iMvY;
  PredMv (&pMbCache->sMvComponents, 0, 4, 0, &pWelsMd->sMe.sMe16x16.sMvp);
  pMbCache->sMbMvp[0] = pWelsMd->sMe.sMe16x16.sMvp;

  UpdateP16x16MotionInfo (pMbCache, pCurMb, 0, &pWelsMd->sMe.sMe16x16.sMv);

  if (pWelsMd->bMdUsingSad)
    pWelsMd->iCostLuma = pCurMb->pSadCost[0];
  else
    pWelsMd->iCostLuma = pFunc->sSampleDealingFuncs.pfSampleSad[BLOCK_16x16] (pMbCache->SPicData.pEncMb[0],
                         pCurDqLayer->iEncStride[0], pRefLuma, iLineSizeY);

  WelsInterMbEncode (pEncCtx, pSlice, pCurMb);
  WelsPMbChromaEncode (pEncCtx, pSlice, pCurMb);

  pFunc->pfCopy16x16Aligned (pMbCache->SPicData.pCsMb[0], pCurDqLayer->iCsStride[0], pMbCache->pMemPredLuma, 16);
  pFunc->pfCopy8x8Aligned (pMbCache->SPicData.pCsMb[1], pCurDqLayer->iCsStride[1], pMbCache->pMemPredChroma, 8);
  pFunc->pfCopy8x8Aligned (pMbCache->SPicData.pCsMb[2], pCurDqLayer->iCsStride[1], pMbCache->pMemPredChroma + 64, 8);
}

bool MdInterSCDPskipProcess (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* pSlice, SMB* pCurMb, SMbCache* pMbCache,
                             ESkipModes eSkipMode) {
  SVAAFrameInfoExt_t* pVaaExt   = static_cast<SVAAFrameInfoExt_t*> (pEncCtx->pVaa);
  SDqLayer* pCurDqLayer         = pEncCtx->pCurDqLayer;

  const int32_t kiRefMbQp = pCurDqLayer->pRefPic->pRefMbQp[pCurMb->iMbXY];
  const int32_t kiCurMbQp = pCurMb->uiLumaQp;// unsigned -> signed

  pJudgeSkipFun pJudeSkip[2] = {JudgeStaticSkip, JudgeScrollSkip};
  bool bSkipFlag = pJudeSkip[eSkipMode] (pEncCtx, pCurMb, pMbCache, pWelsMd);

  if (bSkipFlag) {
    bool bQpSimilarFlag = (kiRefMbQp - kiCurMbQp <= DELTA_QP_SCD_THD || kiRefMbQp <= 26);
    SMVUnitXY sVaaPredSkipMv = {0, 0}, sCurMbMv[2] = {{0, 0}, {0, 0}};
    PredSkipMv (pMbCache, &sVaaPredSkipMv);

    if (eSkipMode == SCROLLED) {
      sCurMbMv[1].iMvX = static_cast<int16_t> (pVaaExt->sScrollDetectInfo.iScrollMvX << 2);
      sCurMbMv[1].iMvY = static_cast<int16_t> (pVaaExt->sScrollDetectInfo.iScrollMvY << 2);
    }

    bool bMbSkipFlag = (LD32 (&sVaaPredSkipMv) ==  LD32 (&sCurMbMv[eSkipMode])) ;
    SvcMdSCDMbEnc (pEncCtx, pWelsMd, pCurMb, pMbCache, pSlice, bQpSimilarFlag, bMbSkipFlag, sCurMbMv, eSkipMode);

    return true;
  }

  return false;
}

void SetBlockStaticIdcToMd (void* pVaa, SWelsMD* pWelsMd, SMB* pCurMb, SDqLayer* pDqLayer) {
  SVAAFrameInfoExt_t* pVaaExt = static_cast<SVAAFrameInfoExt_t*> (pVaa);

  const int32_t kiMbX = pCurMb->iMbX;
  const int32_t kiMbY = pCurMb->iMbY;
  const int32_t kiMbWidth = pDqLayer->iMbWidth;
  const int32_t kiWidth = kiMbWidth << 1;

  const int32_t kiBlockIndexUp = (kiMbY << 1) * kiWidth + (kiMbX << 1);
  const int32_t kiBlockIndexLow = ((kiMbY << 1) + 1) * kiWidth + (kiMbX << 1);

  pWelsMd->iBlock8x8StaticIdc[0] = pVaaExt->pVaaBestBlockStaticIdc[kiBlockIndexUp];
  pWelsMd->iBlock8x8StaticIdc[1] = pVaaExt->pVaaBestBlockStaticIdc[kiBlockIndexUp + 1];
  pWelsMd->iBlock8x8StaticIdc[2] = pVaaExt->pVaaBestBlockStaticIdc[kiBlockIndexLow];
  pWelsMd->iBlock8x8StaticIdc[3] = pVaaExt->pVaaBestBlockStaticIdc[kiBlockIndexLow + 1];

}

// Scene Change Detection (SCD) PSkip Decision for screen content
////////////////////////
bool WelsMdInterJudgeSCDPskip (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* slice, SMB* pCurMb, SMbCache* pMbCache) {
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;

  SetBlockStaticIdcToMd (pEncCtx->pVaa, pWelsMd, pCurMb, pCurDqLayer);

  if (MdInterSCDPskipProcess (pEncCtx, pWelsMd, slice, pCurMb, pMbCache, STATIC)) {
    return true;
  }

  if (MdInterSCDPskipProcess (pEncCtx, pWelsMd, slice, pCurMb, pMbCache, SCROLLED)) {
    return true;
  }

  return false;
}
bool WelsMdInterJudgeSCDPskipFalse (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* slice, SMB* pCurMb,
                                    SMbCache* pMbCache) {
  return false;
}


void WelsInitSCDPskipFunc (SWelsFuncPtrList* pFuncList, const bool bScrollingDetection) {
  if (bScrollingDetection) {
    pFuncList->pfSCDPSkipDecision = WelsMdInterJudgeSCDPskip;
  } else {
    pFuncList->pfSCDPSkipDecision = WelsMdInterJudgeSCDPskipFalse;
  }
}

// SubP16x16 Mode Decision for screen content
////////////////////////
//
//func pointer of inter MD for sub16x16 INTER MD for screen content coding
//
static inline void MergeSub16Me (const SWelsME& sSrcMe0, const SWelsME& sSrcMe1, SWelsME* pTarMe) {
  memcpy (pTarMe, &sSrcMe0, sizeof (sSrcMe0)); // confirmed_safe_unsafe_usage

  pTarMe->uiSadCost = sSrcMe0.uiSadCost + sSrcMe1.uiSadCost;//not precise cost since MVD cost is not the same
  pTarMe->uiSatdCost = sSrcMe0.uiSatdCost + sSrcMe1.uiSatdCost;//not precise cost since MVD cost is not the same
}
static inline bool IsSameMv (const SMVUnitXY& sMv0, const SMVUnitXY& sMv1) {
  return ((sMv0.iMvX == sMv1.iMvX) && (sMv0.iMvY == sMv1.iMvY));
}
bool TryModeMerge (SMbCache* pMbCache, SWelsMD* pWelsMd, SMB* pCurMb) {
  SWelsME* pMe8x8 = & (pWelsMd->sMe.sMe8x8[0]);
  const bool bSameMv16x8_0 = IsSameMv (pMe8x8[0].sMv, pMe8x8[1].sMv);
  const bool bSameMv16x8_1 = IsSameMv (pMe8x8[2].sMv, pMe8x8[3].sMv);

  const bool bSameMv8x16_0 = IsSameMv (pMe8x8[0].sMv, pMe8x8[2].sMv);
  const bool bSameMv8x16_1 = IsSameMv (pMe8x8[1].sMv, pMe8x8[3].sMv);

  const bool bSameRefIdx16x8_0 = true; //pMe8x8[0].iRefIdx == pMe8x8[1].iRefIdx;
  const bool bSameRefIdx16x8_1 = true; //pMe8x8[2].iRefIdx == pMe8x8[3].iRefIdx;
  const bool bSameRefIdx8x16_0 = true; //pMe8x8[0].iRefIdx == pMe8x8[2].iRefIdx;
  const bool bSameRefIdx8x16_1 = true; //pMe8x8[1].iRefIdx == pMe8x8[3].iRefIdx;
  const int32_t iSameMv = ((bSameMv16x8_0 && bSameRefIdx16x8_0  && bSameMv16x8_1 && bSameRefIdx16x8_1) << 1) |
                          (bSameMv8x16_0 && bSameRefIdx8x16_0 && bSameMv8x16_1 && bSameRefIdx8x16_1);

  switch (iSameMv) {
  case 3:






    break;
  case 2:
    pCurMb->uiMbType = MB_TYPE_16x8;
    MergeSub16Me (pMe8x8[0], pMe8x8[1], & (pWelsMd->sMe.sMe16x8[0]));
    MergeSub16Me (pMe8x8[2], pMe8x8[3], & (pWelsMd->sMe.sMe16x8[1]));
    PredInter16x8Mv (pMbCache, 0, 0, & (pWelsMd->sMe.sMe16x8[0].sMvp));
    PredInter16x8Mv (pMbCache, 8, 0, & (pWelsMd->sMe.sMe16x8[1].sMvp));
    break;
  case 1:
    pCurMb->uiMbType = MB_TYPE_8x16;
    MergeSub16Me (pMe8x8[0], pMe8x8[2], & (pWelsMd->sMe.sMe8x16[0]));
    MergeSub16Me (pMe8x8[1], pMe8x8[3], & (pWelsMd->sMe.sMe8x16[1]));
    PredInter8x16Mv (pMbCache, 0, 0, & (pWelsMd->sMe.sMe8x16[0].sMvp));
    PredInter8x16Mv (pMbCache, 4, 0, & (pWelsMd->sMe.sMe8x16[1].sMvp));
    break;
  default:
    break;
  }
  return (MB_TYPE_8x8 != pCurMb->uiMbType);
}


void WelsMdInterFinePartitionVaaOnScreen (sWelsEncCtx* pEncCtx, SWelsMD* pWelsMd, SSlice* pSlice, SMB* pCurMb,
    int32_t iBestCost) {
  SMbCache* pMbCache = &pSlice->sMbCacheInfo;
  SDqLayer* pCurDqLayer = pEncCtx->pCurDqLayer;
  int32_t iCostP8x8;
  uint8_t uiMbSign = pEncCtx->pFuncList->pfGetMbSignFromInterVaa (&pEncCtx->pVaa->sVaaCalcInfo.pSad8x8[pCurMb->iMbXY][0]);

  if (MBVAASIGN_FLAT == uiMbSign) {
    return;
  }

  iCostP8x8 = WelsMdP8x8 (pEncCtx->pFuncList, pCurDqLayer, pWelsMd, pSlice);
  if (iCostP8x8 < iBestCost) {
    iBestCost = iCostP8x8;
    pCurMb->uiMbType = MB_TYPE_8x8;
    memset (pCurMb->uiSubMbType, SUB_MB_TYPE_8x8, 4);
#if 0 //Disable for sub8x8 modes for now
    iBestCost = 0;

    pMbCache->sMvComponents.iRefIndexCache [9] = pMbCache->sMvComponents.iRefIndexCache [21] = REF_NOT_AVAIL;
    for (int32_t i8x8Idx = 0; i8x8Idx < 4; ++i8x8Idx) {
      int32_t iCurCostSub8x8, iBestCostSub8x8 = pWelsMd->sMe.sMe8x8[i8x8Idx].uiSatdCost;

      iCurCostSub8x8 = WelsMdP4x4 (pEncCtx->pFuncList, pCurDqLayer, pWelsMd, pSlice, i8x8Idx);
      if (iCurCostSub8x8 < iBestCostSub8x8) {
        pCurMb->uiSubMbType[i8x8Idx] = SUB_MB_TYPE_4x4;
        iBestCostSub8x8 = iCurCostSub8x8;
      }

      iCurCostSub8x8 = WelsMdP8x4 (pEncCtx->pFuncList, pCurDqLayer, pWelsMd, pSlice, i8x8Idx);
      if (iCurCostSub8x8 < iBestCostSub8x8) {
        pCurMb->uiSubMbType[i8x8Idx] = SUB_MB_TYPE_8x4;
        iBestCostSub8x8 = iCurCostSub8x8;
      }

      iCurCostSub8x8 = WelsMdP4x8 (pEncCtx->pFuncList, pCurDqLayer, pWelsMd, pSlice, i8x8Idx);
      if (iCurCostSub8x8 < iBestCostSub8x8) {
        pCurMb->uiSubMbType[i8x8Idx] = SUB_MB_TYPE_4x8;
        iBestCostSub8x8 = iCurCostSub8x8;
      }
      iBestCost += iBestCostSub8x8;
    }
    if ((pCurMb->uiSubMbType[0] == SUB_MB_TYPE_8x8) && (pCurMb->uiSubMbType[1] == SUB_MB_TYPE_8x8)
        && (pCurMb->uiSubMbType[2] == SUB_MB_TYPE_8x8) && (pCurMb->uiSubMbType[3] == SUB_MB_TYPE_8x8)) //all 8x8
#endif
      TryModeMerge (pMbCache, pWelsMd, pCurMb);
  }
  pWelsMd->iCostLuma = iBestCost;
}

// SetScrollingMvToMd
//
void SetScrollingMvToMd (SVAAFrameInfo* pVaa, SWelsMD* pWelsMd) {
  SVAAFrameInfoExt* pVaaExt = static_cast<SVAAFrameInfoExt*> (pVaa);

  SMVUnitXY          sTempMv;
  sTempMv.iMvX = pVaaExt->sScrollDetectInfo.iScrollMvX;
  sTempMv.iMvY = pVaaExt->sScrollDetectInfo.iScrollMvY;

  (pWelsMd->sMe.sMe16x16).sDirectionalMv =
    (pWelsMd->sMe.sMe8x8[0]).sDirectionalMv =
      (pWelsMd->sMe.sMe8x8[1]).sDirectionalMv =
        (pWelsMd->sMe.sMe8x8[2]).sDirectionalMv =
          (pWelsMd->sMe.sMe8x8[3]).sDirectionalMv = sTempMv;
}

void SetScrollingMvToMdNull (SVAAFrameInfo* pVaa, SWelsMD* pWelsMd) {
}

} // namespace WelsEnc
