




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

  //printf("a");
  const uint16V4* TstPtr = Tst->getAddr() + TstOffset; //pierwszy 16v4 wektor obrazu
  int32x4_t RowDistV = vdupq_n_s32(0);
  for (int32 x = 0; x < Width; x++)
  {
    uint16x4_t TstU16V  = vld1_u16((TstPtr + x)->getElementsPtr()); //x-owy 4-elementowy pixel, getElemPtr() wskazuje wewnatrz na pierwszy z elementow, wrzuca do 16x4
    int32x4_t  TstV     = vaddq_s32(GlobalColorShiftV, vreinterpretq_s32_u32(vmovl_u16(TstU16V))); 
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

  //const uint16V4* RefPtr = Ref->getAddr();
  const int32     Stride = Ref->getStride(); 
  const uint16V4* RefPtrBeg = Ref->getAddr() + Stride*BegY + BegX; // adres poczatku obrazu + xy = adres poczatku okna
                                                                   // wskazuje na 4 elementowe pixele obrazu
  int32 BestError = std::numeric_limits<int32>::max();
  int32x4_t BestDistV = vdupq_n_s32(0);

  for(int32 y = BegY; y < WindowSize; y++)
  {
    //const uint16V4* Offset = RefPtrBeg + y*Stride;
    for(int32 x = BegX; x<= WindowSize; x++)
    {
      uint16x4_t RefV16 = vld1_u16((RefPtrBeg + y*Stride + x)->getElementsPtr()); // adres poczatku okna + yx = liczony pixel w oknie
                                                                                  //getElements() wskazuje na pierwszy z 4 elementow pixela, wrzuca do 16x4
      //uint16x4_t RefV16 = vld1_u16((Offset + x)->getElementsPtr());
      int32x4_t RefV = vreinterpretq_s32_u32(vmovl_u16(RefV16));
      int32x4_t Diff = vsubq_s32(TstPelV, RefV); //tst32x4 - ref32x4
      int32x4_t Dist = vmulq_s32(Diff, Diff);    //^2
      int32x4_t ErrorV = vmulq_s32(Dist, CmpWeightsV); 
      int32 Error = vaddvq_s32(ErrorV);          // Error = suma[(tst-ref)^2*waga]
      if (Error < BestError) { BestError = Error; BestDistV = Dist; }
    }
  }
  return BestDistV;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON