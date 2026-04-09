// GrabNiuNiuAvatar.cpp

#include "GrabNiuNiuAvatar.h"
#include "Base/BaseUtils.h"

namespace NiuMa
{
	GrabNiuNiuAvatar::GrabNiuNiuAvatar(const std::string& playerId, bool robot)
		: GameAvatar(playerId, robot)
	{
		_joinTick = BaseUtils::getCurrentMillisecond();
	}

	GrabNiuNiuAvatar::~GrabNiuNiuAvatar()
	{
	}

	void GrabNiuNiuAvatar::reset()
	{
		grabMultiple = -1;
		betMultiple = 0;
		score = 0;
		_nextActionTick = 0;
	}
}
