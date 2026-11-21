#pragma once
#include <cmath>

// Las 16 curvas de ganancia. p = fase del ciclo (0..1), devuelve ganancia 0..1
// (0 = ducking maximo, 1 = sin atenuacion).
namespace mb
{
    constexpr int kNumCurves = 16;

    inline float clamp01 (float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
    inline float smooth (float x)  { x = clamp01 (x); return x * x * (3.f - 2.f * x); }

    inline float curveValue (int idx, float p)
    {
        p = clamp01 (p);
        constexpr float pi = 3.14159265f;

        switch (idx)
        {
            case 0:  return 1.f - (1.f - p) * (1.f - p);
            case 1:  { float q = 1.f - p; return 1.f - q * q * q; }
            case 2:  return smooth ((p - 0.04f) / 0.5f);      // S-curve (por defecto)
            case 3:  return smooth ((p - 0.15f) / 0.6f);
            case 4:  return smooth ((p - 0.10f) / 0.35f);
            case 5:  return p < 0.3f ? 0.f : 1.f;              // gate duro
            case 6:  return p;
            case 7:  return 0.5f - 0.5f * std::cos (pi * p);
            case 8:  { float q = 1.f - p; q *= q; return 1.f - q * q; }
            case 9:  return p * p;
            case 10: return smooth (p / 0.6f);
            case 11: return std::sqrt (p);
            case 12: return smooth ((p - 0.25f) / 0.35f);
            case 13: return smooth ((p - 0.5f) / 0.4f);
            case 14: { float q = p * 2.f; q -= std::floor (q); return smooth (q / 0.8f); } // doble bombeo
            default: return 0.5f - 0.5f * std::cos (3.f * pi * p);
        }
    }
}
