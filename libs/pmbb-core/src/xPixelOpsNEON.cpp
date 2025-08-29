#include "xPixelOpsNEON.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsNEON::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16)==0)
  {
    for(int32 y=0; y=Height; y++)
    {
      for(int32 x=0; x=Width; x++)
      {
        uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
        uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);

        
      }
    }
  }  
  return;
}
void xPixelOpsNEON::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  return;
}
void xPixelOpsNEON::UpsampleHV(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::DownsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::CvtUpsampleHV(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::CvtDownsampleHV(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::UpsampleH(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::DownsampleH(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::CvtUpsampleH(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
void xPixelOpsNEON::CvtDownsampleH(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  return;
}
bool xPixelOpsNEON::CheckIfInRange(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  return true;
}
void xPixelOpsNEON::AOS4fromSOA3(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, const uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  return;
}
void xPixelOpsNEON::SOA3fromAOS4(uint16* restrict DstA, uint16* restrict DstB, uint16* restrict DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  return;
}
int32 xPixelOpsNEON::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  return ;
}
bool xPixelOpsNEON::CompareEqual(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  return 0;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
