#include "GrabNiuNiuAvatar.h"

namespace NiuMa {

	GrabNiuNiuAvatar::GrabNiuNiuAvatar(const std::string& accountId, const std::string& password, int seat)
		: Avatar(accountId, password, seat),
		  isReady(false),
		  isGrabbed(false),
		  grabMultiple(0),
		  isBetted(false),
		  betMultiple(1),
		  genreType(0),
		  winScore(0)
	{
	}

	GrabNiuNiuAvatar::~GrabNiuNiuAvatar() {
	}

	void GrabNiuNiuAvatar::reset() {
		isReady = false;
		isGrabbed = false;
		grabMultiple = 0;
		isBetted = false;
		betMultiple = 1;
		cards.clear();
		genreType = 0;
		winScore = 0;
	}

}