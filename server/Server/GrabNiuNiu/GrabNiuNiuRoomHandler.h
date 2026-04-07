// GrabNiuNiuRoomHandler.h

#ifndef _NIU_MA_GRAB_NIU_NIU_ROOM_HANDLER_H_
#define _NIU_MA_GRAB_NIU_NIU_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	class GrabNiuNiuRoomHandler : public GameRoomHandler
	{
	public:
		GrabNiuNiuRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~GrabNiuNiuRoomHandler();
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_ROOM_HANDLER_H_
