#include "GrabNiuNiuMessages.h"

namespace NiuMa {

	MsgGrabNiuSync::MsgGrabNiuSync() {
		msgType = MSG_GRAB_NIU_SYNC;
	}

	MsgGrabNiuSync::~MsgGrabNiuSync() {}

	MsgGrabNiuDeal4::MsgGrabNiuDeal4() {
		msgType = MSG_GRAB_NIU_DEAL4;
	}

	MsgGrabNiuDeal4::~MsgGrabNiuDeal4() {}

	MsgGrabNiuGrab::MsgGrabNiuGrab() {
		msgType = MSG_GRAB_NIU_GRAB;
		multiple = 0;
	}

	MsgGrabNiuGrab::~MsgGrabNiuGrab() {}

	MsgGrabNiuBet::MsgGrabNiuBet() {
		msgType = MSG_GRAB_NIU_BET;
		multiple = 1;
	}

	MsgGrabNiuBet::~MsgGrabNiuBet() {}

	MsgGrabNiuDeal1::MsgGrabNiuDeal1() {
		msgType = MSG_GRAB_NIU_DEAL1;
		genreType = 0;
	}

	MsgGrabNiuDeal1::~MsgGrabNiuDeal1() {}

	MsgGrabNiuCompare::MsgGrabNiuCompare() {
		msgType = MSG_GRAB_NIU_COMPARE;
	}

	MsgGrabNiuCompare::~MsgGrabNiuCompare() {}

}