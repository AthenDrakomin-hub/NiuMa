// GrabNiuNiuMessages.h

#ifndef _NIU_MA_GRAB_NIU_NIU_MESSAGES_H_
#define _NIU_MA_GRAB_NIU_NIU_MESSAGES_H_

#include "Game/GameMessages.h"

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

		MSGPACK_DEFINE_MAP(gameState, elapsed);
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_MESSAGES_H_
