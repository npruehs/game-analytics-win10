#pragma once

#include <memory>

namespace GameAnalytics
{
	class GameAnalyticsInterface
	{
	public:
		// Initializes a new instance of the GameAnalytics interface
		// for the game with the specified game key and secret key.
		// Uses the app package version as build id,
		// uses the Application Specific Hardware Identifier (ASHWID) as user id,
		// and generates a new GUID for the session.
		GameAnalyticsInterface(std::wstring gameKey, std::wstring secretKey);

		// Sends the design event with the specified id to the GameAnalytics backend.
		// Can be sub-categorized by using ":" notation.
		// For example, an event_id could be: "PickedUpAmmo:Shotgun" (for design),
		// "Purchase:RocketLauncher" (for business),
		// or "Exception:NullReference" (for quality).
		void SendGameAnalyticsEvent(std::wstring eventId);

		// Sends the event with the specified id and category to the GameAnalytics backend.
		// Can be sub-categorized by using ":" notation.
		// For example, an event_id could be: "PickedUpAmmo:Shotgun" (for design),
		// "Purchase:RocketLauncher" (for business),
		// or "Exception:NullReference" (for quality).
		void SendGameAnalyticsEvent(std::wstring eventId, std::wstring category);

		// Sets the current version of the game being played. Defaults to the app package version.
		void SetBuild(std::wstring build);

		// Sets the unique ID representing the user playing the game.
		// This ID should remain the same across different play sessions.
		// Defaults to the ASHWID.
		void SetUserId(std::wstring userId);
		
	private:
		Windows::Web::Http::HttpClient^ httpClient;

		std::wstring gameKey;
		std::wstring secretKey;

		std::wstring build;
		std::wstring sessionId;
		std::wstring userId;
		
		// Gets the app package version.
		std::wstring GetAppVersion();

		// Gets the Application Specific Hardware Identifier (ASHWID).
		// See https://msdn.microsoft.com/en-us/library/windows/apps/jj553431
		std::wstring GetHardwareId();
		
		// Generates a new GUID for the current session.
		std::wstring GenerateSessionId();
	};
}