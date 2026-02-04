#pragma once
#include <string>

// Option B: richer triage (educational)
inline int calculateAcuity2(int pain, int hr, int sbp, int spo2, int rr, int tempC,
    bool unconscious, bool severeBleeding,
    bool chestPain, bool strokeSymptoms)
{
    // Hard-stop red flags → acuity 1
    if (unconscious || severeBleeding || spo2 < 85 || sbp < 80 ||
        hr < 40 || hr > 140 || rr < 8 || rr > 30 || strokeSymptoms)
        return 1;

    int score = 0;

    if (pain >= 9) score += 1;
    if (hr < 50 || hr > 120) score += 1;
    if (hr < 45 || hr > 130) score += 1;
    if (sbp < 90) score += 1;
    if (sbp < 85) score += 1;
    if (spo2 < 92) score += 1;
    if (spo2 < 90) score += 1;
    if (rr < 10 || rr > 22) score += 1;
    if (rr < 8 || rr > 28) score += 1;
    if (tempC <= 35 || tempC >= 39) score += 1;
    if (tempC < 35 || tempC >= 40) score += 1;
    if (chestPain) score += 1;

    if (score >= 6) return 2;
    if (score == 5) return 3;
    if (score == 3 || score == 4) return 4;
    return 5;
}
