#pragma once

#include <string>

#include "GameAnalyticsUserGender.h"

namespace GameAnalytics
{
	struct User
	{
		User::User()
			: birthYear(-1),
			gender(Gender::Unknown)
			
		{
		}

		int birthYear;
		std::wstring facebookId;
		Gender::Gender gender;
		std::wstring googlePlusId;
	};
}