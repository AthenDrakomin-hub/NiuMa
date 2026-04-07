// GrabNiuNiuRoomHandler.cpp

#include "GrabNiuNiuRoomHandler.h"
#include "GrabNiuNiuMessages.h"
#include "../GameDefines.h"

namespace NiuMa
{
	GrabNiuNiuRoomHandler::GrabNiuNiuRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		addMessage(MsgGrabNiuSync::TYPE);
		
		addGameType(static_cast<int>(GameType::NiuNiu));
	}

	GrabNiuNiuRoomHandler::~GrabNiuNiuRoomHandler() {}
}
