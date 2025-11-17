#include "xDistortionNEON.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
int32 xDistortionNEON::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  
    const int32 Area8   = (int32)((uint32)Area & c_MultipleMask8);
    int32x4_t  SD_V128 = vdupq_n_s32(0);

    for(int32 i = 0; i < Area8; i += 8)
    {
        uint16x8_t Tst_V128   = vld1q_u16       (&Tst[i]);
        uint16x8_t Ref_V128   = vld1q_u16       (&Ref[i]);
        int32x4_t Diffl_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V128))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V128)))); //sub 0-3 to uint32
        int32x4_t Diffh_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V128)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V128))); //sub 4-7
        int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
        SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
    }//koniec wierszy
    int32 SD        = vaddvq_s32(SD_V128);

    //tail
    for(int32 i = Area8; i < Area; i++) { SD += (int32)Tst[i] - (int32)Ref[i]; }

    return SD;
}
int32 xDistortionNEON::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    int32x4_t  SD_V128 = vdupq_n_s32(0);
    if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
    {
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width; x+=8)
            {
                uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
                uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);
                int32x4_t Diffl_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V128))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V128)))); //sub 0-3 to uint32
                int32x4_t Diffh_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V128)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V128))); //sub 4-7
                int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
                SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        int32 SD        = vaddvq_s32(SD_V128);
        return SD;
    }
    else
    {
        const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
        const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);
        int32 SD = 0;
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width8; x+=8)
            {
                uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
                uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);
                int32x4_t Diffl_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V128))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V128)))); //sub 0-3 to uint32
                int32x4_t Diffh_V128  = vsubq_s32       (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V128)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V128))); //sub 4-7
                int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
                SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V64    = vld1_u16     (&Tst[x]);
                uint16x4_t Ref_V64    = vld1_u16     (&Ref[x]);
                int32x4_t  Tst_V128   = vreinterpretq_s32_u32(vmovl_u16(Tst_V64));
                int32x4_t  Ref_V128   = vreinterpretq_s32_u32(vmovl_u16(Ref_V64));
                int32x4_t Diff_V128   = vsubq_s32    (Tst_V128, Ref_V128);
                SD_V128               = vaddq_s32    (SD_V128, Diff_V128);
            }//4x
            for(int32 x=Width4; x<Width; x++)
            {
                SD += (int32)Tst[x] - (int32)Ref[x];
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        SD += vaddvq_s32(SD_V128);
        return SD;
    }
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
    const int32 Area8        = (int32)((uint32)Area & c_MultipleMask8);
    uint32x4_t SAD_V128      = vdupq_n_u32(0);

    for(int32 i = 0; i<Area8; i+= 8)
    {
        uint16x8_t Tst_V128   = vld1q_u16       (&Tst[i]);
        uint16x8_t Ref_V128   = vld1q_u16       (&Ref[i]);

        SAD_V128 = vabal_u16(SAD_V128, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
        SAD_V128 = vabal_high_u16(SAD_V128, Tst_V128, Ref_V128);

    }//i
    uint32 SAD = vaddvq_u32(SAD_V128);

    for(int32 i=Area8; i < Area; i++) { SAD += (uint32)xAbs(((int32)Tst[i]) - ((int32)Ref[i])); }

    return SAD;
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    uint32x4_t  SAD_V = vdupq_n_u32(0);
    if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
    {
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width; x+=8)
            {
                uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
                uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);

                SAD_V = vabal_u16(SAD_V, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
                SAD_V = vabal_high_u16(SAD_V, Tst_V128, Ref_V128);
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        uint32 SAD = vaddvq_u32(SAD_V);
        return SAD;
    }
    else
    {
        const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
        const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);
        int32 SAD = 0;
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width8; x+=8)
            {
                uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
                uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);

                SAD_V = vabal_u16(SAD_V, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
                SAD_V = vabal_high_u16(SAD_V, Tst_V128, Ref_V128);
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V128   = vld1_u16       (&Tst[x]);
                uint16x4_t Ref_V128   = vld1_u16       (&Ref[x]);

                SAD_V = vabal_u16(SAD_V, Tst_V128, Ref_V128);
            }//4x
            for(int32 x=Width4; x<Width; x++)
            {
                SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x]));
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        SAD += vaddvq_u32(SAD_V);
        return SAD;
    }
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  
    const int32 Area8 = (int32)((uint32)Area & c_MultipleMask8);
    int64x2_t Pow_V = vdupq_n_s64(0);

    for(int32 i = 0; i < Area8; i += 8)
    {
        uint16x8_t Tst_V    = vld1q_u16     (&Tst[i]);
        uint16x8_t Ref_V    = vld1q_u16     (&Ref[i]);
        int32x4_t Diffl_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V)))); 
        int32x4_t Diffh_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V)));
        
        //multiply accumulate and widen
        Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
        Pow_V = vmlal_high_s32 (Pow_V, Diffl_V128, Diffl_V128);
        Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
        Pow_V = vmlal_high_s32 (Pow_V, Diffh_V128, Diffh_V128);

        //SSD_V128 = vaddvq_s64 (Pow_V);
    }
    uint64 SSD = (uint64)(vaddvq_s64 (Pow_V));

    for(int32 i = Area8; i < Area; i++) { SSD += (uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i])); }
    return SSD;
    
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    int64x2_t  Pow_V = vdupq_n_s64(0);
    if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
    {
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width; x+=8)
            {
                uint16x8_t Tst_V    = vld1q_u16     (&Tst[x]);
                uint16x8_t Ref_V    = vld1q_u16     (&Ref[x]);
                int32x4_t Diffl_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V)))); 
                int32x4_t Diffh_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V)));
                
                //multiply accumulate and widen
                Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
                Pow_V = vmlal_high_s32 (Pow_V, Diffl_V128, Diffl_V128);
                Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
                Pow_V = vmlal_high_s32 (Pow_V, Diffh_V128, Diffh_V128);
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        uint64 SSD = (uint64)(vaddvq_s64 (Pow_V));
        return SSD;
    }
    else
    {
        const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
        const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);
        uint64 SSD = 0;
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width8; x+=8)
            {
                uint16x8_t Tst_V    = vld1q_u16     (&Tst[x]);
                uint16x8_t Ref_V    = vld1q_u16     (&Ref[x]);
                int32x4_t Diffl_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V)))); 
                int32x4_t Diffh_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V)));
                
                //multiply accumulate and widen
                Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
                Pow_V = vmlal_high_s32 (Pow_V, Diffl_V128, Diffl_V128);
                Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
                Pow_V = vmlal_high_s32 (Pow_V, Diffh_V128, Diffh_V128);
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V    = vld1_u16     (&Tst[x]);
                uint16x4_t Ref_V    = vld1_u16     (&Ref[x]);
                int32x4_t Diffl_V128  = vsubq_s32 (vreinterpretq_s32_u32(vmovl_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_u16(Ref_V))); 
                
                //multiply accumulate and widen
                Pow_V = vmlal_s32      (Pow_V, vget_low_s32(Diffl_V128), vget_low_s32(Diffl_V128));
                Pow_V = vmlal_high_s32 (Pow_V, Diffl_V128, Diffl_V128);
            }//4x
            for(int32 x=Width4; x<Width; x++)
            {
                SSD += (uint64)xPow2(((int32)Tst[x]) - ((int32)Ref[x]));
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        SSD += (uint64)(vaddvq_s64 (Pow_V));
        return SSD;
    }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
