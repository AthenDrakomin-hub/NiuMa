// GrabNiuNiuMessages.h

#ifndef _NIU_MA_GRAB_NIU_NIU_MESSAGES_H_
#define _NIU_MA_GRAB_NIU_NIU_MESSAGES_H_

#include "Game/GameMessages.h"
#include "PokerGenre.h"
#include <vector>

namespace NiuMa
{
	class GrabNiuNiuMessages
	{
	private:
		GrabNiuNiuMessages() {}

	public:
		virtual ~GrabNiuNiuMessages() {}

		static void registMessages();
	};

	/**
	 * 请求同步抢庄牛牛游戏数据消息
	 */
	class MsgGrabNiuSync : public MsgVenueInner {
	public:
		MsgGrabNiuSync() {}
		virtual ~MsgGrabNiuSync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步抢庄牛牛游戏数据消息
	 */
	class MsgGrabNiuSyncResp : public MsgBase {
	public:
		MsgGrabNiuSyncResp() {}
		virtual ~MsgGrabNiuSyncResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int gameState = 0;
		int elapsed = 0;
		int bankerMultiple = 0;
		std::string bankerId;

		MSGPACK_DEFINE_MAP(gameState, elapsed, bankerMultiple, bankerId);
	};

	/**
	 * 抢庄消息
	 * 客户端->服务器
	 */
	class MsgGrabNiuGrab : public MsgVenueInner {
	public:
		MsgGrabNiuGrab() : multiple(1) {}
		virtual ~MsgGrabNiuGrab() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 抢庄倍数 (1, 2, 3, 4)
		int multiple;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, multiple);
	};

	/**
	 * 下注消息
	 * 客户端->服务器
	 */
	class MsgGrabNiuBet : public MsgVenueInner {
	public:
		MsgGrabNiuBet() : multiple(1) {}
		virtual ~MsgGrabNiuBet() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 下注倍数 (1, 2, 3, 4, 5)
		int multiple;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, multiple);
	};

	/**
	 * 发牌通知消息
	 * 服务器->客户端
	 */
	class MsgGrabNiuDealCards : public MsgBase {
	public:
		MsgGrabNiuDealCards() {}
		virtual ~MsgGrabNiuDealCards() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家自己的牌
		CardArray cards;
		// 是否是最后一张牌
		bool last = false;

		MSGPACK_DEFINE_MAP(cards, last);
	};

	/**
	 * 抢庄结果通知
	 * 服务器->客户端
	 */
	class MsgGrabNiuGrabResult : public MsgBase {
	public:
		MsgGrabNiuGrabResult() {}
		virtual ~MsgGrabNiuGrabResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 庄家ID
		std::string bankerId;
		// 庄家倍数
		int bankerMultiple = 0;

		MSGPACK_DEFINE_MAP(bankerId, bankerMultiple);
	};

	/**
	 * 比牌结果通知
	 * 服务器->客户端
	 */
	class MsgGrabNiuCompare : public MsgBase {
	public:
		MsgGrabNiuCompare() {}
		virtual ~MsgGrabNiuCompare() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 所有玩家的牌型数据
		// 格式: playerId1,cards1,genre1,playerId2,cards2,genre2,...
		std::string playerCards;
		// 庄家ID
		std::string bankerId;

		MSGPACK_DEFINE_MAP(playerCards, bankerId);
	};

	/**
	 * 结算通知
	 * 服务器->客户端
	 */
	class MsgGrabNiuSettlement : public MsgBase {
	public:
		MsgGrabNiuSettlement() {}
		virtual ~MsgGrabNiuSettlement() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 结算数据
		// 格式: playerId1,score1,playerId2,score2,...
		std::string results;

		MSGPACK_DEFINE_MAP(results);
	};

	/**
	 * 错误响应
	 * 服务器->客户端
	 */
	class MsgGrabNiuError : public MsgBase {
	public:
		MsgGrabNiuError() : errorCode(0) {}
		virtual ~MsgGrabNiuError() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int errorCode = 0;
		std::string errMsg;

		MSGPACK_DEFINE_MAP(errorCode, errMsg);
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_MESSAGES_H_
