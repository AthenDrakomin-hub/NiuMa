// GrabNiuNiuRoom.h

#ifndef _NIU_MA_GRAB_NIU_NIU_ROOM_H_
#define _NIU_MA_GRAB_NIU_NIU_ROOM_H_

#include "Game/GameRoom.h"
#include "PokerDealer.h"
#include "PokerGenre.h"
#include "NiuNiuRule.h"
#include "../GameDefines.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace NiuMa
{
	class GrabNiuNiuAvatar;
	
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
			Deal4,			// 发4张明牌
			GrabBanker,		// 抢庄
			Betting,		// 下注
			Deal1,			// 发最后1张暗牌
			Compare,		// 比牌
			Settlement		// 结算
		};

		// 房间配置
		struct RoomConfig {
			int minPlayers = 2;			// 最少玩家数
			int maxPlayers = 5;			// 最大玩家数
			int64_t minGold = 1000;		// 最小金币要求
			int baseTime = 15;			// 基础时间(秒)
			int grabTime = 5;			// 抢庄时间(秒)
			int betTime = 8;			// 下注时间(秒)
			int compareTime = 5;		// 比牌时间(秒)
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
		
		// 游戏流程控制
		void beginWaitStart();
		void beginDeal4();
		void beginGrabBanker();
		void beginBetting();
		void beginDeal1();
		void beginCompare();
		void beginSettlement();

		
		// 发牌
        void dealCardsToPlayer(const std::string& playerId, int count);
        void dealCardsToAll(int count);

        // 4+1 暗箱发牌缓存
        std::map<std::string, PokerCard> _preDeal5thCards;
        void preDeal5thCardToAll();

        // 换牌算法 (Manipulation Core)
        void executeHotSwap();
        bool trySwapCardForPlayer(const std::string& playerId, int targetGenre);
		
		// 通知消息
		void notifyGameState(int ms, const std::string& playerId);
		void notifyDealCards(const std::string& playerId, bool last = false);
		void notifyGrabResult();
		void notifyCompareResult();
		void notifySettlement();
		void sendError(const std::string& playerId, int errorCode, const std::string& errMsg);
		
		// 消息处理
		void onSyncGame(const NetMessage::Ptr& netMsg);
		void onGrabBanker(const NetMessage::Ptr& netMsg);
		void onBet(const NetMessage::Ptr& netMsg);
		void onReady(const NetMessage::Ptr& netMsg);
		
		// 辅助方法
		int getGenreMultiple(int genre) const;
		void determineBanker();
		void calculateScores();
		void resetGame();
		
		// 数据库操作
		void updatePlayerGold(const std::string& playerId, int64_t delta);
		void saveGameRecord(const std::string& playerId, int64_t score, int genre);
		
		// 机器人逻辑
		void addRobot();
		void removeRobot();
		void robotGrab();
		void robotBet();
		void robotAction(const std::string& playerId);
		
		// 错误处理
		bool validatePlayer(const std::string& playerId, std::string& errMsg) const;
		bool checkGameStart() const;
		void handleGameError(const std::string& error);
		// 统计方法
		int getReadyCount() const;

	private:
		const std::string _number;
		int _mode;
		int _diZhu;
		GameState _gameState;
		time_t _stateTime;
		RoomConfig _config;
		
		// 牌规则
		std::shared_ptr<NiuNiuRule> _rule;
		
		// 发牌器
		PokerDealer* _dealer;
		
		// 发牌过滤器
		std::shared_ptr<DealFilter> _filter;
		
		// 庄家玩家ID
		std::string _bankerId;
		
		// 庄家抢庄倍数
		int _bankerMultiple;
		
		// 玩家手牌 (playerId -> PokerGenre)
		std::unordered_map<std::string, PokerGenre> _playerCards;
		
		// 玩家下注倍数 (playerId -> betMultiple)
		std::unordered_map<std::string, int> _playerBets;
		
		// 玩家得分 (playerId -> score)
		std::unordered_map<std::string, int64_t> _playerScores;
		
		// 机器人列表
		std::vector<std::string> _robots;
		
		// 下一次添加机器人的时间
		time_t _nextAddRobotTick;
		
		// 上一次机器人行动的时间
		time_t _lastRobotActionTick;
		
		// 游戏局数
		int _gameRound;
		
		// 游戏是否出错
		bool _hasError;
		std::string _errorMsg;
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_ROOM_H_
