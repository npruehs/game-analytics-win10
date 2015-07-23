#include "pch.h"

#include "GameAnalyticsInterface.h"

#include <ppltasks.h>

using namespace GameAnalytics;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;


GameAnalyticsInterface::GameAnalyticsInterface(const std::wstring & gameKey, const std::wstring & secretKey)
	: httpClient(ref new Windows::Web::Http::HttpClient()),
	gameKey(gameKey),
	secretKey(secretKey),
	build(this->GetAppVersion()),
	sessionId(this->GenerateSessionId()),
	userId(this->GetHardwareId())
{
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount) const
{
	// Build parameter map.
	auto parameters = std::map<std::wstring, std::wstring>();
	parameters.insert(std::pair<std::wstring, std::wstring>(L"event_id", eventId));
	parameters.insert(std::pair<std::wstring, std::wstring>(L"currency", currency));
	parameters.insert(std::pair<std::wstring, std::wstring>(L"amount", std::to_wstring(amount)));

	if (!this->area.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"area", area));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"business", parameters);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId) const
{
	// Build parameter map.
	auto parameters = this->BuildDesignParameterMap(eventId);

	// Send event.
	this->SendGameAnalyticsEvent(L"design", parameters);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId, const float value) const
{
	// Build parameter map.
	auto parameters = this->BuildDesignParameterMap(eventId);
	parameters.insert(std::pair<std::wstring, std::wstring>(L"value", std::to_wstring(value)));

	// Send event.
	this->SendGameAnalyticsEvent(L"design", parameters);
}

void GameAnalyticsInterface::SendErrorEvent(const std::wstring & message, const Severity::Severity severity) const
{
	// Build parameter map.
	auto parameters = std::map<std::wstring, std::wstring>();
	parameters.insert(std::pair<std::wstring, std::wstring>(L"message", message));
	parameters.insert(std::pair<std::wstring, std::wstring>(L"severity", Severity::ToWString(severity)));

	if (!this->area.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"area", area));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"error", parameters);
}

void GameAnalyticsInterface::SendUserEvent(const User & user) const
{
	// Build parameter map.
	auto parameters = std::map<std::wstring, std::wstring>();

	if (user.gender != Gender::Unknown)
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"gender", Gender::ToWString(user.gender)));
	}

	if (user.birthYear >= 0)
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"birth_year", std::to_wstring(user.birthYear)));
	}

	if (user.friendCount >= 0)
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"friend_count", std::to_wstring(user.friendCount)));
	}

	if (!user.facebookId.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"facebook_id", user.facebookId));
	}

	if (!user.googlePlusId.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"googleplus_id", user.googlePlusId));
	}

	if (!user.iOSId.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"ios_id", user.iOSId));
	}

	if (!user.androidId.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"android_id", user.androidId));
	}

	if (!user.adTruthId.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"adtruth_id", user.adTruthId));
	}

	if (!user.platform.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"platform", user.platform));
	}

	if (!user.device.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"device", user.device));
	}

	if (!user.osMajor.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"os_major", user.osMajor));
	}

	if (!user.osMinor.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"os_minor", user.osMinor));
	}

	if (!user.installPublisher.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_publisher", user.installPublisher));
	}

	if (!user.installSite.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_site", user.installSite));
	}

	if (!user.installCampaign.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_campaign", user.installCampaign));
	}

	if (!user.installAdGroup.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_adgroup", user.installAdGroup));
	}

	if (!user.installAd.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_ad", user.installAd));
	}

	if (!user.installKeyword.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"install_keyword", user.installKeyword));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"user", parameters);
}

void GameAnalyticsInterface::SetArea(const std::wstring & area)
{
	this->area = area;
}

void GameAnalyticsInterface::SetBuild(const std::wstring & build)
{
	this->build = build;
}

void GameAnalyticsInterface::SetUserId(const std::wstring & userId)
{
	this->userId = userId;
}

std::map<std::wstring, std::wstring> GameAnalyticsInterface::BuildDesignParameterMap(const std::wstring & eventId) const
{
	auto parameters = std::map<std::wstring, std::wstring>();
	parameters.insert(std::pair<std::wstring, std::wstring>(L"event_id", eventId));

	if (!this->area.empty())
	{
		parameters.insert(std::pair<std::wstring, std::wstring>(L"area", area));
	}

	return parameters;
}

std::wstring GameAnalyticsInterface::GenerateSessionId() const
{
	GUID result;
	HRESULT hr = CoCreateGuid(&result);

	if (SUCCEEDED(hr))
	{
		// Generate new GUID.
		Guid guid(result);
		auto guidString = std::wstring(guid.ToString()->Data());

		// Remove curly brackets.
		auto sessionId = guidString.substr(1, guidString.length() - 2);
		return sessionId;
	}

	throw Exception::CreateException(hr);
}

std::wstring GameAnalyticsInterface::GetAppVersion() const
{
	auto thisPackage = Windows::ApplicationModel::Package::Current;
	auto version = thisPackage->Id->Version;
	return std::wstring(std::to_wstring(version.Major)
		+ L"." + std::to_wstring(version.Minor)
		+ L"." + std::to_wstring(version.Build)
		+ L"." + std::to_wstring(version.Revision));
}

std::wstring GameAnalyticsInterface::GetHardwareId() const
{
	auto packageSpecificToken = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
	auto hardwareId = packageSpecificToken->Id;
	auto hardwareIdString = CryptographicBuffer::EncodeToHexString(hardwareId);
	return std::wstring(hardwareIdString->Data());
}

void GameAnalyticsInterface::SendGameAnalyticsEvent(const std::wstring & category, const std::map<std::wstring, std::wstring> & parameters) const
{
	// Build event JSON.
	// http://support.gameanalytics.com/hc/en-us/articles/200841486-General-event-structure
	// http://support.gameanalytics.com/hc/en-us/articles/200841506-Design-event-structure

	std::wstring json = L"[";
	json.append(L"{");

	// Add header.
	json.append(L"\"user_id\":\"" + this->userId + L"\",");
	json.append(L"\"session_id\":\"" + this->sessionId + L"\",");
	json.append(L"\"build\":\"" + this->build + L"\"");

	// Add category specific parameters.
	for (auto it = parameters.begin(); it != parameters.end(); ++it)
	{
		json.append(L",\"" + it->first + L"\":\"" + it->second + L"\"");
	}

	json.append(L"}");
	json.append(L"]");

	// Generate MD5 of event data and secret key.
	auto jsonString = ref new String(json.c_str());
	auto secretKeyString = ref new String(this->secretKey.c_str());

	auto alg = MacAlgorithmProvider::OpenAlgorithm(MacAlgorithmNames::HmacSha256);
	auto jsonBuffer = CryptographicBuffer::ConvertStringToBinary(jsonString, BinaryStringEncoding::Utf8);
	auto secretKeyBuffer = CryptographicBuffer::ConvertStringToBinary(secretKeyString, BinaryStringEncoding::Utf8);
	auto hmacKey = alg->CreateKey(secretKeyBuffer);

	auto hashedJsonBuffer = CryptographicEngine::Sign(hmacKey, jsonBuffer);
	auto hashedJsonBase64 = CryptographicBuffer::EncodeToBase64String(hashedJsonBuffer);

	// Build category URL.
	std::wstring relativeUrl = this->gameKey + L"/" + category;
	
	// TODO: Replase by production URL http://api.gameanalytics.com/v2/
	std::wstring absoluteUrl = L"http://sandbox-api.gameanalytics.com/v2/" + relativeUrl;
	auto absoluteUrlString = ref new String(absoluteUrl.c_str());

	// Send event to GameAnalytics.
	auto message = ref new HttpRequestMessage();

	message->RequestUri = ref new Uri(absoluteUrlString);
	message->Method = HttpMethod::Post;
	message->Content = ref new HttpStringContent
		(ref new Platform::String(json.c_str()),
		Windows::Storage::Streams::UnicodeEncoding::Utf8,
		ref new Platform::String(L"application/json"));
	message->Headers->TryAppendWithoutValidation(L"Authorization", hashedJsonBase64);

	auto response = ref new HttpResponseMessage();

	create_task(httpClient->SendRequestAsync(message)).then([=](HttpResponseMessage^ response)
	{
		// Validate HTTP status code.
		response->EnsureSuccessStatusCode();
		return create_task(response->Content->ReadAsStringAsync());
	}).then([=](String^ responseBodyAsText){
		// Verify response.
		auto responseString = std::wstring(responseBodyAsText->Data());

		if (responseString.find(L"{\"status\":\"ok\"}") == std::string::npos)
		{
			auto message = L"Error sending analytics event: " + responseString;
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	});
}
