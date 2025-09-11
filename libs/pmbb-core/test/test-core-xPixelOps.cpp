/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <functional>
#include <random>

#include "../src/xCommonDefCORE.h"
#include "../src/xPixelOps.h"
#include "../src/xPic.h"
#include "../src/xPlane.h"
#include "../src/xTestUtils.h"
#include "xTimeUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32> c_Dimms = {128, 127, 129, 512, 511, 513};
static const std::vector<int32> c_Margs = {0, 4, 32};
static const std::vector<int32> c_BitDs = {8, 10, 12, 14};
static constexpr int32 c_DefBitDepth = 14;
static constexpr int32 c_DefMaxValue = (1 << c_DefBitDepth) - 1;
static constexpr int32 c_NumRandomTests = 8;

static const int32 c_PerfUnitSize = 512;
static const int32 c_PerfNumIters = 1000;
static const int32 c_PerfBitDep = 14; // for checkifinrange
//===============================================================================================================================================================================================================

void testCopy()
{
  std::random_device RandomDevice;              // Will be used to obtain a seed for the random number engine
  std::mt19937 RandomGenerator(RandomDevice()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        // buffers create
        xPlane<uint16> *Src = new xPlane<uint16>(Size, 14, m);
        xPlane<uint16> *Dst = new xPlane<uint16>(Size, 14, m);

        for (int32 n = 0; n < c_NumRandomTests; n++)
        {
          Src->fill(0);
          uint32 Seed = RandomDistribution(RandomGenerator);
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, Seed);
          CAPTURE(Description + fmt::format(" Seed={}", Seed));

          if (m == 0)
          {
            Dst->fill(0);
            xPixelOps::Copy(Dst->getAddr(), Src->getAddr(), Dst->getArea());
            CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
          }

          Dst->fill(0);
          xPixelOps::Copy(Dst->getAddr(), Src->getAddr(), Dst->getStride(), Src->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

          Dst->fill(0);
          xPixelOps::CopyPart(Dst->getAddr(), Src->getAddr(), Dst->getStride(), Src->getStride(), {0, 0}, {0, 0}, {Dst->getWidth(), Dst->getHeight()});
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        // buffers destroy
        delete Src;
        delete Dst;
      }
    }
  }
}

void testCvt(
    std::function<void(uint16 *, const uint8 *, int32, int32, int32, int32)> CvtU8toU16,
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtU16toU8)
{
  std::random_device RandomDevice;              // Will be used to obtain a seed for the random number engine
  std::mt19937 RandomGenerator(RandomDevice()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        // buffers create
        xPlane<uint16> *Src = new xPlane<uint16>(Size, 8, m);
        xPlane<uint8> *Imm = new xPlane<uint8>(Size, 8, m);
        xPlane<uint16> *Dst = new xPlane<uint16>(Size, 8, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for (int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::format(" Seed={}", Seed));
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 8, Seed);
          CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        // clip cases
        CAPTURE(Description + " Fill=256");
        Src->fill(256);
        CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
        CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
        CHECK(xTestUtils::isEqualValue(Dst->getAddr(), Dst->getStride(), Dst->getWidth(), Dst->getHeight(), (uint16)255));

        CAPTURE(Description + " Fill=c_Max14bit");
        Src->fill(c_DefMaxValue);
        CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
        CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
        CHECK(xTestUtils::isEqualValue(Dst->getAddr(), Dst->getStride(), Dst->getWidth(), Dst->getHeight(), (uint16)255));

        // buffers destroy
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

std::tuple<flt64, flt64> perfCvt(
    std::function<void(uint16 *, const uint8 *, int32, int32, int32, int32)> CvtU8toU16,
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtU16toU8)
{
  const int32V2 Size = {c_PerfUnitSize, c_PerfUnitSize};

  xPlane<uint16> *Src = new xPlane<uint16>(Size, 8, 0);
  xPlane<uint8> *Imm = new xPlane<uint8>(Size, 8, 0);
  xPlane<uint16> *Dst = new xPlane<uint16>(Size, 8, 0);

  uint32 State;
  State = xTestUtils::fillMidNoise(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), Src->getBitDepth(), 0);
  Imm->fill(0);
  Dst->fill(0);

  tDuration AT = (tDuration)0;
  tDuration BT = (tDuration)0;

  // warmup
  CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
  CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
  CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

  // measure
  for (int32 j = 0; j < c_PerfNumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
    tTimePoint T1 = tClock::now();
    CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
    tTimePoint T2 = tClock::now();
    CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

    AT += T1 - T0;
    BT += T2 - T1;
  }

  int64 NumBytes = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSecAT = NumBytes / std::chrono::duration_cast<tDurationS>(AT).count();
  flt64 BytesPerSecBT = NumBytes / std::chrono::duration_cast<tDurationS>(BT).count();

  return {BytesPerSecAT, BytesPerSecBT};
}

void testResample(
    std::function<void(uint16 *, const uint16 *, int32, int32, int32, int32)> Upsample,
    std::function<void(uint16 *, const uint16 *, int32, int32, int32, int32)> Downsample,
    const int32V2 &SizeMultiplier)
{
  std::random_device RandomDevice;              // Will be used to obtain a seed for the random number engine
  std::mt19937 RandomGenerator(RandomDevice()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
        const int32V2 ImmSize = Size * SizeMultiplier;
        const int64 AreaMult = SizeMultiplier.getMul();

        // buffers create
        xPlane<uint16> *Src = new xPlane<uint16>(Size, 14, m);
        xPlane<uint16> *Imm = new xPlane<uint16>(ImmSize, 14, m);
        xPlane<uint16> *Dst = new xPlane<uint16>(Size, 14, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for (int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::format(" Seed={}", Seed));
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, Seed);
          int64 SumSrc = xTestUtils::calcSum(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight());
          Upsample(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          int64 SumImm = xTestUtils::calcSum(Imm->getAddr(), Imm->getStride(), Imm->getWidth(), Imm->getHeight());
          CHECK(AreaMult * SumSrc == SumImm);
          Downsample(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        // buffers destroy
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

std::tuple<flt64, flt64> perfResample(
    std::function<void(uint16 *, const uint16 *, int32, int32, int32, int32)> Upsample,
    std::function<void(uint16 *, const uint16 *, int32, int32, int32, int32)> Downsample,
    const int32V2 &SizeMultiplier)
{
  const int32V2 Size = {c_PerfUnitSize, c_PerfUnitSize};
  const int32V2 ImmSize = Size * SizeMultiplier;

  // buffers create
  xPlane<uint16> *Src = new xPlane<uint16>(Size, 14, 0);
  xPlane<uint16> *Imm = new xPlane<uint16>(ImmSize, 14, 0);
  xPlane<uint16> *Dst = new xPlane<uint16>(Size, 14, 0);

  uint32 State;
  State = xTestUtils::fillMidNoise(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), Src->getBitDepth(), 0);
  Imm->fill(0);
  Dst->fill(0);

  tDuration AT = (tDuration)0;
  tDuration BT = (tDuration)0;

  // warmup
  Upsample(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
  Downsample(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
  CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

  // measure
  for (int32 j = 0; j < c_PerfNumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    Upsample(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
    tTimePoint T1 = tClock::now();
    Downsample(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
    tTimePoint T2 = tClock::now();
    CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
    AT += T1 - T0;
    BT += T2 - T1;
  }

  int64 NumBytes = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSecAT = NumBytes / std::chrono::duration_cast<tDurationS>(AT).count();
  flt64 BytesPerSecBT = NumBytes / std::chrono::duration_cast<tDurationS>(BT).count();

  return {BytesPerSecAT, BytesPerSecBT};
}

void testCvtResample(
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtU16toU8,
    std::function<void(uint16 *, const uint8 *, int32, int32, int32, int32)> CvtUpsampleU8toU16,
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtDownsampleU16toU8,
    const int32V2 &SizeMultiplier)
{
  std::random_device RandomDevice;              // Will be used to obtain a seed for the random number engine
  std::mt19937 RandomGenerator(RandomDevice()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
        const int32V2 ImmSize = Size * SizeMultiplier;
        const int64 AreaMult = SizeMultiplier.getMul();

        // buffers create
        xPlane<uint16> *Pre = new xPlane<uint16>(Size, 8, m);
        xPlane<uint8> *Src = new xPlane<uint8>(Size, 8, m);
        xPlane<uint16> *Imm = new xPlane<uint16>(ImmSize, 8, m);
        xPlane<uint8> *Dst = new xPlane<uint8>(Size, 8, m);

        Pre->fill(0);
        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for (int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::format(" Seed={}", Seed));
          xTestUtils::fillRandom(Pre->getAddr(), Pre->getStride(), Pre->getWidth(), Pre->getHeight(), 8, Seed);
          CvtU16toU8(Src->getAddr(), Pre->getAddr(), Src->getStride(), Pre->getStride(), Src->getWidth(), Src->getHeight());
          int64 SumSrc = xTestUtils::calcSum(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight());
          CvtUpsampleU8toU16(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          int64 SumImm = xTestUtils::calcSum(Imm->getAddr(), Imm->getStride(), Imm->getWidth(), Imm->getHeight());
          CHECK(AreaMult * SumSrc == SumImm);
          CvtDownsampleU16toU8(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        // buffers destroy
        delete Pre;
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

std::tuple<flt64, flt64> perfCvtResample(
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtU16toU8,
    std::function<void(uint16 *, const uint8 *, int32, int32, int32, int32)> CvtUpsampleU8toU16,
    std::function<void(uint8 *, const uint16 *, int32, int32, int32, int32)> CvtDownsampleU16toU8,
    const int32V2 &SizeMultiplier)
{
  const int32V2 Size = {c_PerfUnitSize, c_PerfUnitSize};
  const int32V2 ImmSize = Size * SizeMultiplier;

  // buffers create
  xPlane<uint16> *Pre = new xPlane<uint16>(Size, 8, 0);
  xPlane<uint8> *Src = new xPlane<uint8>(Size, 8, 0);
  xPlane<uint16> *Imm = new xPlane<uint16>(ImmSize, 8, 0);
  xPlane<uint8> *Dst = new xPlane<uint8>(Size, 8, 0);

  uint32 State = xTestUtils::fillMidNoise(Pre->getAddr(), Pre->getStride(), Pre->getWidth(), Pre->getHeight(), Pre->getBitDepth(), 0);
  Src->fill(0);
  Imm->fill(0);
  Dst->fill(0);

  tDuration AT = (tDuration)0;
  tDuration BT = (tDuration)0;

  // warmup
  CvtU16toU8(Src->getAddr(), Pre->getAddr(), Src->getStride(), Pre->getStride(), Src->getWidth(), Src->getHeight());
  CvtUpsampleU8toU16(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
  CvtDownsampleU16toU8(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
  CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

  // measure
  for (int32 j = 0; j < c_PerfNumIters; j++)
  {
    CvtU16toU8(Src->getAddr(), Pre->getAddr(), Src->getStride(), Pre->getStride(), Src->getWidth(), Src->getHeight());
    tTimePoint T0 = tClock::now();
    CvtUpsampleU8toU16(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
    tTimePoint T1 = tClock::now();
    CvtDownsampleU16toU8(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
    tTimePoint T2 = tClock::now();
    CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
    AT += T1 - T0;
    BT += T2 - T1;
  }
  int64 NumBytes = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSecAT = NumBytes / std::chrono::duration_cast<tDurationS>(AT).count();
  flt64 BytesPerSecBT = NumBytes / std::chrono::duration_cast<tDurationS>(BT).count();

  return {BytesPerSecAT, BytesPerSecBT};
}

void testRearrange(
    std::function<void(uint16 *, const uint16 *, const uint16 *, const uint16 *, const uint16, int32, int32, int32, int32)> AOS4fromSOA3,
    std::function<void(uint16 *, uint16 *, uint16 *, const uint16 *, int32, int32, int32, int32)> SOA3fromAOS4)
{
  std::random_device RandomDevice;              // Will be used to obtain a seed for the random number engine
  std::mt19937 RandomGenerator(RandomDevice()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        // buffers create
        xPicP *SrcP = new xPicP(Size, 14, m);
        xPicI *ImmI = new xPicI(Size, 14, m);
        xPicP *DstP = new xPicP(Size, 14, m);

        SrcP->fill(0);
        ImmI->fill(0);
        DstP->fill(0);

        // simple deterministic test
        {
          CAPTURE(Description + fmt::format(" SimpleDeterministic"));
          SrcP->fill(0);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C0), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14, 0);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C1), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14, 100);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C2), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14, 200);
          ImmI->fill(0);
          DstP->fill(0);
          AOS4fromSOA3((uint16 *)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 16384, ImmI->getStride() * 4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
          SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16 *)ImmI->getAddr(), DstP->getStride(), ImmI->getStride() * 4, DstP->getWidth(), DstP->getHeight());
          for (int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
          }
        }

        // random test
        SrcP->fill(0);
        for (int32 n = 0; n < c_NumRandomTests; n++)
        {
          CAPTURE(Description + fmt::format(" RandomTestCnt={}", n));
          for (int32 c = 0; c < 3; c++)
          {
            xTestUtils::fillRandom(SrcP->getAddr((eCmp)c), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 8, RandomDistribution(RandomGenerator));
          }
          ImmI->fill(0);
          DstP->fill(0);
          AOS4fromSOA3((uint16 *)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 0, ImmI->getStride() * 4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
          SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16 *)ImmI->getAddr(), DstP->getStride(), ImmI->getStride() * 4, DstP->getWidth(), DstP->getHeight());
          for (int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
          }
        }

        // buffers destroy
        delete SrcP;
        delete ImmI;
        delete DstP;
      }
    }
  }
}

std::tuple<flt64, flt64> perfRearrange(
    std::function<void(uint16 *, const uint16 *, const uint16 *, const uint16 *, const uint16, int32, int32, int32, int32)> AOS4fromSOA3,
    std::function<void(uint16 *, uint16 *, uint16 *, const uint16 *, int32, int32, int32, int32)> SOA3fromAOS4)
{
    const int32V2 Size = { c_PerfUnitSize, c_PerfUnitSize };
  // buffers create
  xPicP *SrcP = new xPicP(Size, 14, 0);
  xPicI *ImmI = new xPicI(Size, 14, 0);
  xPicP *DstP = new xPicP(Size, 14, 0);

  uint16 State;
  State = xTestUtils::fillMidNoise(SrcP->getAddr(eCmp::C0), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), SrcP->getBitDepth(), 0);
  State = xTestUtils::fillMidNoise(SrcP->getAddr(eCmp::C1), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), SrcP->getBitDepth(), 0);
  State = xTestUtils::fillMidNoise(SrcP->getAddr(eCmp::C2), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), SrcP->getBitDepth(), 0);
  ImmI->fill(0);
  DstP->fill(0);

  tDuration AT = (tDuration)0;
  tDuration BT = (tDuration)0;

  // warmup
  AOS4fromSOA3((uint16 *)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 16384, ImmI->getStride() * 4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
  SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16 *)ImmI->getAddr(), DstP->getStride(), ImmI->getStride() * 4, DstP->getWidth(), DstP->getHeight());
  for (int32 c = 0; c < 3; c++)
  {
    CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
  }

  //measure
for(int32 j = 0; j < c_PerfNumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    AOS4fromSOA3((uint16 *)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 16384, ImmI->getStride() * 4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
    tTimePoint T1 = tClock::now();
    SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16 *)ImmI->getAddr(), DstP->getStride(), ImmI->getStride() * 4, DstP->getWidth(), DstP->getHeight());
    tTimePoint T2 = tClock::now();
  for (int32 c = 0; c < 3; c++)
  {
    CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
  }
    AT += T1 - T0;
    BT += T2 - T1;
  }
  int64 NumBytes = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSecAT = NumBytes / std::chrono::duration_cast<tDurationS>(AT).count();
  flt64 BytesPerSecBT = NumBytes / std::chrono::duration_cast<tDurationS>(BT).count();

  return {BytesPerSecAT, BytesPerSecBT};
}

void testCheckIfInRange(std::function<bool(const uint16 *, int32, int32, int32, int32)> CheckIfInRange)
{
  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        for (const int32 b : c_BitDs)
        {
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
          CAPTURE(Description);

          const int32 MaxValue = xBitDepth2MaxValue(b);

          // buffers create
          xPlane<uint16> *P = new xPlane<uint16>(Size, b, m);

          P->fill(0);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

          P->accessPel({0, 0}) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({0, 0}) = 0;

          P->accessPel({5, 9}) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({5, 9}) = 0;

          P->accessPel({x - 1, 0}) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({x - 1, 0}) = 0;

          P->accessPel({0, y - 1}) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({0, y - 1}) = 0;

          P->accessPel({x - 1, y - 1}) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({x - 1, y - 1}) = 0;

          P->fill(uint16(MaxValue));
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

          // buffers destroy
          delete P;
        }
      }
    }
  }
}

flt64 perfCheckIfInRange(std::function<bool(const uint16 *, int32, int32, int32, int32)> CheckIfInRange)
{
  const int32V2 Size = { c_PerfUnitSize, c_PerfUnitSize };
  const int32 b = c_PerfBitDep;

  xPlane<uint16> *P = new xPlane<uint16>(Size, b,  0);

  uint32 State = xTestUtils::fillMidNoise(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), P->getBitDepth(), 0);

  tDuration AT = (tDuration)0;

  //warmup
  CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

    //measure
  for(int32 j = 0; j < c_PerfNumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    bool ResultA = CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b);
    tTimePoint T1 = tClock::now();
    bool ResultB = CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b);

    CHECK(ResultA == ResultB);
    AT += T1 - T0;
  }

  int64 NumBytes      = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSecT = NumBytes / std::chrono::duration_cast<tDurationS>(AT).count();

  return BytesPerSecT;
}

void testCountNonZero(std::function<int32(const uint16 *, int32, int32, int32)> CountNonZero)
{
  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};
      int64 Area = x * y;

      for (const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
        CAPTURE(Description);

        // buffers create
        xPlane<uint16> *P = new xPlane<uint16>(Size, 14, m);

        P->fill(0);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 0);
        P->accessPel({0, 0}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 1);
        P->accessPel({5, 9}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 2);
        P->accessPel({1, 1}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 3);
        P->accessPel({34, 19}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 4);
        P->accessPel({x - 1, 0}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 5);
        P->accessPel({0, y - 1}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 6);
        P->accessPel({x - 1, y - 1}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 7);

        P->fill(c_DefMaxValue);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 0);
        P->accessPel({0, 0}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 1);
        P->accessPel({5, 9}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 2);
        P->accessPel({1, 1}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 3);
        P->accessPel({34, 19}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 4);
        P->accessPel({x - 1, 0}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 5);
        P->accessPel({0, y - 1}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 6);
        P->accessPel({x - 1, y - 1}) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 7);

        // buffers destroy
        delete P;
      }
    }
  }
}

void testCompareEqual(std::function<bool(const uint16 *, const uint16 *, int32, int32, int32, int32)> CompareEqual)
{
  for (const int32 y : c_Dimms)
  {
    for (const int32 x : c_Dimms)
    {
      int32V2 Size = {x, y};

      for (const int32 m : c_Margs)
      {
        for (const int32 b : c_BitDs)
        {
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
          CAPTURE(Description);

          const int32 MaxValue = xBitDepth2MaxValue(b);

          // buffers create
          xPlane<uint16> *R = new xPlane<uint16>(Size, b, m);
          xPlane<uint16> *T = new xPlane<uint16>(Size, b, m);

          R->fill(0);
          T->fill(0);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == true);

          T->accessPel({0, 0}) = uint16(MaxValue + 1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({0, 0}) = 0;

          T->accessPel({5, 9}) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({5, 9}) = 0;

          T->accessPel({x - 1, 0}) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({x - 1, 0}) = 0;

          T->accessPel({0, y - 1}) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({0, y - 1}) = 0;

          T->accessPel({x - 1, y - 1}) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({x - 1, y - 1}) = 0;

          R->fill(uint16(MaxValue));
          T->fill(uint16(MaxValue));
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == true);

          // buffers destroy
          delete R;
          delete T;
        }
      }
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("xPixelOps::Copy")
{
  tTimePoint T = tClock::now();
  testCopy();
  fmt::print("TIME(xPixelOps::Copy) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

TEST_CASE("xPixelOpsSTD")
{
  tTimePoint T = tClock::now();
  testCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt));
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleHV),
      {2, 2});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleHV),
      {2, 2});
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleH),
      {2, 1});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleH),
      {2, 1});
  testRearrange(
      &xPixelOpsSTD::AOS4fromSOA3,
      &xPixelOpsSTD::SOA3fromAOS4);
  testCheckIfInRange(
      &xPixelOpsSTD::CheckIfInRange);
  testCountNonZero(
      &xPixelOpsSTD::CountNonZero);
  testCompareEqual(
      &xPixelOpsSTD::CompareEqual);
  fmt::print("TIME(xPixelOpsSTD) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

#if X_SIMD_CAN_USE_NEON
TEST_CASE("xPixelOpsNEON")
{
  tTimePoint T = tClock::now();
  testCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt));
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::DownsampleHV),
      {2, 2});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtDownsampleHV),
      {2, 2});
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::DownsampleH),
      {2, 1});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtDownsampleH),
      {2, 1});
  testRearrange(
      &xPixelOpsNEON::AOS4fromSOA3,
      &xPixelOpsNEON::SOA3fromAOS4 // needs fix
  );
  testCheckIfInRange(
      &xPixelOpsNEON::CheckIfInRange);
  testCountNonZero(
      &xPixelOpsNEON::CountNonZero);
  testCompareEqual(
      &xPixelOpsNEON::CompareEqual);
  fmt::print("TIME(xPixelOpsNEON) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xPixelOpsSSE")
{
  tTimePoint T = tClock::now();
  testCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt));
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::DownsampleHV),
      {2, 2});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtDownsampleHV),
      {2, 2});
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::DownsampleH),
      {2, 1});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtDownsampleH),
      {2, 1});
  testRearrange(
      &xPixelOpsSSE::AOS4fromSOA3,
      &xPixelOpsSSE::SOA3fromAOS4);
  testCheckIfInRange(
      &xPixelOpsSSE::CheckIfInRange);
  testCountNonZero(
      &xPixelOpsSSE::CountNonZero);
  testCompareEqual(
      &xPixelOpsSSE::CompareEqual);
  fmt::print("TIME(xPixelOpsSSE) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xPixelOpsAVX")
{
  tTimePoint T = tClock::now();
  testCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt));
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::DownsampleHV),
      {2, 2});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtDownsampleHV),
      {2, 2});
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::DownsampleH),
      {2, 1});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtDownsampleH),
      {2, 1});

  testRearrange(
      &xPixelOpsAVX::AOS4fromSOA3,
      &xPixelOpsAVX::SOA3fromAOS4);
  testCheckIfInRange(
      &xPixelOpsAVX::CheckIfInRange);
  testCountNonZero(
      &xPixelOpsAVX::CountNonZero);
  testCompareEqual(
      &xPixelOpsAVX::CompareEqual);
  fmt::print("TIME(xPixelOpsAVX) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xPixelOpsAVX512")
{
  tTimePoint T = tClock::now();
  testCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt));
  testResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::DownsampleHV),
      {2, 2});
  testCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsAVX512::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD ::CvtDownsampleHV),
      {2, 2});
  // testResample
  //(
  //   static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::UpsampleH  ),
  //   static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::DownsampleH),
  //   { 2,1 }
  //);
  // testCvtResample
  //(
  //   static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt           ),
  //   static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX512::CvtUpsampleH  ),
  //   static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::CvtDownsampleH),
  //   { 2,1 }
  //);
  testRearrange(
      &xPixelOpsAVX512::AOS4fromSOA3,
      &xPixelOpsAVX512::SOA3fromAOS4);
  testCheckIfInRange(
      &xPixelOpsAVX512::CheckIfInRange);
  testCountNonZero(
      &xPixelOpsAVX512::CountNonZero);
  testCompareEqual(
      &xPixelOpsAVX512::CompareEqual);
  fmt::print("TIME(xPixelOpsAVX512) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

// performance tests
TEST_CASE("xPixelOpsSTD")
{
  auto [AT1, BT1] = perfCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt));
  fmt::print("TIME(xPixelOpsSTD::Cvt) = {:.2f} MiB/s\n", AT1 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::Cvt) = {:.2f} MiB/s\n", BT1 / (1024 * 1024));

  auto [AT2, BT2] = perfResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleHV),
      {2, 2});
  fmt::print("TIME(xPixelOpsSTD::UpsampleHV) = {:.2f} MiB/s\n", AT2 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::DownsampleHV) = {:.2f} MiB/s\n", BT2 / (1024 * 1024));

  auto [AT3, BT3] = perfCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleHV),
      {2, 2});
  fmt::print("TIME(xPixelOpsSTD::CvtUpsampleHV) = {:.2f} MiB/s\n", AT3 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::CvtDownsampleHV) = {:.2f} MiB/s\n", BT3 / (1024 * 1024));

  auto [AT21, BT21] = perfResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleH),
      {2, 1});
  fmt::print("TIME(xPixelOpsSTD::UpsampleH) = {:.2f} MiB/s\n", AT21 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::DownsampleH) = {:.2f} MiB/s\n", BT21 / (1024 * 1024));

  auto [AT32, BT32] = perfCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleH),
      {2, 1});
  fmt::print("TIME(xPixelOpsSTD::UpsampleH) = {:.2f} MiB/s\n", AT32 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::DownsampleH) = {:.2f} MiB/s\n", BT32 / (1024 * 1024));

  auto [AT4, BT4] = perfRearrange(
      &xPixelOpsSTD::AOS4fromSOA3,
      &xPixelOpsSTD::SOA3fromAOS4);
  fmt::print("TIME(xPixelOpsSTD::AOS4fromSOA3) = {:.2f} MiB/s\n", AT4 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsSTD::SOA3fromAOS4) = {:.2f} MiB/s\n", BT4 / (1024 * 1024));

  auto T = perfCheckIfInRange(
      &xPixelOpsSTD::CheckIfInRange);
  fmt::print("TIME(xPixelOpsSTD::CheckIfInRange) = {:.2f} MiB/s\n", T / (1024 * 1024));
  
}
TEST_CASE("xPixelOpsNEON")
{
  auto [AT1, BT1] = perfCvt(
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt));
  fmt::print("TIME(xPixelOpsNEON::Cvt) = {:.2f} MiB/s\n", AT1 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::Cvt) = {:.2f} MiB/s\n", BT1 / (1024 * 1024));

  auto [AT2, BT2] = perfResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::UpsampleHV),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::DownsampleHV),
      {2, 2});
  fmt::print("TIME(xPixelOpsNEON::UpsampleHV) = {:.2f} MiB/s\n", AT2 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::DownsampleHV) = {:.2f} MiB/s\n", BT2 / (1024 * 1024));

  auto [AT3, BT3] = perfCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtUpsampleHV),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtDownsampleHV),
      {2, 2});
  fmt::print("TIME(xPixelOpsNEON::CvtUpsampleHV) = {:.2f} MiB/s\n", AT3 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::CvtDownsampleHV) = {:.2f} MiB/s\n", BT3 / (1024 * 1024));

  auto [AT21, BT21] = perfResample(
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::UpsampleH),
      static_cast<void (*)(uint16 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::DownsampleH),
      {2, 1});
  fmt::print("TIME(xPixelOpsNEON::UpsampleH) = {:.2f} MiB/s\n", AT21 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::DownsampleH) = {:.2f} MiB/s\n", BT21 / (1024 * 1024));

  auto [AT32, BT32] = perfCvtResample(
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::Cvt),
      static_cast<void (*)(uint16 *, const uint8 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtUpsampleH),
      static_cast<void (*)(uint8 *, const uint16 *, int32, int32, int32, int32)>(&xPixelOpsNEON::CvtDownsampleH),
      {2, 1});
  fmt::print("TIME(xPixelOpsNEON::UpsampleH) = {:.2f} MiB/s\n", AT32 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::DownsampleH) = {:.2f} MiB/s\n", BT32 / (1024 * 1024));

  auto [AT4, BT4] = perfRearrange(
      &xPixelOpsNEON::AOS4fromSOA3,
      &xPixelOpsNEON::SOA3fromAOS4);
  fmt::print("TIME(xPixelOpsNEON::AOS4fromSOA3) = {:.2f} MiB/s\n", AT4 / (1024 * 1024));
  fmt::print("TIME(xPixelOpsNEON::SOA3fromAOS4) = {:.2f} MiB/s\n", BT4 / (1024 * 1024));

  auto T = perfCheckIfInRange(
      &xPixelOpsNEON::CheckIfInRange);
  fmt::print("TIME(xPixelOpsNEON::CheckIfInRange) = {:.2f} MiB/s\n", T / (1024 * 1024));
}