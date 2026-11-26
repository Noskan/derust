#include "core/pch.h"
#include "entities/TMFlail.h"
#include "effects/TMEffectBillBoard.h"

TMFlail::TMFlail()
	: TMObject()
{
}

TMFlail::~TMFlail()
{
}

int TMFlail::FrameMove(unsigned int dwServerTime)
{
	return 1;
}

int TMFlail::Render()
{
	return 1;
}
