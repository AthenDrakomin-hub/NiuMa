// GrabNiuNiuMessages.cpp

#include "GrabNiuNiuMessages.h"
#include "Network/NetMessage.h"

namespace NiuMa
{
	const std::string MsgGrabNiuSync::TYPE = "GrabNiuSync";
	const std::string MsgGrabNiuSyncResp::TYPE = "GrabNiuSyncResp";

	void GrabNiuNiuMessages::registMessages()
	{
		NetMessage::registMessage(MsgGrabNiuSync::TYPE, []() {
			return std::make_shared<MsgGrabNiuSync>();
		});
		NetMessage::registMessage(MsgGrabNiuSyncResp::TYPE, []() {
			return std::make_shared<MsgGrabNiuSyncResp>();
		});
	}
}
