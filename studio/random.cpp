/*
 ---------------------------------------------------------------------
 Tunefish 4  -  http://tunefish-synth.com
 ---------------------------------------------------------------------
 This file is part of Tunefish.

 Tunefish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Tunefish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Tunefish.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#if defined(eRELEASE) && defined(ePLAYER)
#include <windows.h>
#endif

#include "system.hpp"
#include <time.h>

eRandom::eRandom()
{
    Seed(0);
}

eRandom::eRandom(eU32 initSeed)
{
    Seed(initSeed);
}

void eRandom::Seed(eU32 newSeed)
{
    // seed may not be 0 => +1, take care of overflows
    m_seed = (newSeed == eU32_MAX ? 1 : newSeed+1);
}

void eRandom::SeedRandomly()
{
#if defined(eRELEASE) && defined(ePLAYER)
    eU64 curTime;
    QueryPerformanceCounter((LARGE_INTEGER *)&curTime);
    Seed(static_cast<eU32>(curTime));
#else
    Seed(static_cast<eU32>(clock()));
#endif
}

eU32 eRandom::NextInt()
{
    eU32 lo = 16807*(m_seed&0xffff);
    eU32 hi = 16807*(m_seed>>16);
    lo += (hi&0x7fff)<<16;
    hi >>= 15;
    lo += hi;
    lo = (lo&0x7fffffff)+(lo>>31);
    m_seed = lo;
    return m_seed;
}

eInt eRandom::NextInt(eInt min, eInt max)
{
    return NextInt()%(max-min)+min;
}

eF32 eRandom::NextFloat()
{
    return (eF32)NextInt()/(eF32)Max();
}

eF32 eRandom::NextFloat(eF32 min, eF32 max)
{
    return NextFloat()*(max-min)+min;
}

eF32 eRandom::NextFloatNormalDistribution(eF32 Mean, eF32 StdDeviation)
{
    // Box-Muller Transform
    eF32 x1, x2, w, y1, y2;

    do {
        x1 = 2.0f * NextFloat() - 1.0f;
        x2 = 2.0f * NextFloat() - 1.0f;
        w = x1 * x1 + x2 * x2;
    } while (w >= 1.0);

    w = eSqrt((-2.0f * eLogE(w)) / w);
    y1 = x1 * w;
    y2 = x2 * w;

    return Mean + y1 * StdDeviation;
}

eU32 eRandom::Max() const
{
    return 2147483647;
}
