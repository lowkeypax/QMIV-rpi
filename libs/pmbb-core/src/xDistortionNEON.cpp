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
        int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
        int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
        int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
        SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
    }//koniec wierszy
    int32x4_t Tmp1V = vpaddq_s32     (SD_V128, SD_V128);
    int32x4_t Tmp2V = vpaddq_s32     (Tmp1V,   Tmp1V);
    int32 SD        = vgetq_lane_s32 (vget_high_s32(Tmp2V), 0);

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
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
                int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
                SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        int32x4_t Tmp1V = vpaddq_s32     (SD_V128, SD_V128);
        int32x4_t Tmp2V = vpaddq_s32     (Tmp1V,   Tmp1V);
        int32 SD        = vgetq_lane_s32 (vget_high_s32(Tmp2V), 0);
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
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
                int32x4_t Sum_V128    = vaddq_s32       (Diffl_V128, Diffh_V128);
                SD_V128               = vaddq_s32       (SD_V128,    Sum_V128);
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V64    = vld1_u16     (&Tst[x]);
                uint16x4_t Ref_V64    = vld1_u16     (&Ref[x]);
                int32x4_t  Tst_V128   = vmovl_s16    (vreinterpret_s16_u16(Tst_V64));
                int32x4_t  Ref_V128   = vmovl_s16    (vreinterpret_s16_u16(Ref_V64));
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
        int32x4_t Tmp1V = vpaddq_s32     (SD_V128, SD_V128);
        int32x4_t Tmp2V = vpaddq_s32     (Tmp1V,   Tmp1V);
        SD += vgetq_lane_s32 (vget_high_s32(Tmp2V), 0);
        return SD;
    }
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
    const int32 Area8        = (int32)((uint32)Area & c_MultipleMask8);
    uint32x4_t SAD_V128      = vdupq_n_u32(0);
    uint32 SAD = 0;
    for(int32 i = 0; i<Area8; i+= 8)
    {
        uint16x8_t Tst_V128   = vld1q_u16       (&Tst[i]);
        uint16x8_t Ref_V128   = vld1q_u16       (&Ref[i]);
        int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
        int32x4_t ADiffl_V128 = vabsq_s32       (Diffl_V128);
        int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
        int32x4_t ADiffh_V128 = vabsq_s32       (Diffh_V128);
        int32x4_t Sum_V128    = vaddq_s32       (ADiffl_V128, ADiffh_V128);
        SAD_V128              = vaddq_u32       (SAD_V128,    vreinterpretq_u32_s32(Sum_V128));


        //vabal_u16(SAD_V128, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
        //vabal_high_u16(SAD_V128, Tst_V128, Ref_V128);

        //uint32x4_t ADiffl_V128  = vabdl_u16       (vget_low_u16(Tst_V128), vget_low_u16(Ref_V128)); //absub 0-3 to uint32
        //uint32x4_t ADiffh_V128  = vabdl_high_u16  (Tst_V128, Ref_V128); //absub 4-7


    }//i
    //uint32x4_t Tmp1V = vpaddq_u32     (SAD_V128, SAD_V128);
    //uint32x4_t Tmp2V = vpaddq_u32     (Tmp1V,   Tmp1V);
    //SAD += vdups_lane_u32 (vget_high_u32(Tmp2V), 0);
    SAD += vgetq_lane_u32(SAD_V128, 0);
    SAD += vgetq_lane_u32(SAD_V128, 1);
    SAD += vgetq_lane_u32(SAD_V128, 2);
    SAD += vgetq_lane_u32(SAD_V128, 3);


    for(int32 i=Area8; i < Area; i++) { SAD += (uint32)xAbs(((int32)Tst[i]) - ((int32)Ref[i])); }
    return SAD;

}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    uint32x4_t  SAD_V128 = vdupq_n_u32(0);
    if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
    {
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width; x+=8)
            {
                uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
                uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
                int32x4_t ADiffl_V128 = vabsq_s32       (Diffl_V128);
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
                int32x4_t ADiffh_V128 = vabsq_s32       (Diffh_V128);
                int32x4_t Sum_V128    = vaddq_s32       (ADiffl_V128, ADiffh_V128);
                SAD_V128              = vaddq_u32       (SAD_V128,    vreinterpretq_u32_s32(Sum_V128));
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        int32x4_t Tmp1V = vpaddq_s32     (SAD_V128, SAD_V128);
        int32x4_t Tmp2V = vpaddq_s32     (Tmp1V,   Tmp1V);
        uint32 SAD        = vgetq_lane_s32 (vget_high_s32(vreinterpretq_u32_s32(Tmp2V)), 0);
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
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); //sub 0-3 to uint32
                int32x4_t ADiffl_V128 = vabsq_s32       (Diffl_V128);
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128)); //sub 4-7
                int32x4_t ADiffh_V128 = vabsq_s32       (Diffh_V128);
                int32x4_t Sum_V128    = vaddq_s32       (ADiffl_V128, ADiffh_V128);
                SAD_V128              = vaddq_u32       (SAD_V128,    vreinterpretq_u32_s32(Sum_V128));
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V64    = vld1_u16     (&Tst[x]);
                uint16x4_t Ref_V64    = vld1_u16     (&Ref[x]);
                int32x4_t  Tst_V128   = vmovl_s16    (vreinterpret_s16_u16(Tst_V64));
                int32x4_t  Ref_V128   = vmovl_s16    (vreinterpret_s16_u16(Ref_V64));
                int32x4_t Diff_V128   = vsubq_s32    (Tst_V128, Ref_V128);
                int32x4_t ADiff_V128  = vabsq_s32    (Diff_V128);
                SAD_V128              = vaddq_u32    (SAD_V128, vreinterpret_u32_s32(ADiff_V128));
            }//4x
            for(int32 x=Width4; x<Width; x++)
            {
                SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x]));
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        int32x4_t Tmp1V = vpaddq_s32     (SAD_V128, SAD_V128);
        int32x4_t Tmp2V = vpaddq_s32     (Tmp1V,   Tmp1V);
        SAD += vgetq_lane_u32 (vget_high_u32(vreinterpretq_u32_s32(Tmp2V)), 0);
        return SAD;
    }
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  
    const int32 Area8 = (int32)((uint32)Area & c_MultipleMask8);
    uint64x2_t  SSD_V128 = vdupq_n_u64(0);

    for(int32 i = 0; i < Area8; i += 8)
    {
        uint16x8_t Tst_V64    = vld1q_u16     (&Tst[x]);
        uint16x8_t Ref_V64    = vld1q_u16     (&Ref[x]);
        int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); 
        int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128));
        //multiply accumulate and widen todo

        int64x2_t PowlDiffl_V128 = vmull_s32      (vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
        int64x2_t PowhDiffl_V128 = vmull_high_s32 (Diffl_V128, Diffl_V128);
        int64x2_t PowlDiffh_V128 = vmull_s32      (vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
        int64x2_t PowhDiffh_V128 = vmull_high_s32 (Diffh_V128, Diffh_V128);

        int64x2_t Powl_V128 = vaddq_s64 (PowlDiffh_V128, PowlDiffl_V128);
        int64x2_t Powh_V128 = vaddq_s64 (PowhDiffh_V128, PowhDiffl_V128);

        SSD_V128 = vaddq_s64 (Powh_V128, Powl_V128);
    }
    uint64 SSD = (uint64)(vgetq_laneq_s64(vpaddq_s64 (SSD_V128,0),0));

    for(int32 i = Area8; i < Area; i++) { SSD += (uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i])); }
    return SSD;
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    uint64x2_t  SSD_V128 = vdupq_n_u64(0);
    if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
    {
        for(int32 y=0; y<Height; y++)
        {
            for(int32 x=0; x<Width; x+=8)
            {
                uint16x8_t Tst_V64    = vld1q_u16     (&Tst[x]);
                uint16x8_t Ref_V64    = vld1q_u16     (&Ref[x]);
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); 
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128));
                //multiply accumulate and widen todo

                int64x2_t PowlDiffl_V128 = vmull_s32      (vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
                int64x2_t PowhDiffl_V128 = vmull_high_s32 (Diffl_V128, Diffl_V128);
                int64x2_t PowlDiffh_V128 = vmull_s32      (vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
                int64x2_t PowhDiffh_V128 = vmull_high_s32 (Diffh_V128, Diffh_V128);

                int64x2_t Powl_V128 = vaddq_s64 (PowlDiffh_V128, PowlDiffl_V128);
                int64x2_t Powh_V128 = vaddq_s64 (PowhDiffh_V128, PowhDiffl_V128);

                SSD_V128 = vaddq_s64 (Powh_V128, Powl_V128);
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        uint64 SSD = (uint64)(vgetq_lane_s64(vpaddq_s64 (SSD_V128,0),0));
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
                uint16x8_t Tst_V64    = vld1q_u16     (&Tst[x]);
                uint16x8_t Ref_V64    = vld1q_u16     (&Ref[x]);
                int32x4_t Diffl_V128  = vsubl_s16       (vreinterpret_s16_u16(vget_low_u16(Tst_V128)), vreinterpret_s16_u16(vget_low_u16(Ref_V128))); 
                int32x4_t Diffh_V128  = vsubl_high_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128));
                //multiply accumulate and widen todo

                int64x2_t PowlDiffl_V128 = vmull_s32      (vget_low_s32(Diffl_V128), vget_low_s32(Diffl_V128));
                int64x2_t PowhDiffl_V128 = vmull_high_s32 (Diffl_V128, Diffl_V128);
                int64x2_t PowlDiffh_V128 = vmull_s32      (vget_low_s32(Diffh_V128), vget_low_s32(Diffh_V128));
                int64x2_t PowhDiffh_V128 = vmull_high_s32 (Diffh_V128, Diffh_V128);

                int64x2_t Powl_V128 = vaddq_s64 (PowlDiffh_V128, PowlDiffl_V128);
                int64x2_t Powh_V128 = vaddq_s64 (PowhDiffh_V128, PowhDiffl_V128);

                SSD_V128 = vaddq_s64 (Powh_V128, Powl_V128);
            }//8x
            for(int32 x=Width8; x<Width4; x+=4)
            {
                uint16x4_t Tst_V64    = vld1q_u16     (&Tst[x]);
                uint16x4_t Ref_V64    = vld1q_u16     (&Ref[x]);
                int32x4_t Diff_V128   = vsubl_s16  (vreinterpretq_s16_u16(Tst_V128), vreinterpretq_s16_u16(Ref_V128));
                //multiply accumulate and widen todo

                int64x2_t Powl_V128 = vmull_s32          (vget_low_s32(Diff_V128), vget_low_s32(Diff_V128));
                int64x2_t Powh_V128 = vmull_high_s32     (Diff_V128, Diff_V128);

                SSD_V128 = vaddq_s64 (Powh_V128, Powl_V128);
            }//4x
            for(int32 x=Width4; x<Width; x++)
            {
                SSD += (uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i]));
            }//x
            Tst += TstStride;
            Ref += RefStride;
        }//y
        SSD += (uint64)(vgetq_lane_s64(vpaddq_s64 (SSD_V128,0),0));
        return 0;
    }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
