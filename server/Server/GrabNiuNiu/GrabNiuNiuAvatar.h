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
		
		// 获取加入时间
		time_t getJoinTick() const { return _joinTick; }
		void setJoinTick(time_t t) { _joinTick = t; }
		
		// 获取下次行动时间
		time_t getNextActionTick() const { return _nextActionTick; }
		void setNextActionTick(time_t t) { _nextActionTick = t; }

	public:
		int grabMultiple = -1; // 抢庄倍数，-1表示未操作，0表示不抢，1-4表示抢庄倍数
		int betMultiple = 0;   // 下注倍数，0表示未下注，1-5表示下注倍数
		int64_t score = 0;     // 本局得分

	private:
		time_t _joinTick = 0;       // 加入房间时间
		time_t _nextActionTick = 0; // 下次行动时间
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_AVATAR_H_
