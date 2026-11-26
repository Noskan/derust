#pragma once

#include "effects/TMEffect.h"
#include "common/Structures.h"

class TMEffectParticle : public TMEffect
{
public:
    TMEffectParticle(TMVector3 vecPosition, int nType, int nCount, float fSize, unsigned int dwColor,
		int bRand, int nTextureIndex, float fRadius, int bBright, TMVector3 vecDir, unsigned int dwLifeTime);
    ~TMEffectParticle();
    
    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    float m_fAngle;
    int m_nType;
};