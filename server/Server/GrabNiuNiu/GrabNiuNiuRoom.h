// GrabNiuNiuRoom.h

#ifndef _NIU_MA_GRAB_NIU_NIU_ROOM_H_
#define _NIU_MA_GRAB_NIU_NIU_ROOM_H_

#include "Game/GameRoom.h"

namespace NiuMa
{
	class GrabNiuNiuRoom : public GameRoom
	{
	public:
		GrabNiuNiuRoom(const std::string& venueId, const std::string& number, int mode, int diZhu);
		virtual ~GrabNiuNiuRoom();

	public:
		// 游戏状态
		enum class GameState : int
		{
			WaitStart,		// 等待开始
			Deal4,			// 发4张牌
			GrabBanker,		// 抢庄
			Betting,		// 下注
			Deal1,			// 发最后1张牌
			Compare,		// 比牌
			Settlement		// 结算
		};

	public:
		virtual void initialize() override;
		virtual void onTimer() override;
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;
		virtual void onConnect(const std::string& playerId) override;

	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg, bool robot = false) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const override;
		virtual void onAvatarJoined(int seat, const std::string& playerId) override;
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;

	private:
		void setState(GameState s);
		int getStateElapsed() const;

	private:
		const std::string _number;
		int _mode;
		int _diZhu;
		GameState _gameState;
		time_t _stateTime;
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_ROOM_H_
