// GrabNiuNiuAvatar.h

#ifndef _NIU_MA_GRAB_NIU_NIU_AVATAR_H_
#define _NIU_MA_GRAB_NIU_NIU_AVATAR_H_

#include "Game/GameAvatar.h"

namespace NiuMa
{
	class GrabNiuNiuAvatar : public GameAvatar
	{
	public:
		GrabNiuNiuAvatar(const std::string& playerId, bool robot);
		virtual ~GrabNiuNiuAvatar();

	public:
		void reset();

	public:
		int grabMultiple = 0; // 抢庄倍数，-1表示未操作
		int betMultiple = 0;  // 下注倍数，0表示未下注
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_AVATAR_H_
