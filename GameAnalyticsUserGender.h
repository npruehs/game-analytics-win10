#pragma once

#include <string>

namespace GameAnalytics
{
	namespace Gender
	{
		enum Gender
		{
			Unknown,
			Male,
			Female,
		};

		inline std::wstring ToWString(const Gender gender)
		{
			switch (gender)
			{
			case Male:
				return L"male";

			case Female:
				return L"female";
			}

			// Unknown gender value.
			auto message = L"Unknown gender: " + std::to_wstring(gender);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}