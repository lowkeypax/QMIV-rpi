




#include "xCorrespPixelShiftNEON.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xCorrespPixelShiftNEON
//===============================================================================================================================================================================================================

uint64V4 xCorrespPixelShiftNEON::CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  assert(Tst->isCompatible(Ref));

  const int32  Width     = Tst->getWidth();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const int32x4_t CmpWeightsV       = vld1q_s32(CmpWeights.getElementsPtr());
  const int32x4_t GlobalColorShiftV = vld1q_s32(GlobalColorShift.getElementsPtr());

  const uint16V4* TstPtr = Tst->getAddr() + TstOffset;
  int32x4_t RowDistV = vdupq_n_s32(0);
  for (int32 x = 0; x < Width; x++)
  {
    uint16x4_t TstU16V  = vld1_u16(TstPtr->getElementsPtr() + x);
    int32x4_t  TstV     = vaddq_s32(GlobalColorShiftV, vreinterpretq_s32_u32(vmovl_u16(TstU16V))); //TODO - xc_CLIP_CURR_TST_RANGE
    int32x4_t BestDist = xCalcDistWithinBlock(TstV, Ref, x, y, SearchRange, CmpWeightsV);
    RowDistV = vaddq_s32(RowDistV, BestDist);
  }//x

  int32V4 RowDist;
  vst1q_s32(RowDist.getElementsPtr(), RowDistV);
  return (uint64V4)RowDist;
}
int32x4_t xCorrespPixelShiftNEON::xCalcDistWithinBlock(const int32x4_t& TstPelV, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32x4_t& CmpWeightsV)
{
  const int32 WindowSize = 2 * SearchRange + 1;
  const int32 BegY = CenterY - SearchRange;
  const int32 BegX = CenterX - SearchRange;

  const int32     Stride = Ref->getStride();
  const uint16V4* RefPtr = Ref->getAddr() + BegY * Stride + BegX;

  int32   BestError = std::numeric_limits<int32>::max();
  int32x4_t BestDistV = vdupq_n_s32(0);

  for (int32 y = 0; y < WindowSize; y++)
  {
    const uint16V4* RefPtrY = RefPtr + y * Stride;
    for (int32 x = 0; x < WindowSize; x++)
    {
      uint16x4_t RefU16V = vld1_u16(RefPtrY->getElementsPtr() + x);
    //__m128i RefV    = _mm_unpacklo_epi16(RefU16V, _mm_setzero_si128());
      int32x4_t RefV     = vreinterpretq_s32_u32(vmovl_u16(RefU16V));
      int32x4_t DiffV    = vsubq_s32  (TstPelV, RefV);
      int32x4_t DistV    = vmulq_s32(DiffV, DiffV);
      int32x4_t ErrorV   = vmulq_s32(DistV, CmpWeightsV);
      int32   Error   = vaddvq_s32(ErrorV);
      if (Error < BestError) { BestError = Error; BestDistV = DistV; }
    } //x
  } //y

  return BestDistV;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON