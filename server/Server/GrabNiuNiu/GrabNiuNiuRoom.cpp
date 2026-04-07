#include "GrabNiuNiuRoom.h"
#include "GrabNiuNiuMessages.h"
#include "MsgBase.h"
#include "Logger.h"
#include <algorithm>

namespace NiuMa {

	GrabNiuNiuRoom::GrabNiuNiuRoom(const std::string& id, int mode, long long dizhu, int config, long long limit, int capacity)
		: Venue(id, mode, dizhu, config, limit, capacity),
		  _gameState(GrabNiuNiuState::WaitStart),
		  _stateTimer(0),
		  _bankerMultiple(0),
		  _readyCount(0),
		  _grabbedCount(0),
		  _bettedCount(0),
		  _rule(true) // 百人牛牛规则（包括银牛、五小牛等新倍数）
	{
		_dealer.addRule(&_rule);
	}

	GrabNiuNiuRoom::~GrabNiuNiuRoom() {}

	void GrabNiuNiuRoom::onInit() {
		// 注册客户端消息处理器
		_msgHandlers.insert(std::make_pair(MSG_GRAB_NIU_SYNC, std::bind(&GrabNiuNiuRoom::onMsgGrabNiuSync, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
		_msgHandlers.insert(std::make_pair(MSG_GRAB_NIU_GRAB, std::bind(&GrabNiuNiuRoom::onMsgGrabNiuGrab, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
		_msgHandlers.insert(std::make_pair(MSG_GRAB_NIU_BET, std::bind(&GrabNiuNiuRoom::onMsgGrabNiuBet, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
		// _msgHandlers.insert(std::make_pair(MSG_PLAYER_READY, std::bind(&GrabNiuNiuRoom::onMsgPlayerReady, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
	}

	Avatar* GrabNiuNiuRoom::createAvatar(const std::string& accountId, const std::string& password, int seat) {
		return new GrabNiuNiuAvatar(accountId, password, seat);
	}

	void GrabNiuNiuRoom::onMessage(Avatar* avatar, const std::string& msgType, const char* msg, int len) {
		auto it = _msgHandlers.find(msgType);
		if (it != _msgHandlers.end()) {
			it->second(avatar, msg, len);
		}
	}

	void GrabNiuNiuRoom::onAvatarLeave(Avatar* avatar) {
		// 若游戏已开始，可能需要暂缓离开或托管
		Venue::onAvatarLeave(avatar);
	}

	void GrabNiuNiuRoom::onTick(long long elapsed) {
		Venue::onTick(elapsed);

		if (_stateTimer > 0) {
			_stateTimer -= elapsed;
			if (_stateTimer <= 0) {
				_stateTimer = 0;
				onStateTimeout();
			}
		}
	}

	void GrabNiuNiuRoom::changeState(GrabNiuNiuState state, long long timeoutMs) {
		_gameState = state;
		_stateTimer = timeoutMs;
		broadcastState();
	}

	void GrabNiuNiuRoom::broadcastState() {
		// 发送 MSG_GRAB_NIU_STATE
		// 暂省略详细打包
	}

	void GrabNiuNiuRoom::onStateTimeout() {
		switch (_gameState) {
		case GrabNiuNiuState::WaitStart:
			checkStartGame();
			break;
		case GrabNiuNiuState::Deal4:
			beginGrabBanker();
			break;
		case GrabNiuNiuState::GrabBanker:
			endGrabBanker();
			break;
		case GrabNiuNiuState::Betting:
			endBetting();
			break;
		case GrabNiuNiuState::Deal1:
			beginCompare();
			break;
		case GrabNiuNiuState::Compare:
			beginSettlement();
			break;
		case GrabNiuNiuState::Settlement:
			changeState(GrabNiuNiuState::WaitStart, 1000);
			break;
		default:
			break;
		}
	}

	void GrabNiuNiuRoom::checkStartGame() {
		int activePlayers = 0;
		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady) {
				activePlayers++;
			}
		}
		// 至少2人准备才能开局
		if (activePlayers >= 2) {
			beginDeal4();
		} else {
			// 继续等待
			_stateTimer = 2000;
		}
	}

	void GrabNiuNiuRoom::beginDeal4() {
		_gameState = GrabNiuNiuState::Deal4;
		_stateTimer = 2000; // 发牌动画时间
		_bankerId = "";
		_bankerMultiple = 0;
		_grabbedCount = 0;
		_bettedCount = 0;

		// 1. 洗牌 (去除大小王，共52张)
		_dealer.shuffle(1);

		// 2. 发牌: 给每位参与玩家发4张明牌
		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady) {
				a->reset();
				a->isReady = true; // 保持准备状态进入游戏中
				
				PokerGenre pg;
				_dealer.deal(pg, 4);
				CardArray& cards = pg.getCards();
				for (const auto& c : cards) {
					a->cards.push_back(c.getId());
				}

				// 发送 4 张牌给客户端
				MsgGrabNiuDeal4 msgOut;
				msgOut.cards = a->cards;
				a->sendMessage(msgOut);
			}
		}

		broadcastState();
	}

	void GrabNiuNiuRoom::beginGrabBanker() {
		changeState(GrabNiuNiuState::GrabBanker, 10000); // 10秒抢庄
	}

	void GrabNiuNiuRoom::endGrabBanker() {
		// 统计抢庄倍数
		std::vector<GrabNiuNiuAvatar*> candidates;
		int maxMultiple = -1;

		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady) {
				if (!a->isGrabbed) {
					a->grabMultiple = 0; // 超时未抢默认不抢
				}
				if (a->grabMultiple > maxMultiple) {
					maxMultiple = a->grabMultiple;
					candidates.clear();
					candidates.push_back(a);
				} else if (a->grabMultiple == maxMultiple) {
					candidates.push_back(a);
				}
			}
		}

		if (candidates.empty()) {
			// 异常处理：退回 WaitStart
			changeState(GrabNiuNiuState::WaitStart, 1000);
			return;
		}

		// 如果都不抢庄 (maxMultiple == 0)
		if (maxMultiple == 0) {
			// 选金币最多的
			long long maxGold = -1;
			for (auto& a : candidates) {
				if (a->getGold() > maxGold) {
					maxGold = a->getGold();
				}
			}
			// 过滤出金币最多的玩家集合
			std::vector<GrabNiuNiuAvatar*> richCandidates;
			for (auto& a : candidates) {
				if (a->getGold() == maxGold) {
					richCandidates.push_back(a);
				}
			}
			// 若还有多个则随机
			int randIdx = rand() % richCandidates.size();
			_bankerId = richCandidates[randIdx]->getAccountId();
			_bankerMultiple = 1; // 兜底当庄默认1倍
		} else {
			// 有人抢庄，最高倍数者当庄，多人最高则随机
			int randIdx = rand() % candidates.size();
			_bankerId = candidates[randIdx]->getAccountId();
			_bankerMultiple = maxMultiple;
		}

		// 广播定庄结果
		// MsgGrabNiuBanker msg; ...

		// 进入下注阶段
		beginBetting();
	}

	void GrabNiuNiuRoom::beginBetting() {
		changeState(GrabNiuNiuState::Betting, 10000); // 10秒下注
	}

	void GrabNiuNiuRoom::endBetting() {
		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady && a->getAccountId() != _bankerId && !a->isBetted) {
				a->betMultiple = 1; // 超时未下注默认1倍
			}
		}
		beginDeal1();
	}

	void GrabNiuNiuRoom::beginDeal1() {
		_gameState = GrabNiuNiuState::Deal1;
		_stateTimer = 3000; // 发最后1张牌并展示动画时间

		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady) {
				PokerGenre pg;
				_dealer.deal(pg, 1);
				const PokerCard& c = pg.getCards()[0];
				a->cards.push_back(c.getId());

				// 计算最终牌型
				PokerGenre finalPg;
				for (int id : a->cards) {
					PokerCard pc(id);
					finalPg.addCard(pc);
				}
				a->genreType = _rule.predicateCardGenre(finalPg);

				MsgGrabNiuDeal1 msgOut;
				msgOut.cards = a->cards;
				msgOut.genreType = a->genreType;
				a->sendMessage(msgOut);
			}
		}
		broadcastState();
	}

	void GrabNiuNiuRoom::beginCompare() {
		_gameState = GrabNiuNiuState::Compare;
		_stateTimer = 5000;

		GrabNiuNiuAvatar* banker = getBankerAvatar();
		if (!banker) {
			changeState(GrabNiuNiuState::WaitStart, 1000);
			return;
		}

		PokerGenre bankerPg;
		for (int id : banker->cards) bankerPg.addCard(PokerCard(id));
		banker->genreType = _rule.predicateCardGenre(bankerPg);
		int bankerMul = getGenreMultiple(banker->genreType);

		MsgGrabNiuCompare msgCompare;
		msgCompare.bankerId = banker->getAccountId();

		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady && a->getAccountId() != banker->getAccountId()) {
				PokerGenre playerPg;
				for (int id : a->cards) playerPg.addCard(PokerCard(id));
				
				int ret = _rule.compareGenre(bankerPg, playerPg);
				// ret: 1 庄家赢, 2 玩家赢
				
				int playerMul = getGenreMultiple(a->genreType);
				long long finalScore = 0;
				long long baseScore = _dizhu * a->betMultiple * _bankerMultiple;

				if (ret == 1) {
					// 庄家赢，玩家输庄家牌型倍数
					finalScore = -(baseScore * bankerMul);
				} else {
					// 玩家赢，玩家赢自己牌型倍数
					finalScore = baseScore * playerMul;
				}

				a->winScore = finalScore;
				banker->winScore += (-finalScore);

				GrabNiuCompareItem item;
				item.playerId = a->getAccountId();
				item.cards = a->cards;
				item.genreType = a->genreType;
				item.multiple = (ret == 1) ? bankerMul : playerMul;
				item.winScore = finalScore;
				msgCompare.comcompares.push_back(item);
			}
		}

		// 加上庄家自己的比牌数据广播
		GrabNiuCompareItem bItem;
		bItem.playerId = banker->getAccountId();
		bItem.cards = banker->cards;
		bItem.genreType = banker->genreType;
		bItem.multiple = bankerMul;
		bItem.winScore = banker->winScore;
		msgCompare.comcompares.push_back(bItem);

		for (auto& pair : _avatars) {
			pair.second->sendMessage(msgCompare);
		}
	}

	void GrabNiuNiuRoom::beginSettlement() {
		_gameState = GrabNiuNiuState::Settlement;
		_stateTimer = 3000;

		// 抽水 1% 并加减金币
		for (auto& pair : _avatars) {
			GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
			if (a && a->isReady) {
				long long finalScore = a->winScore;
				if (finalScore > 0) {
					double tax = finalScore * 0.01;
					finalScore -= static_cast<long long>(tax + 0.5); // 四舍五入
					
					// 返佣逻辑 (简化调用)
					// rewardAgency(a->getPlayerId(), tax * 0.5, a->winScore);
				}
				a->addGold(finalScore);
				a->isReady = false; // 结算完毕，需要重新准备
			}
		}
	}

	int GrabNiuNiuRoom::getGenreMultiple(int genre) const {
		int mul = 1;
		if (genre == static_cast<int>(NiuNiuGenre::Niu7) ||
			genre == static_cast<int>(NiuNiuGenre::Niu8) ||
			genre == static_cast<int>(NiuNiuGenre::Niu9))
			mul = 2;
		else if (genre == static_cast<int>(NiuNiuGenre::NiuNiu))
			mul = 3;
		else if (genre == static_cast<int>(NiuNiuGenre::YinNiu))
			mul = 4;
		else if (genre == static_cast<int>(NiuNiuGenre::WuHua))
			mul = 5;
		else if (genre == static_cast<int>(NiuNiuGenre::ZhaDan))
			mul = 6;
		else if (genre == static_cast<int>(NiuNiuGenre::WuXiao))
			mul = 8;
		return mul;
	}

	GrabNiuNiuAvatar* GrabNiuNiuRoom::getBankerAvatar() const {
		auto it = _avatars.find(_bankerId);
		if (it != _avatars.end()) {
			return dynamic_cast<GrabNiuNiuAvatar*>(it->second);
		}
		return nullptr;
	}

	// 客户端请求处理
	void GrabNiuNiuRoom::onMsgGrabNiuSync(Avatar* avatar, const char* msg, int len) {
		// 同步当前房间状态
	}

	void GrabNiuNiuRoom::onMsgGrabNiuGrab(Avatar* avatar, const char* msg, int len) {
		if (_gameState != GrabNiuNiuState::GrabBanker) return;
		MsgGrabNiuGrab req;
		if (!req.unpack(msg, len)) return;

		GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(avatar);
		if (a && !a->isGrabbed) {
			a->isGrabbed = true;
			a->grabMultiple = req.multiple;
			_grabbedCount++;

			int activeCount = 0;
			for (auto& pair : _avatars) {
				GrabNiuNiuAvatar* tmp = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
				if (tmp && tmp->isReady) activeCount++;
			}
			if (_grabbedCount >= activeCount) {
				endGrabBanker(); // 所有人抢庄完毕，提前结束抢庄阶段
			}
		}
	}

	void GrabNiuNiuRoom::onMsgGrabNiuBet(Avatar* avatar, const char* msg, int len) {
		if (_gameState != GrabNiuNiuState::Betting) return;
		MsgGrabNiuBet req;
		if (!req.unpack(msg, len)) return;

		GrabNiuNiuAvatar* a = dynamic_cast<GrabNiuNiuAvatar*>(avatar);
		if (a && !a->isBetted && a->getAccountId() != _bankerId) {
			a->isBetted = true;
			a->betMultiple = req.multiple;
			_bettedCount++;

			int activeCount = 0;
			for (auto& pair : _avatars) {
				GrabNiuNiuAvatar* tmp = dynamic_cast<GrabNiuNiuAvatar*>(pair.second);
				if (tmp && tmp->isReady) activeCount++;
			}
			if (_bettedCount >= activeCount - 1) { // 除庄家外的所有人都下注了
				endBetting();
			}
		}
	}

}