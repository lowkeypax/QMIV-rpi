/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xDistortionSSE.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

===============================================================================================================================================================================================================
int32 xDistortionNEON::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  

}
int32 xDistortionNEON::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  
}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{

}
uint32 xDistortionNEON::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{

}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{  

}
uint64 xDistortionNEON::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
 
}
int64 xDistortionNEON::CalcWeightedSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 Area)
{

}
int64 xDistortionNEON::CalcWeightedSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{

}
uint64 xDistortionNEON::CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 Area)
{

}
uint64 xDistortionNEON::CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{

}

===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
