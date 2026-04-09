// GrabNiuNiuMessages.cpp

#include "GrabNiuNiuMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgGrabNiuSync::TYPE = "GrabNiuSync";
	const std::string MsgGrabNiuSyncResp::TYPE = "GrabNiuSyncResp";
	const std::string MsgGrabNiuGrab::TYPE = "GrabNiuGrab";
	const std::string MsgGrabNiuBet::TYPE = "GrabNiuBet";
	const std::string MsgGrabNiuDealCards::TYPE = "GrabNiuDealCards";
	const std::string MsgGrabNiuGrabResult::TYPE = "GrabNiuGrabResult";
	const std::string MsgGrabNiuCompare::TYPE = "GrabNiuCompare";
	const std::string MsgGrabNiuSettlement::TYPE = "GrabNiuSettlement";
	const std::string MsgGrabNiuError::TYPE = "GrabNiuError";

	void GrabNiuNiuMessages::registMessages()
	{
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuSync>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuSync::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuSyncResp>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuSyncResp::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuGrab>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuGrab::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuBet>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuBet::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuDealCards>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuDealCards::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuGrabResult>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuGrabResult::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuCompare>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuCompare::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuSettlement>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuSettlement::TYPE, creator);
		
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGrabNiuError>());
		MessageManager::getSingleton().registCreator(MsgGrabNiuError::TYPE, creator);
	}
}
