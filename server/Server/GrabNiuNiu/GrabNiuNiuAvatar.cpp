// GrabNiuNiuAvatar.cpp

#include "GrabNiuNiuAvatar.h"

namespace NiuMa
{
	GrabNiuNiuAvatar::GrabNiuNiuAvatar(const std::string& playerId, bool robot)
		: GameAvatar(playerId, robot)
	{
	}

	GrabNiuNiuAvatar::~GrabNiuNiuAvatar()
	{
	}

	void GrabNiuNiuAvatar::reset()
	{
		grabMultiple = -1;
		betMultiple = 0;
	}
}
