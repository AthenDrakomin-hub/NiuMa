// GrabNiuNiuLoader.cpp

#include "GrabNiuNiuLoader.h"
#include "GrabNiuNiuRoom.h"
#include "../GameDefines.h"
#include "Base/Log.h"

namespace NiuMa
{
	GrabNiuNiuLoader::GrabNiuNiuLoader()
		: VenueLoader(static_cast<int>(GameType::NiuNiu))
	{
	}

	GrabNiuNiuLoader::~GrabNiuNiuLoader() {}

	Venue::Ptr GrabNiuNiuLoader::load(const std::string& id)
	{
		// 暂且创建一个默认房间，真实情况应该从数据库读取配置
		std::shared_ptr<GrabNiuNiuRoom> room = std::make_shared<GrabNiuNiuRoom>(id, "000000", 0, 100);
		return room;
	}
}
