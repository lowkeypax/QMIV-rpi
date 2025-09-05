
#include "xStructSimNEON.h"
#include "xStructSim.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

flt64 xStructSimSSE::CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL)
{
  if(BlockSize != 8 && BlockSize != 16 && BlockSize != 32) { return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, BlockSize, C1, C2, CalcL); }
  const int32 c_BlockArea    = BlockSize * BlockSize;
  const flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  int32x4_t SumR_I32_V  = vdupq_n_s32(0);
  int32x4_t SumT_I32_V  = vdupq_n_s32(0);
  int64x2_t SumR2_I64_V = vdupq_n_s64(0);
  int64x2_t SumT2_I64_V = vdupq_n_s64(0);
  int64x2_t SumRT_I64_V = vdupq_n_s64(0);

  for(int32 y = 0; y < BlockSize; y++)
  {
    for(int32 x = 0; x < BlockSize; x+=8)
    {
      uint16x8_t Tst_V = vld1q_u16(Tst+x);
      uint16x8_t Ref_V = vld1q_u16(Ref+x);
      //SumT  += T;
      //__m128i Tst_I32_V1 = _mm_unpacklo_epi16(Tst_U16_V, _mm_setzero_si128());
      //__m128i Tst_I32_V2 = _mm_unpackhi_epi16(Tst_U16_V, _mm_setzero_si128());
      //SumT_I32_V  = _mm_add_epi32(SumT_I32_V, _mm_add_epi32(Tst_I32_V1, Tst_I32_V2)); 
      SumT_I32_V = vaddq_u32(SumT_I32_V, vpaddlq_u16(Tst_V));
      //SumR  += R;
      //__m128i Ref_I32_V1 = _mm_unpacklo_epi16(Ref_U16_V, _mm_setzero_si128());
      //__m128i Ref_I32_V2 = _mm_unpackhi_epi16(Ref_U16_V, _mm_setzero_si128());
      //SumR_I32_V  = _mm_add_epi32(SumR_I32_V, _mm_add_epi32(Ref_I32_V1, Ref_I32_V2)); 
      SumR_I32_V = vaddq_u32(SumT_I32_V, vpaddlq_u16(Ref_V));
      //SumT2 += xPow2(T);
      uint64x2_t T2_Vl = vmull_u16(vget_low_u16(Tst_V), vget_low_u16(Tst_V));
      uint64x2_t T2_vh = vmull_high_u16(Tst_V, Tst_V);
      SumT2_I64_V = vaddq_u64(SumT2_I64_V, T2_Vl);
      SumT2_I64_V = vaddq_u64(SumT2_I64_V, T2_Vh);
      //SumR2 += xPow2(R);
      uint64x2_t R2_Vl = vmull_u16(vget_low_u16(Ref_V), vget_low_u16(Ref_V));
      uint64x2_t R2_vh = vmull_high_u16(Ref_V, Ref_V);
      SumR2_I64_V = vaddq_u64(SumT2_I64_V, R2_Vl);
      SumR2_I64_V = vaddq_u64(SumT2_I64_V, R2_Vh);
      //SumRT += R*T;
      SumRT = vmlal_u16(SumRT_I64_V, vget_low_u16(Tst_V), vget_low_u16(Ref_V));
      SumRT = vmlal_high_u16(SumRT_I64_V, Tst_U16_V, Ref_U16_V);
    }    
    Ref += StrideR;
    Tst += StrideT;
  }

  //__m128i TmpSumRandT = _mm_hadd_epi32(_mm_hadd_epi32(SumR_I32_V, SumT_I32_V), _mm_setzero_si128());
  //int32 SumR = _mm_extract_epi32(TmpSumRandT, 0);
  //int32 SumT = _mm_extract_epi32(TmpSumRandT, 1);

  int64 SumR2 = vaddvq_s64(SumR2_I64_V); //int uint??
  int64 SumT2 = vaddvq_s64(SumT2_I64_V);
  int64 SumRT = vaddvq_s64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumR2 * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * c_InvBlockArea - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * c_InvBlockArea - AvgR*AvgT;

  if(CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON