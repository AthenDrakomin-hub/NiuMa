#ifndef _GRAB_NIUNIU_AVATAR_H_
#define _GRAB_NIUNIU_AVATAR_H_

#include "Avatar.h"
#include <vector>

namespace NiuMa {

	class GrabNiuNiuAvatar : public Avatar {
	public:
		GrabNiuNiuAvatar(const std::string& accountId, const std::string& password, int seat);
		virtual ~GrabNiuNiuAvatar();

		void reset();

		// 玩家手牌 (发4张 + 发最后1张)
		std::vector<int> cards;
		int genreType;

		// 玩家动作状态
		bool isReady;
		bool isGrabbed;         // 是否已经进行了抢庄操作
		int grabMultiple;       // 抢庄倍数 (0表示不抢)
		bool isBetted;          // 是否已经下注
		int betMultiple;        // 下注倍数
		long long winScore;     // 本局输赢

	};
}

#endif