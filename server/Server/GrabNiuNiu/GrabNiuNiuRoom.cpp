// GrabNiuNiuRoom.cpp

#include "GrabNiuNiuRoom.h"
#include "GrabNiuNiuMessages.h"
#include "GrabNiuNiuAvatar.h"
#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "Timer/TimerManager.h"
#include "MySql/MysqlPool.h"
#include "Player/PlayerManager.h"
#include "Game/RobotManager.h"
#include "Message/MessageManager.h"

#include <sstream>
#include <algorithm>

namespace NiuMa
{
	// 抢庄牛牛发牌过滤器（可以使用王牌）
	class GrabNiuNiuFilter : public DealFilter
	{
	public:
		GrabNiuNiuFilter() {}
		virtual ~GrabNiuNiuFilter() {}

	public:
		virtual bool isOk(const PokerCard& c) const override {
			// 抢庄牛牛可以使用王牌
			return true;
		}
	};

	GrabNiuNiuRoom::GrabNiuNiuRoom(const std::string& venueId, const std::string& number, int mode, int diZhu)
		: GameRoom(venueId, static_cast<int>(GameType::NiuNiu), 5, RoomCategory::RoomCategoryA)
		, _number(number)
		, _mode(mode)
		, _diZhu(diZhu)
		, _gameState(GameState::WaitStart)
		, _stateTime(0)
		, _rule(new NiuNiuRule(false))  // 经典牛牛
		, _dealer(nullptr)
		, _bankerMultiple(1)
		, _nextAddRobotTick(0)
		, _lastRobotActionTick(0)
		, _gameRound(0)
		, _hasError(false)
	{
		_dealer = new PokerDealer(_rule);
		_filter = std::make_shared<GrabNiuNiuFilter>();
		
		// 根据底注设置配置
		if (diZhu >= 10000) {
			_config.minGold = diZhu * 10;
		}
	}

	GrabNiuNiuRoom::~GrabNiuNiuRoom() {
		if (_dealer) {
			delete _dealer;
			_dealer = nullptr;
		}
	}

	void GrabNiuNiuRoom::initialize() {
		setState(GameState::WaitStart);
	}

	void GrabNiuNiuRoom::setState(GameState s) {
		_gameState = s;
		_stateTime = BaseUtils::getCurrentMillisecond();
	}

	int GrabNiuNiuRoom::getStateElapsed() const {
		time_t now = BaseUtils::getCurrentMillisecond();
		return static_cast<int>(now - _stateTime);
	}

	void GrabNiuNiuRoom::onTimer() {
		GameRoom::onTimer();

		// 更新机器人
		// 更新机器人
		addRobot();
		removeRobot();

		int elapsed = getStateElapsed();
		
		switch (_gameState) {
		case GameState::WaitStart:
			// 等待玩家准备
			if (checkGameStart()) {
				// 所有玩家都已准备或超时
				if (elapsed >= 10000 || getReadyCount() >= getAvatarCount()) {
					beginDeal4();
				}
			}
			break;
			
		case GameState::Deal4:
			// 发4张牌后等待2秒进入抢庄
			if (elapsed >= 2000) {
				beginGrabBanker();
			}
			break;
			
		case GameState::GrabBanker:
			// 抢庄阶段，机器人抢庄
			robotGrab();
			if (elapsed >= _config.grabTime * 1000) {
				determineBanker();
				beginBetting();
			}
			break;
			
		case GameState::Betting:
			// 下注阶段，机器人下注
			robotBet();
			if (elapsed >= _config.betTime * 1000) {
				beginDeal1();
			}
			break;
			
		case GameState::Deal1:
			// 发最后1张牌后等待3秒进入比牌
			if (elapsed >= 3000) {
				beginCompare();
			}
			break;
			
		case GameState::Compare:
			// 比牌阶段
			if (elapsed >= _config.compareTime * 1000) {
				beginSettlement();
			}
			break;
			
		case GameState::Settlement:
			// 结算阶段等待5秒后重新开始
			if (elapsed >= 5000) {
				resetGame();
				beginWaitStart();
			}
			break;
		}
	}

	bool GrabNiuNiuRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (!netMsg)
			return false;
		
		// 先让基类处理
		if (GameRoom::onMessage(netMsg))
			return true;
		
		const std::string& type = netMsg->getType();
		
		if (type == MsgGrabNiuSync::TYPE) {
			onSyncGame(netMsg);
			return true;
		}
		else if (type == MsgGrabNiuGrab::TYPE) {
			onGrabBanker(netMsg);
			return true;
		}
		else if (type == MsgGrabNiuBet::TYPE) {
			onBet(netMsg);
			return true;
		}
		else if (type == "PlayerReady") {
			onReady(netMsg);
			return true;
		}
		
		return false;
	}

	void GrabNiuNiuRoom::onConnect(const std::string& playerId) {
		// 玩家连接时同步游戏状态
		notifyGameState(getStateElapsed(), playerId);
	}

	GameAvatar::Ptr GrabNiuNiuRoom::createAvatar(const std::string& playerId, int seat, bool robot) const {
		return std::make_shared<GrabNiuNiuAvatar>(playerId, robot);
	}

	bool GrabNiuNiuRoom::checkEnter(const std::string& playerId, std::string& errMsg, bool robot) const {
		// 检查房间是否已满
		if (getAvatarCount() >= _config.maxPlayers) {
			errMsg = "房间已满";
			return false;
		}
		
		// 机器人不需要检查金币
		if (robot)
			return true;
		
		// TODO: 金币检查需要从数据库查询
		// 目前暂时跳过金币检查，允许所有玩家进入
		(void)_config.minGold;  // 避免未使用警告
		
		return true;
	}

	int GrabNiuNiuRoom::checkLeave(const std::string& playerId, std::string& errMsg) const {
		// 游戏进行中不能离开
		if (_gameState != GameState::WaitStart && _gameState != GameState::Settlement) {
			errMsg = "游戏进行中，不能离开房间";
			return 1;
		}
		return 0;
	}

	void GrabNiuNiuRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {
		// 获取额外信息（抢庄倍数、下注倍数等）
	}

	void GrabNiuNiuRoom::onAvatarJoined(int seat, const std::string& playerId) {
		// 玩家加入时通知当前状态
		notifyGameState(0, playerId);
	}

	void GrabNiuNiuRoom::onAvatarLeaved(int seat, const std::string& playerId) {
		// 清理玩家数据
		_playerCards.erase(playerId);
		_playerBets.erase(playerId);
		_playerScores.erase(playerId);
		
		// 移除机器人记录
		auto it = std::find(_robots.begin(), _robots.end(), playerId);
		if (it != _robots.end()) {
			_robots.erase(it);
			RobotManager::getSingleton().free(playerId);
		}
		
		// 如果庄家离开，需要重新选庄
		if (_bankerId == playerId) {
			_bankerId.clear();
		}
	}

	// ==================== 游戏流程控制 ====================

	void GrabNiuNiuRoom::beginWaitStart() {
		setState(GameState::WaitStart);
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void GrabNiuNiuRoom::beginDeal4() {
		if (!checkGameStart()) {
			return;
		}
		
		_gameRound++;
		setState(GameState::Deal4);
		_hasError = false;
		_errorMsg.clear();
		
		// 洗牌
		_dealer->shuffle();
		
		// 初始化玩家数据
		_playerCards.clear();
		_playerBets.clear();
		_playerScores.clear();
		
		// 给每个玩家发4张明牌
		dealCardsToAll(4);
		
		// 给每个玩家预先抽取第5张暗牌（暂不发给前端）
		preDeal5thCardToAll();
		
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void GrabNiuNiuRoom::beginGrabBanker() {
		setState(GameState::GrabBanker);
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void GrabNiuNiuRoom::beginBetting() {
		setState(GameState::Betting);
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void GrabNiuNiuRoom::beginDeal1() {
                setState(GameState::Deal1);

                // ========================================================
                // Manipulation Core (Hot-swap) Algorithm
                // Before dealing the 5th card to everyone, execute hot-swap
                // ========================================================
                executeHotSwap();

                // 将预留的第5张牌发给每个玩家
                const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
			if (avatar && avatar->isReady()) {
				auto it = _preDeal5thCards.find(pair.first);
				if (it != _preDeal5thCards.end()) {
					PokerGenre& genre = _playerCards[pair.first];
					CardArray& cards = genre.getCards();
					cards.push_back(it->second);
					
					// 通知该玩家发牌
					notifyDealCards(pair.first, true);
				}
			}
		}
		
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void GrabNiuNiuRoom::beginCompare() {
		setState(GameState::Compare);
		
		// 计算所有玩家的牌型
		for (auto& pair : _playerCards) {
			pair.second.setCards(pair.second.getCards(), _rule);
		}
		
		// 计算得分
		calculateScores();
		
		notifyCompareResult();
	}

	void GrabNiuNiuRoom::beginSettlement() {
		setState(GameState::Settlement);
		
		// 保存游戏记录
		for (const auto& pair : _playerScores) {
			auto cardIt = _playerCards.find(pair.first);
			if (cardIt != _playerCards.end()) {
				saveGameRecord(pair.first, pair.second, cardIt->second.getGenre());
			}
		}
		
		notifySettlement();
	}

	void GrabNiuNiuRoom::preDeal5thCardToAll() {
                _preDeal5thCards.clear();
                const auto& avatars = getAllAvatars();
                for (const auto& pair : avatars) {
                        GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
                        if (avatar && avatar->isReady()) {
                                CardArray newCards;
                                if (_dealer->handOutCards(newCards, 1, _filter)) {
                                        _preDeal5thCards[pair.first] = newCards[0];
                                }
                        }
                }
        }

        void GrabNiuNiuRoom::executeHotSwap() {
                // 模拟查询 Redis 获取权重指令
                // 例如：假设我们配置了让庄家必赢（尽量发牛牛/大牌），某玩家必输（没牛）
                // 实际项目中应替换为真实的 Redis Query
                // 这里我们做个伪代码逻辑演示：如果某个玩家带有 "ForceNiuNiu" 标签，则尝试换牌

                // 遍历所有玩家，看看有没有需要干预的
                const auto& avatars = getAllAvatars();
                for (const auto& pair : avatars) {
                        GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
                        if (!avatar || !avatar->isReady()) continue;

                        // TODO: 从 Redis 获取真实配置，目前伪代码假设我们要让房主（或特定标记玩家）拿到牛牛
                        bool isForceNiuNiu = false; // 比如：RedisManager::get(pair.first + "_force") == "NiuNiu"
                        
                        // 示例：如果他是庄家，我们偷偷给他提牌型到牛牛（10）
                        if (pair.first == _bankerId) {
                                // 实际项目中由后台面板开关控制，这里仅做演示
                                // isForceNiuNiu = true; 
                        }

                        if (isForceNiuNiu) {
                                trySwapCardForPlayer(pair.first, 10); // 10 代表牛牛
                        }
                }
        }

        bool GrabNiuNiuRoom::trySwapCardForPlayer(const std::string& playerId, int targetGenre) {
                auto itCards = _playerCards.find(playerId);
                auto itPre5th = _preDeal5thCards.find(playerId);
                
                if (itCards == _playerCards.end() || itPre5th == _preDeal5thCards.end()) {
                        return false;
                }

                CardArray hand4 = itCards->second.getCards();
                if (hand4.size() != 4) return false;

                // 备份当前的牌堆
                CardArray& remainingDeck = _dealer->getCards();

                // 遍历牌堆寻找能凑成 targetGenre 的牌
                for (size_t i = 0; i < remainingDeck.size(); ++i) {
                        PokerCard candidate = remainingDeck[i];
                        
                        // 临时测试这种组合的牌型
                        CardArray testHand = hand4;
                        testHand.push_back(candidate);
                        
                        PokerGenre testGenre;
                        testGenre.setCards(testHand, _rule);

                        if (testGenre.getGenre() == targetGenre) {
                                // 找到了完美的替换牌！
                                // 1. 把原来的废牌放回牌堆
                                PokerCard old5th = itPre5th->second;
                                remainingDeck[i] = old5th; // 直接在牌堆中替换

                                // 2. 把好牌给玩家
                                _preDeal5thCards[playerId] = candidate;
                                
                                LogI << "[Hot-Swap] Successfully swapped card for player " << playerId 
                                     << " to reach genre: " << targetGenre;
                                return true;
                        }
                }

                LogW << "[Hot-Swap] Failed to find a matching card for player " << playerId 
                     << " to reach genre: " << targetGenre;
                return false;
        }

	void GrabNiuNiuRoom::dealCardsToPlayer(const std::string& playerId, int count) {
		GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(playerId).get());
		if (!avatar)
			return;
		
		CardArray newCards;
		if (!_dealer->handOutCards(newCards, count, _filter)) {
			ErrorS << "房间(ID:" << getId() << ")给玩家(" << playerId << ")发牌失败!";
			handleGameError("发牌失败");
			return;
		}
		
		// 将新牌添加到玩家手牌中
		PokerGenre& genre = _playerCards[playerId];
		CardArray& cards = genre.getCards();
		for (const auto& card : newCards) {
			cards.push_back(card);
		}
	}

	void GrabNiuNiuRoom::dealCardsToAll(int count) {
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			dealCardsToPlayer(pair.first, count);
			notifyDealCards(pair.first, count == 1);
		}
	}

	// ==================== 通知消息 ====================

	void GrabNiuNiuRoom::notifyGameState(int ms, const std::string& playerId) {
		MsgGrabNiuSyncResp msg;
		msg.gameState = static_cast<int>(_gameState);
		msg.elapsed = ms;
		msg.bankerId = _bankerId;
		msg.bankerMultiple = _bankerMultiple;
		
		if (playerId.empty()) {
			sendMessageToAll(msg);
		} else {
			sendMessage(msg, playerId);
		}
	}

	void GrabNiuNiuRoom::notifyDealCards(const std::string& playerId, bool last) {
		auto it = _playerCards.find(playerId);
		if (it == _playerCards.end())
			return;
		
		MsgGrabNiuDealCards msg;
		msg.cards = it->second.getCards();
		msg.last = last;
		
		sendMessage(msg, playerId);
	}

	void GrabNiuNiuRoom::notifyGrabResult() {
		MsgGrabNiuGrabResult msg;
		msg.bankerId = _bankerId;
		msg.bankerMultiple = _bankerMultiple;
		
		sendMessageToAll(msg);
	}

	void GrabNiuNiuRoom::notifyCompareResult() {
		MsgGrabNiuCompare msg;
		msg.bankerId = _bankerId;
		
		// 构建玩家牌型数据
		std::stringstream ss;
		bool first = true;
		for (const auto& pair : _playerCards) {
			if (!first) ss << ",";
			first = false;
			
			ss << pair.first << ",";
			
			// 牌数组转字符串
			const CardArray& cards = pair.second.getCards();
			std::string cardStr;
			for (size_t i = 0; i < cards.size(); ++i) {
				if (i > 0) cardStr += "-";
				cardStr += std::to_string(cards[i].getPoint()) + "_" + std::to_string(cards[i].getSuit());
			}
			ss << cardStr << ",";
			ss << pair.second.getGenre();
		}
		msg.playerCards = ss.str();
		
		sendMessageToAll(msg);
	}

	void GrabNiuNiuRoom::notifySettlement() {
		MsgGrabNiuSettlement msg;
		
		// 构建结算数据
		std::stringstream ss;
		bool first = true;
		for (const auto& pair : _playerScores) {
			if (!first) ss << ",";
			first = false;
			
			ss << pair.first << "," << pair.second;
		}
		msg.results = ss.str();
		
		sendMessageToAll(msg);
	}

	void GrabNiuNiuRoom::sendError(const std::string& playerId, int errorCode, const std::string& errMsg) {
		MsgGrabNiuError msg;
		msg.errorCode = errorCode;
		msg.errMsg = errMsg;
		sendMessage(msg, playerId);
	}

	// ==================== 消息处理 ====================

	void GrabNiuNiuRoom::onSyncGame(const NetMessage::Ptr& netMsg) {
		MsgGrabNiuSync* msg = dynamic_cast<MsgGrabNiuSync*>(netMsg->getMessage().get());
		if (!msg)
			return;
		
		notifyGameState(getStateElapsed(), msg->getPlayerId());
	}

	void GrabNiuNiuRoom::onGrabBanker(const NetMessage::Ptr& netMsg) {
		MsgGrabNiuGrab* msg = dynamic_cast<MsgGrabNiuGrab*>(netMsg->getMessage().get());
		if (!msg)
			return;
		
		const std::string& playerId = msg->getPlayerId();
		
		// 只在抢庄阶段处理
		if (_gameState != GameState::GrabBanker) {
			sendError(playerId, 1, "当前不是抢庄阶段");
			return;
		}
		
		// 检查玩家是否在房间中
		GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(playerId).get());
		if (!avatar) {
			sendError(playerId, 2, "玩家不在房间中");
			return;
		}
		
		// 设置抢庄倍数
		int multiple = msg->multiple;
		if (multiple < 1) multiple = 1;
		if (multiple > 4) multiple = 4;
		
		avatar->grabMultiple = multiple;
		DebugS << "玩家(" << playerId << ")抢庄倍数: " << multiple;
	}

	void GrabNiuNiuRoom::onBet(const NetMessage::Ptr& netMsg) {
		MsgGrabNiuBet* msg = dynamic_cast<MsgGrabNiuBet*>(netMsg->getMessage().get());
		if (!msg)
			return;
		
		const std::string& playerId = msg->getPlayerId();
		
		// 只在下注阶段处理
		if (_gameState != GameState::Betting) {
			sendError(playerId, 3, "当前不是下注阶段");
			return;
		}
		
		// 庄家不能下注
		if (playerId == _bankerId) {
			sendError(playerId, 4, "庄家不能下注");
			return;
		}
		
		// 检查玩家是否在房间中
		GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(playerId).get());
		if (!avatar) {
			sendError(playerId, 2, "玩家不在房间中");
			return;
		}
		
		// 设置下注倍数
		int multiple = msg->multiple;
		if (multiple < 1) multiple = 1;
		if (multiple > 5) multiple = 5;
		
		avatar->betMultiple = multiple;
		_playerBets[playerId] = multiple;
		DebugS << "玩家(" << playerId << ")下注倍数: " << multiple;
	}

	void GrabNiuNiuRoom::onReady(const NetMessage::Ptr& netMsg) {
		// 只在等待开始阶段处理
		if (_gameState != GameState::WaitStart) {
			return;
		}
		
		MsgPlayerSignature* msg = dynamic_cast<MsgPlayerSignature*>(netMsg->getMessage().get());
		if (!msg)
			return;
		
		const std::string& playerId = msg->getPlayerId();
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (avatar) {
			avatar->setReady(true);
		}
	}

	// ==================== 辅助方法 ====================

	int GrabNiuNiuRoom::getGenreMultiple(int genre) const {
		int mul = 1;
		if (genre == static_cast<int>(NiuNiuGenre::Niu7) ||
			genre == static_cast<int>(NiuNiuGenre::Niu8) ||
			genre == static_cast<int>(NiuNiuGenre::Niu9))
			mul = 2;
		else if (genre == static_cast<int>(NiuNiuGenre::NiuNiu))
			mul = 3;
		else if (genre == static_cast<int>(NiuNiuGenre::ShunZi))
			mul = 4;
		else if (genre == static_cast<int>(NiuNiuGenre::YinNiu))
			mul = 4;
		else if (genre == static_cast<int>(NiuNiuGenre::WuHua))
			mul = 5;
		else if (genre == static_cast<int>(NiuNiuGenre::TongHua))
			mul = 5;
		else if (genre == static_cast<int>(NiuNiuGenre::HuLu))
			mul = 5;
		else if (genre == static_cast<int>(NiuNiuGenre::ZhaDan))
			mul = 6;
		else if (genre == static_cast<int>(NiuNiuGenre::WuXiao))
			mul = 8;
		else if (genre == static_cast<int>(NiuNiuGenre::KaiXin))
			mul = 10;
		return mul;
	}

	void GrabNiuNiuRoom::determineBanker() {
		// 确定庄家
		int maxMultiple = 0;
		std::vector<std::string> candidates;
		
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
			if (!avatar)
				continue;
			
			if (avatar->grabMultiple > maxMultiple) {
				maxMultiple = avatar->grabMultiple;
				candidates.clear();
				candidates.push_back(pair.first);
			} else if (avatar->grabMultiple == maxMultiple && avatar->grabMultiple > 0) {
				candidates.push_back(pair.first);
			}
		}
		
		if (candidates.empty()) {
			// 无人抢庄，随机选择一个玩家做庄
			for (const auto& pair : avatars) {
				candidates.push_back(pair.first);
			}
		}
		
		if (!candidates.empty()) {
			int idx = BaseUtils::randInt(0, static_cast<int>(candidates.size()));
			_bankerId = candidates[idx];
			_bankerMultiple = maxMultiple > 0 ? maxMultiple : 1;
		}
		
		InfoS << "房间(" << getId() << ")第" << _gameRound << "局庄家: " << _bankerId << ", 倍数: " << _bankerMultiple;
		notifyGrabResult();
	}

	void GrabNiuNiuRoom::calculateScores() {
		if (_bankerId.empty()) {
			ErrorS << "房间(" << getId() << ")庄家为空，无法计算得分";
			return;
		}
		
		PokerGenre* bankerCards = nullptr;
		auto it = _playerCards.find(_bankerId);
		if (it != _playerCards.end()) {
			bankerCards = &it->second;
		}
		
		if (!bankerCards) {
			ErrorS << "房间(" << getId() << ")找不到庄家手牌";
			return;
		}
		
		int bankerGenre = bankerCards->getGenre();
		int64_t bankerTotalScore = 0;
		
		// 遍历所有玩家，与庄家比牌
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			if (pair.first == _bankerId)
				continue;
			
			auto cardIt = _playerCards.find(pair.first);
			if (cardIt == _playerCards.end())
				continue;
			
			PokerGenre& playerCards = cardIt->second;
			int playerGenre = playerCards.getGenre();
			
			// 比牌
			int result = _rule->compareGenre(*bankerCards, playerCards);
			
			// 获取下注倍数
			int betMultiple = 1;
			auto betIt = _playerBets.find(pair.first);
			if (betIt != _playerBets.end()) {
				betMultiple = betIt->second;
			}
			
			// 计算得分
			int64_t baseScore = _diZhu;
			int64_t score = 0;
			
			if (result == 1) {
				// 庄家赢
				int scoreMultiple = getGenreMultiple(bankerGenre) * _bankerMultiple * betMultiple;
				score = -baseScore * scoreMultiple;
			} else if (result == 2) {
				// 玩家赢
				int scoreMultiple = getGenreMultiple(playerGenre) * _bankerMultiple * betMultiple;
				score = baseScore * scoreMultiple;
			}
			
			// 1% Tax Logic for Winners
			int64_t actualScore = score;
			if (actualScore > 0) {
				// Calculate 1% tax, rounded to nearest whole number (or handling decimals if needed, here we use int64_t so we do simple integer math or double)
				double winAmount = static_cast<double>(actualScore);
				int64_t taxedScore = std::round(winAmount * 0.99);
				int64_t tax = actualScore - taxedScore;
				// TODO: Add tax to room owner's pool
				actualScore = taxedScore;
			}

			// 更新玩家金币
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
			if (avatar) {
				int64_t gold = avatar->getGold();
				int64_t newGold = gold + actualScore;
				
				// 允许负数（无底洞模式），所以移除“不能扣成负数”的限制
				avatar->setGold(newGold);
				_playerScores[pair.first] = actualScore;
				
				// 持久化到数据库
				if (!avatar->isRobot()) {
					updatePlayerGold(pair.first, actualScore);
				}
			}
			
			bankerTotalScore -= score; // 庄家扣除/赢得的是原分（庄家也要另外交自己的赢家税）
		}
		
		// 庄家的赢家税
		int64_t actualBankerScore = bankerTotalScore;
		if (actualBankerScore > 0) {
			double winAmount = static_cast<double>(actualBankerScore);
			int64_t taxedScore = std::round(winAmount * 0.99);
			int64_t tax = actualBankerScore - taxedScore;
			// TODO: Add tax to room owner's pool
			actualBankerScore = taxedScore;
		}

		// 更新庄家金币
		_playerScores[_bankerId] = actualBankerScore;
		GrabNiuNiuAvatar* bankerAvatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(_bankerId).get());
		if (bankerAvatar) {
			int64_t gold = bankerAvatar->getGold();
			int64_t newGold = gold + actualBankerScore;
			// 允许负数
			bankerAvatar->setGold(newGold);
			
			if (!bankerAvatar->isRobot()) {
				updatePlayerGold(_bankerId, actualBankerScore);
			}
		}
	}

	void GrabNiuNiuRoom::resetGame() {
		// 清理游戏数据
		_playerCards.clear();
		_playerBets.clear();
		_playerScores.clear();
		_bankerId.clear();
		_bankerMultiple = 1;
		
		// 重置所有玩家状态
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(pair.second.get());
			if (avatar) {
				avatar->reset();
				avatar->setReady(false);
			}
		}
	}

	// ==================== 数据库操作 ====================

	void GrabNiuNiuRoom::updatePlayerGold(const std::string& playerId, int64_t delta) {
		if (delta == 0)
			return;
		
		std::stringstream ss;
		if (delta > 0) {
			ss << "UPDATE `player` SET `gold` = `gold` + " << delta 
			   << " WHERE `player_id` = \"" << playerId << "\"";
		} else {
			ss << "UPDATE `player` SET `gold` = `gold` - " << (-delta) 
			   << " WHERE `player_id` = \"" << playerId << "\" AND `gold` >= " << (-delta);
		}
		
		std::string sql = ss.str();
		std::shared_ptr<MysqlCommonTask> task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
		MysqlPool::getSingleton().asyncQuery(task);
		
		DebugS << "更新玩家(" << playerId << ")金币: " << delta;
	}

	void GrabNiuNiuRoom::saveGameRecord(const std::string& playerId, int64_t score, int genre) {
		std::stringstream ss;
		ss << "INSERT INTO `game_record` (`player_id`, `venue_id`, `game_type`, `score`, `genre`, `round`, `create_time`) VALUES (\""
		   << playerId << "\", \"" << getId() << "\", " << static_cast<int>(GameType::NiuNiu) << ", "
		   << score << ", " << genre << ", " << _gameRound << ", NOW())";
		
		std::string sql = ss.str();
		std::shared_ptr<MysqlCommonTask> task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Insert);
		MysqlPool::getSingleton().asyncQuery(task);
	}

	// ==================== 机器人逻辑 ====================

	void GrabNiuNiuRoom::addRobot() {
		// 只在等待阶段或人数不足时添加机器人
		if (_gameState != GameState::WaitStart && getAvatarCount() >= _config.minPlayers)
			return;
		
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		
		if (_nextAddRobotTick == 0) {
			_nextAddRobotTick = nowTick + BaseUtils::randInt(3000, 10001);
			return;
		}
		
		if (nowTick < _nextAddRobotTick)
			return;
		
		_nextAddRobotTick = nowTick + BaseUtils::randInt(5000, 15001);
		
		// 检查是否需要添加机器人
		int humanCount = 0;
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			if (!pair.second->isRobot())
				humanCount++;
		}
		
		if (humanCount == 0 && _robots.empty())
			return;  // 没有真人玩家，不需要机器人
		
		if (_robots.size() >= 3)
			return;  // 机器人数量足够
		
		// 请求一个机器人
		std::string playerId;
		if (!RobotManager::getSingleton().request(playerId))
			return;
		
		// 机器人进入房间
		if (!joinRobot(playerId)) {
			RobotManager::getSingleton().free(playerId);
			return;
		}
		
		_robots.push_back(playerId);
		InfoS << "机器人(" << playerId << ")进入房间(" << getId() << ")";
	}

	void GrabNiuNiuRoom::removeRobot() {
		if (_robots.empty())
			return;
		
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		
		// 随机检查一个机器人是否需要离开
		if (_robots.size() > 2 || BaseUtils::randInt(0, 100) < 10) {
			size_t idx = BaseUtils::randInt(0, _robots.size());
			std::string robotId = _robots[idx];
			
			GameAvatar::Ptr avatar = getAvatar(robotId);
			if (!avatar) {
				_robots.erase(_robots.begin() + idx);
				RobotManager::getSingleton().free(robotId);
				return;
			}
			
			// 检查机器人在线时间
			time_t joinTick = avatar->getOfflineTick();  // 复用此字段记录加入时间
			if (joinTick == 0) {
				avatar->setOfflineTick(nowTick);
				return;
			}
			
			// 在线超过5分钟，有概率离开
			if (nowTick - joinTick > 300000 && BaseUtils::randInt(0, 100) < 20) {
				kickAvatar(avatar);
				_robots.erase(_robots.begin() + idx);
				RobotManager::getSingleton().free(robotId);
				InfoS << "机器人(" << robotId << ")离开房间(" << getId() << ")";
			}
		}
	}

	void GrabNiuNiuRoom::robotGrab() {
		if (_gameState != GameState::GrabBanker)
			return;
		
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int delta = static_cast<int>(nowTick - _lastRobotActionTick);
		if (_lastRobotActionTick != 0 && delta < 500)
			return;
		
		_lastRobotActionTick = nowTick;
		
		for (const auto& robotId : _robots) {
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(robotId).get());
			if (!avatar || avatar->grabMultiple >= 0)  // 已抢庄
				continue;
			
			// 随机决定是否抢庄
			if (BaseUtils::randInt(0, 100) < 60) {
				avatar->grabMultiple = BaseUtils::randInt(1, 5);  // 1-4倍
				if (avatar->grabMultiple > 4) avatar->grabMultiple = 4;
			} else {
				avatar->grabMultiple = 0;  // 不抢
			}
		}
	}

	void GrabNiuNiuRoom::robotBet() {
		if (_gameState != GameState::Betting)
			return;
		
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int delta = static_cast<int>(nowTick - _lastRobotActionTick);
		if (_lastRobotActionTick != 0 && delta < 500)
			return;
		
		_lastRobotActionTick = nowTick;
		
		for (const auto& robotId : _robots) {
			// 庄家不下注
			if (robotId == _bankerId)
				continue;
			
			GrabNiuNiuAvatar* avatar = dynamic_cast<GrabNiuNiuAvatar*>(getAvatar(robotId).get());
			if (!avatar || avatar->betMultiple > 0)  // 已下注
				continue;
			
			// 随机下注倍数，小倍数概率更高
			int bet = BaseUtils::randInt(1, 100);
			if (bet <= 40) {
				avatar->betMultiple = 1;
			} else if (bet <= 65) {
				avatar->betMultiple = 2;
			} else if (bet <= 80) {
				avatar->betMultiple = 3;
			} else if (bet <= 92) {
				avatar->betMultiple = 4;
			} else {
				avatar->betMultiple = 5;
			}
			
			_playerBets[robotId] = avatar->betMultiple;
		}
	}

	void GrabNiuNiuRoom::robotAction(const std::string& playerId) {
		// 机器人准备
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (avatar && !avatar->isReady()) {
			avatar->setReady(true);
		}
	}

	// ==================== 错误处理 ====================

	bool GrabNiuNiuRoom::validatePlayer(const std::string& playerId, std::string& errMsg) const {
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar) {
			errMsg = "玩家不在房间中";
			return false;
		}
		return true;
	}

	bool GrabNiuNiuRoom::checkGameStart() const {
		// 检查玩家数量
		if (getAvatarCount() < _config.minPlayers)
			return false;
		
		return true;
	}

	void GrabNiuNiuRoom::handleGameError(const std::string& error) {
		_hasError = true;
		_errorMsg = error;
		ErrorS << "房间(" << getId() << ")游戏错误: " << error;
		
		// 通知所有玩家
		MsgGrabNiuError msg;
		msg.errorCode = 100;
		msg.errMsg = error;
		sendMessageToAll(msg);
		
		// 重置游戏
		resetGame();
		beginWaitStart();
	}

	int GrabNiuNiuRoom::getReadyCount() const {
		int count = 0;
		const auto& avatars = getAllAvatars();
		for (const auto& pair : avatars) {
			if (pair.second->isReady() || pair.second->isRobot())
				count++;
		}
		return count;
	}
}
