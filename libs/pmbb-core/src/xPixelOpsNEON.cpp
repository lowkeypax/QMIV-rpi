#include "xPixelOpsNEON.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsNEON::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16)==0) //ifnotail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint8x16_t SrcV   = vld1q_u8       (&Src[x]);
        
        uint16x8_t DstV1  = vmovl_u8  (vget_low_u8(SrcV));
        uint16x8_t DstV2  = vmovl_high_u8  (SrcV);

        vst1q_u16(&Dst[x],  DstV1);
        vst1q_u16(&Dst[x+8],DstV2); //kolejnosc???

      }
      Src += SrcStride;
      Dst += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint8x16_t SrcV   = vld1q_u8       (&Src[x]);
        
        uint16x8_t DstV1  = vmovl_u8  (vget_low_u8(SrcV));
        uint16x8_t DstV2  = vmovl_high_u8  (SrcV);

        vst1q_u16(&Dst[x],  DstV1);
        vst1q_u16(&Dst[x+8],DstV2); //kolejnosc ok

      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint8x8_t SrcV    = vld1_u8   (&Src[x]);

        uint16x8_t DstV1 = vmovl_u8 (SrcV);

        vst1q_u16(&Dst[x], DstV1);
      }
      for(int32 x=Width8; x<Width; x++)
      {
        Dst[x] = (uint16)(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16)==0) //ifnotail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16 (&Src[x+8]);
        
        uint8x16_t DstV =  vcombine_u8(vqmovn_u16(SrcV1), vqmovn_u16(SrcV2));

        vst1q_u8 (&Dst[x], DstV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16 (&Src[x+8]);
        
        uint8x16_t DstV =  vcombine_u8(vqmovn_u16(SrcV1), vqmovn_u16(SrcV2));

        vst1q_u8 (&Dst[x], DstV);

      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint8x8_t DstV   = vqmovn_u16(SrcV1);

        vst1_u8 (&Dst[x], DstV);
      }
      for(int32 x=Width8; x<Width; x++)
      {
        Dst[x] = (uint8)xClipU8<uint16>(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
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
  return 0;
}
bool xPixelOpsNEON::CompareEqual(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  return 0;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
