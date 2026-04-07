#ifndef _GRAB_NIUNIU_MESSAGES_H_
#define _GRAB_NIUNIU_MESSAGES_H_

#include "MsgBase.h"
#include <vector>

namespace NiuMa {

	// 状态机定义
	enum class GrabNiuNiuState : int {
		WaitStart = 0,   // 等待开始
		Deal4 = 1,       // 发4张牌
		GrabBanker = 2,  // 抢庄阶段
		Betting = 3,     // 下注阶段
		Deal1 = 4,       // 发第5张牌
		Compare = 5,     // 比牌阶段
		Settlement = 6   // 结算阶段
	};

	// 协议名称定义
	const char MSG_GRAB_NIU_SYNC[] = "MsgGrabNiuSync";             // 客户端请求同步状态
	const char MSG_GRAB_NIU_SYNC_RESP[] = "MsgGrabNiuSyncResp";    // 同步状态响应
	const char MSG_GRAB_NIU_STATE[] = "MsgGrabNiuState";           // 状态流转通知
	const char MSG_GRAB_NIU_DEAL4[] = "MsgGrabNiuDeal4";           // 下发前4张牌
	const char MSG_GRAB_NIU_GRAB[] = "MsgGrabNiuGrab";             // 玩家抢庄请求
	const char MSG_GRAB_NIU_GRAB_RESP[] = "MsgGrabNiuGrabResp";    // 抢庄结果广播
	const char MSG_GRAB_NIU_BANKER[] = "MsgGrabNiuBanker";         // 广播定庄结果
	const char MSG_GRAB_NIU_BET[] = "MsgGrabNiuBet";               // 玩家下注请求
	const char MSG_GRAB_NIU_BET_RESP[] = "MsgGrabNiuBetResp";      // 玩家下注广播
	const char MSG_GRAB_NIU_DEAL1[] = "MsgGrabNiuDeal1";           // 下发最后1张牌
	const char MSG_GRAB_NIU_COMPARE[] = "MsgGrabNiuCompare";       // 广播比牌结果
	const char MSG_GRAB_NIU_SETTLEMENT[] = "MsgGrabNiuSettlement"; // 单局结算广播

	// -------------------------------------------------------------
	// 核心协议结构体
	// -------------------------------------------------------------

	// 同步请求
	class MsgGrabNiuSync : public MsgBase {
	public:
		MsgGrabNiuSync();
		virtual ~MsgGrabNiuSync();
		MSGPACK_DEFINE(msgType);
	};

	// 发送4张牌
	class MsgGrabNiuDeal4 : public MsgBase {
	public:
		MsgGrabNiuDeal4();
		virtual ~MsgGrabNiuDeal4();
		std::vector<int> cards;		// 4张牌的ID
		MSGPACK_DEFINE(msgType, cards);
	};

	// 抢庄请求
	class MsgGrabNiuGrab : public MsgBase {
	public:
		MsgGrabNiuGrab();
		virtual ~MsgGrabNiuGrab();
		std::string venueId;
		int multiple;				// 抢庄倍数 (0表示不抢, 1-4表示抢庄倍数)
		MSGPACK_DEFINE(msgType, venueId, multiple);
	};

	// 下注请求
	class MsgGrabNiuBet : public MsgBase {
	public:
		MsgGrabNiuBet();
		virtual ~MsgGrabNiuBet();
		std::string venueId;
		int multiple;				// 下注倍数
		MSGPACK_DEFINE(msgType, venueId, multiple);
	};

	// 下发最后1张牌
	class MsgGrabNiuDeal1 : public MsgBase {
	public:
		MsgGrabNiuDeal1();
		virtual ~MsgGrabNiuDeal1();
		std::vector<int> cards;		// 包含前4张和最后1张牌 (长度5)
		int genreType;				// 牌型
		MSGPACK_DEFINE(msgType, cards, genreType);
	};

	// 单个玩家比牌结果
	struct GrabNiuCompareItem {
		std::string playerId;
		std::vector<int> cards;
		int genreType;
		int multiple;				// 牌型翻倍倍数
		long long winScore;			// 输赢金币
		MSGPACK_DEFINE(playerId, cards, genreType, multiple, winScore);
	};

	// 广播比牌结果
	class MsgGrabNiuCompare : public MsgBase {
	public:
		MsgGrabNiuCompare();
		virtual ~MsgGrabNiuCompare();
		std::string bankerId;		// 庄家ID
		std::vector<GrabNiuCompareItem> compares;
		MSGPACK_DEFINE(msgType, bankerId, compares);
	};

}

#endif