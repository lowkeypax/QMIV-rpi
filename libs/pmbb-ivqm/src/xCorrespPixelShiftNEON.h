#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCorrespPixelShiftNEON
{
public:
  static constexpr bool c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;

  static uint64V4 CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);

protected:
  static int32x4_t  xCalcDistWithinBlock (const int32x4_t& TstPel, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32x4_t& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
