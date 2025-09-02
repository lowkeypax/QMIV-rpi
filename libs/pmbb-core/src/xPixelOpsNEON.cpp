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
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;

  if(((uint32)DstWidth & c_RemainderMask16) == 0)//notail
  {
    for (int32 y=0; y<DstHeight; y+=2)
    {
      for (int32 x=0; x<DstWidth; x+=16)
      {
        uint16x8_t SrcV = vld1q_u16 (&Src[x>>1]);

        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);

        vst1q_u16 (&DstL0[x], DstV.val[0]);
        vst1q_u16 (&DstL0[x+8], DstV.val[1]);
        vst1q_u16 (&DstL1[x], DstV.val[0]);
        vst1q_u16 (&DstL1[x+8], DstV.val[1]);//kolejniosc????
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t SrcV = vld1q_u16 (&Src[x>>1]);

        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);

        vst1q_u16 (&DstL0[x], DstV.val[0]);
        vst1q_u16 (&DstL0[x+8], DstV.val[1]);
        vst1q_u16 (&DstL1[x], DstV.val[0]);
        vst1q_u16 (&DstL1[x+8], DstV.val[1]);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint16x4_t SrcV = vld1_u16 (&Src[x>>1]);

        uint16x4x2_t DstV = vzip_u16 (SrcV, SrcV);

        vst1_u16 (&DstL0[x], DstV.val[0]);
        vst1_u16 (&DstL0[x+4], DstV.val[1]);
        vst1_u16 (&DstL1[x], DstV.val[0]);
        vst1_u16 (&DstL1[x+4], DstV.val[1]);
      }
      for(int32 x=Width8; x<DstWidth; x+=2)
      {
        const uint16 S = Src[x>>1];
        DstL0[x  ] = S;
        DstL0[x+1] = S;
        DstL1[x  ] = S;
        DstL1[x+1] = S;
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  return;
}
void xPixelOpsNEON::DownsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* restrict SrcL0 = Src;
  const uint16* restrict SrcL1 = Src + SrcStride;

  if(((uint32)DstWidth & c_RemainderMask8) == 0)//notail
  {
    for (int32 y=0; y<DstHeight; y++)
    {
      for (int32 x=0; x<DstWidth; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]); //16x8x2 caly top??
        uint16x8_t TopRightV  = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        uint16x8_t BottomRightV = vld1q_u16(&SrcL1[SrcX+8]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half
        uint16x8_t Right = vshrq_n_u16(vaddq_u16(TopRightV, BottomRightV),1);

        uint32x4_t LeftV = vpaddlq_u16(Left);
        uint32x4_t RightV = vpaddlq_u16(Right);
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1);
        uint32x4_t HRightV = vshrq_n_u32(RightV,1);
        
        uint16x8_t DstV = vcombine_u16(vqmovn_u32(HLeftV), vqmovn_u32(HRightV));
        vst1q_u16 (&Dst[x], DstV);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  else
  {
    const int32 Width8  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask8);
    const int32 Width4  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask4);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]); //16x8x2 caly top??
        uint16x8_t TopRightV  = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        uint16x8_t BottomRightV = vld1q_u16(&SrcL1[SrcX+8]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half
        uint16x8_t Right = vshrq_n_u16(vaddq_u16(TopRightV, BottomRightV),1);

        uint32x4_t LeftV = vpaddlq_u16(Left);
        uint32x4_t RightV = vpaddlq_u16(Right);
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1);
        uint32x4_t HRightV = vshrq_n_u32(RightV,1);
        
        uint16x8_t DstV = vcombine_u16(vqmovn_u32(HLeftV), vqmovn_u32(HRightV));
        vst1q_u16 (&Dst[x], DstV);
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half

        uint32x4_t LeftV = vpaddlq_u16(Left); //add pairwise
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1); //half
        
        uint16x4_t DstV = vqmovn_u32(HLeftV);
        vst1_u16 (&Dst[x], DstV);
      }
      for(int32 x=Width4; x<DstWidth; x++)
      {      
        const int32 SrcX = x<<1;
        int16 D = ((int32)SrcL0[SrcX  ] + (int32)SrcL0[SrcX+1] + (int32)SrcL1[SrcX  ] + (int32)SrcL1[SrcX+1] + 2)>>2;
        Dst[x] = D;
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::CvtUpsampleHV(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16 *restrict DstL0 = Dst;
  uint16 *restrict DstL1 = Dst + DstStride;
  if(((uint32)DstWidth & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH); 

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL0[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL0[x+24], DstVH.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL1[x+24], DstVH.val[1]);

      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width32; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH); 

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL0[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL0[x+24], DstVH.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL1[x+24], DstVH.val[1]);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        uint8x8_t SrcV = vld1_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
      }
      for(int32 x=Width16; x<DstWidth; x+=2)
      {
        int16 S = Src[x>>1];
        DstL0[x  ] = S;
        DstL0[x+1] = S;
        DstL1[x  ] = S;
        DstL1[x+1] = S;
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  return;
}
void xPixelOpsNEON::CvtDownsampleHV(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* restrict SrcL0 = Src;
  const uint16* restrict SrcL1 = Src + SrcStride;

  
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
