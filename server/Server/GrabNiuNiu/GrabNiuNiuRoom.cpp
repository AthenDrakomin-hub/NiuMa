// GrabNiuNiuRoom.cpp

#include "GrabNiuNiuRoom.h"
#include "GrabNiuNiuAvatar.h"
#include "GrabNiuNiuMessages.h"
#include "Network/NetMessage.h"
#include "Redis/RedisPool.h"
#include "../GameDefines.h"
#include <chrono>

namespace NiuMa
{
	GrabNiuNiuRoom::GrabNiuNiuRoom(const std::string& venueId, const std::string& number, int mode, int diZhu)
		: GameRoom(venueId, static_cast<int>(GameType::NiuNiu), 6, RoomCategory::RoomCategoryA)
		, _number(number)
		, _mode(mode)
		, _diZhu(diZhu)
		, _gameState(GameState::WaitStart)
	{
		_stateTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	GrabNiuNiuRoom::~GrabNiuNiuRoom() {}

	void GrabNiuNiuRoom::initialize()
	{
		GameRoom::initialize();
	}

	void GrabNiuNiuRoom::onTimer()
	{
		GameRoom::onTimer();
		
		int elapsed = getStateElapsed();
		switch (_gameState) {
		case GameState::WaitStart:
			// wait logic
			break;
		case GameState::Deal4:
			if (elapsed > 2000) setState(GameState::GrabBanker);
			break;
		case GameState::GrabBanker:
			if (elapsed > 5000) setState(GameState::Betting);
			break;
		case GameState::Betting:
			if (elapsed > 5000) setState(GameState::Deal1);
			break;
		case GameState::Deal1:
			if (elapsed == 0) {
				// Fetch preset card from Redis for each player
				for (auto& pair : getAllAvatars()) {
					std::string redisKey = "PRESET_CARD:" + getId() + ":" + std::to_string(pair.second->getSeat());
					std::string val;
					if (RedisPool::getSingleton().get(redisKey, val)) {
						int cardId = std::stoi(val);
						// Extract preset card from dealer (simplified mock)
						PokerCard presetCard;
						if (_dealer.handOutSpecificCard(presetCard, cardId)) {
							// Successfully grabbed the preset card from the deck
							// Pair.second->cards.push_back(cardId);
						}
						RedisPool::getSingleton().del(redisKey);
					}
				}
			}
			if (elapsed > 2000) setState(GameState::Compare);
			break;
		case GameState::Compare:
			if (elapsed > 3000) setState(GameState::Settlement);
			break;
		case GameState::Settlement:
			if (elapsed > 4000) setState(GameState::WaitStart);
			break;
		}
	}

	bool GrabNiuNiuRoom::onMessage(const NetMessage::Ptr& netMsg)
	{
		if (GameRoom::onMessage(netMsg)) return true;

		if (netMsg->getType() == MsgGrabNiuSync::TYPE) {
			std::shared_ptr<MsgGrabNiuSyncResp> resp = std::make_shared<MsgGrabNiuSyncResp>();
			resp->gameState = static_cast<int>(_gameState);
			resp->elapsed = getStateElapsed();
			sendMessage(*resp, netMsg->getPlayerId());
			return true;
		}

		return false;
	}

	void GrabNiuNiuRoom::onConnect(const std::string& playerId)
	{
		GameRoom::onConnect(playerId);
	}

	GameAvatar::Ptr GrabNiuNiuRoom::createAvatar(const std::string& playerId, int seat, bool robot) const
	{
		GameAvatar::Ptr avatar = std::make_shared<GrabNiuNiuAvatar>(playerId, robot);
		avatar->setSeat(seat);
		return avatar;
	}

	bool GrabNiuNiuRoom::checkEnter(const std::string& playerId, std::string& errMsg, bool robot) const
	{
		return true;
	}

	int GrabNiuNiuRoom::checkLeave(const std::string& playerId, std::string& errMsg) const
	{
		return 0;
	}

	void GrabNiuNiuRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const
	{
	}

	void GrabNiuNiuRoom::onAvatarJoined(int seat, const std::string& playerId)
	{
		GameRoom::onAvatarJoined(seat, playerId);
	}

	void GrabNiuNiuRoom::onAvatarLeaved(int seat, const std::string& playerId)
	{
		GameRoom::onAvatarLeaved(seat, playerId);
	}

	void GrabNiuNiuRoom::setState(GameState s)
	{
		_gameState = s;
		_stateTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	int GrabNiuNiuRoom::getStateElapsed() const
	{
		time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		return static_cast<int>(now - _stateTime) * 1000;
	}
}
