#ifndef _GRAB_NIUNIU_ROOM_H_
#define _GRAB_NIUNIU_ROOM_H_

#include "Venue.h"
#include "GrabNiuNiuMessages.h"
#include "GrabNiuNiuAvatar.h"
#include "NiuNiuRule.h"
#include "Dealer.h"

namespace NiuMa {

	class GrabNiuNiuRoom : public Venue {
	public:
		GrabNiuNiuRoom(const std::string& id, int mode, long long dizhu, int config, long long limit, int capacity);
		virtual ~GrabNiuNiuRoom();

		virtual void onInit() override;
		virtual void onTick(long long elapsed) override;
		virtual void onMessage(Avatar* avatar, const std::string& msgType, const char* msg, int len) override;
		virtual void onAvatarLeave(Avatar* avatar) override;

	protected:
		virtual Avatar* createAvatar(const std::string& accountId, const std::string& password, int seat) override;
		
		// 客户端请求
		void onMsgGrabNiuSync(Avatar* avatar, const char* msg, int len);
		void onMsgGrabNiuGrab(Avatar* avatar, const char* msg, int len);
		void onMsgGrabNiuBet(Avatar* avatar, const char* msg, int len);
		void onMsgPlayerReady(Avatar* avatar, const char* msg, int len);

		// 状态机流转控制
		void checkStartGame();
		void changeState(GrabNiuNiuState state, long long timeoutMs);
		void onStateTimeout();

		// 游戏核心流程
		void beginDeal4();
		void beginGrabBanker();
		void endGrabBanker();
		void beginBetting();
		void endBetting();
		void beginDeal1();
		void beginCompare();
		void beginSettlement();

		// 辅助方法
		int getGenreMultiple(int genre) const;
		GrabNiuNiuAvatar* getBankerAvatar() const;
		void broadcastState();

	private:
		GrabNiuNiuState _gameState;
		long long _stateTimer;			// 状态倒计时(毫秒)
		
		NiuNiuRule _rule;
		Dealer _dealer;
		std::string _bankerId;			// 当前庄家ID
		int _bankerMultiple;			// 庄家抢庄倍数

		int _readyCount;				// 准备人数
		int _grabbedCount;				// 已抢庄人数
		int _bettedCount;				// 已下注人数
	};

}

#endif