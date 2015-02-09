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
			case Unknown:
				return L"Unknown";

			case Male:
				return L"M";

			case Female:
				return L"F";
			}

			// Unknown gender value.
			auto message = L"Unknown gender: " + std::to_wstring(gender);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}