#pragma once

#include <string>

#include "GameAnalyticsUserGender.h"

namespace GameAnalytics
{
	struct User
	{
		User::User()
			: gender(Gender::Unknown),
			birthYear(-1),
			friendCount(-1)
		{
		}

		Gender::Gender gender;
		int birthYear;
		int friendCount;
		std::wstring facebookId;
		std::wstring googlePlusId;
		std::wstring iOSId;
		std::wstring androidId;
		std::wstring adTruthId;
		std::wstring platform;
		std::wstring device;
		std::wstring osMajor;
		std::wstring osMinor;
		std::wstring installPublisher;
		std::wstring installSite;
		std::wstring installCampaign;
		std::wstring installAdGroup;
		std::wstring installAd;
		std::wstring installKeyword;
	};
}