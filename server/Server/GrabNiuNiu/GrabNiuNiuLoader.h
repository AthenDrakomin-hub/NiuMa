// GrabNiuNiuLoader.h

#ifndef _NIU_MA_GRAB_NIU_NIU_LOADER_H_
#define _NIU_MA_GRAB_NIU_NIU_LOADER_H_

#include "Venue/VenueLoader.h"

namespace NiuMa
{
	class GrabNiuNiuLoader : public VenueLoader
	{
	public:
		GrabNiuNiuLoader();
		virtual ~GrabNiuNiuLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;
	};
}

#endif // !_NIU_MA_GRAB_NIU_NIU_LOADER_H_
