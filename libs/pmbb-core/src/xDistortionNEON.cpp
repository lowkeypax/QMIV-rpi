/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
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
    int32 SD        = vdups_lane_s32 (vget_high_s32(Tmp2V), 0);

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
        int32 SD        = vdups_lane_s32 (vget_high_s32(Tmp2V), 0);
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
                int32x4_t  Tst_V128   = vmovl_s16(vreinterpret_s16_u16(Tst_V64));
                int32x4_t  Ref_V128   = vmovl_s16(vreinterpret_s16_u16(Ref_V64));
                int32x4_t Diff_V128   = vsubq_s32   (Tst_V128, Ref_V128);
                SD_V128               = vaddq_s32   (SD_V128, Diff_V128);
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
        SD += vdups_lane_s32 (vget_high_s32(Tmp2V), 0);
        return SD;
    }
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
    
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    return 0;
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  
return 0;
}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
 return 0;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
