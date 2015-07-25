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
using namespace Windows::Data::Json;

GameAnalyticsInterface::GameAnalyticsInterface(const std::wstring & gameKey, const std::wstring & secretKey)
	: httpClient(ref new Windows::Web::Http::HttpClient()),
	gameKey(gameKey),
	secretKey(secretKey),
	build(this->GetAppVersion()),
	sessionId(this->GenerateSessionId()),
	userId(this->GetHardwareId())
{
}

task<JsonObject^> GameAnalyticsInterface::Init()
{
	// Build event object.
	auto jsonObject = ref new JsonObject();

	// TODO: Get correct OS version.
	jsonObject->Insert(L"platform", this->ToJsonValue(L"win8"));
	jsonObject->Insert(L"os_version", this->ToJsonValue(L"win 8.1"));
	jsonObject->Insert(L"sdk_version", this->ToJsonValue(L"win 2.0"));

	// Send event.
	return this->SendGameAnalyticsEvent(L"init", jsonObject).then([this](JsonObject^ response)
	{
		// Verify response.
		auto enabled = response->GetNamedBoolean(L"enabled");

		if (!enabled)
		{
			auto message = L"Error initializing GameAnalytics.";
			auto messageString = ref new Platform::String(message);
			throw ref new Platform::FailureException(messageString);
		}

		// Set server timestamp.
		this->initialized = true;
		this->serverTimestamp = response->GetNamedNumber(L"server_ts");

		if (!QueryPerformanceCounter(&this->initializationTime))
		{
			throw ref new Platform::FailureException("Unable to get system time.");
		}

		return response;
	});
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount) const
{
	// Build event object.
	auto jsonObject = ref new JsonObject();
	jsonObject->Insert(L"category", this->ToJsonValue(L"business"));

	jsonObject->Insert(L"event_id", this->ToJsonValue(eventId));
	jsonObject->Insert(L"currency", this->ToJsonValue(currency));
	jsonObject->Insert(L"amount", this->ToJsonValue(amount));

	if (!this->area.empty())
	{
		jsonObject->Insert(L"area", this->ToJsonValue(this->area));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId) const
{
	// Build event object.
	auto jsonObject = this->BuildDesignEventObject(eventId);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId, const float value) const
{
	// Build event object.
	auto jsonObject = this->BuildDesignEventObject(eventId);
	jsonObject->Insert(L"value", this->ToJsonValue(value));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendErrorEvent(const std::wstring & message, const Severity::Severity severity) const
{
	// Build event object.
	auto jsonObject = ref new JsonObject();
	jsonObject->Insert(L"category", this->ToJsonValue(L"error"));

	jsonObject->Insert(L"message", this->ToJsonValue(message));
	jsonObject->Insert(L"severity", this->ToJsonValue(Severity::ToWString(severity)));

	if (!this->area.empty())
	{
		jsonObject->Insert(L"area", this->ToJsonValue(this->area));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendUserEvent(const User & user) const
{
	// Build event object.
	auto jsonObject = ref new JsonObject();
	jsonObject->Insert(L"category", this->ToJsonValue(L"user"));

	if (user.gender != Gender::Unknown)
	{
		jsonObject->Insert(L"gender", this->ToJsonValue(Gender::ToWString(user.gender)));
	}

	if (user.birthYear >= 0)
	{
		jsonObject->Insert(L"birth_year", this->ToJsonValue(user.birthYear));
	}

	if (user.friendCount >= 0)
	{
		jsonObject->Insert(L"friend_count", this->ToJsonValue(user.friendCount));
	}

	if (!user.facebookId.empty())
	{
		jsonObject->Insert(L"facebook_id", this->ToJsonValue(user.facebookId));
	}

	if (!user.googlePlusId.empty())
	{
		jsonObject->Insert(L"googleplus_id", this->ToJsonValue(user.googlePlusId));
	}

	if (!user.iOSId.empty())
	{
		jsonObject->Insert(L"ios_id", this->ToJsonValue(user.iOSId));
	}

	if (!user.androidId.empty())
	{
		jsonObject->Insert(L"android_id", this->ToJsonValue(user.androidId));
	}

	if (!user.adTruthId.empty())
	{
		jsonObject->Insert(L"adtruth_id", this->ToJsonValue(user.adTruthId));
	}

	if (!user.platform.empty())
	{
		jsonObject->Insert(L"platform", this->ToJsonValue(user.platform));
	}

	if (!user.device.empty())
	{
		jsonObject->Insert(L"device", this->ToJsonValue(user.device));
	}

	if (!user.osMajor.empty())
	{
		jsonObject->Insert(L"os_major", this->ToJsonValue(user.osMajor));
	}

	if (!user.osMinor.empty())
	{
		jsonObject->Insert(L"os_minor", this->ToJsonValue(user.osMinor));
	}

	if (!user.installPublisher.empty())
	{
		jsonObject->Insert(L"install_publisher", this->ToJsonValue(user.installPublisher));
	}

	if (!user.installSite.empty())
	{
		jsonObject->Insert(L"install_site", this->ToJsonValue(user.installSite));
	}

	if (!user.installCampaign.empty())
	{
		jsonObject->Insert(L"install_campaign", this->ToJsonValue(user.installCampaign));
	}

	if (!user.installAdGroup.empty())
	{
		jsonObject->Insert(L"install_adgroup", this->ToJsonValue(user.installAdGroup));
	}

	if (!user.installAd.empty())
	{
		jsonObject->Insert(L"install_ad", this->ToJsonValue(user.installAd));
	}

	if (!user.installKeyword.empty())
	{
		jsonObject->Insert(L"install_keyword", this->ToJsonValue(user.installKeyword));
	}

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
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

JsonObject^ GameAnalyticsInterface::BuildDesignEventObject(const std::wstring & eventId) const
{
	auto jsonObject = ref new JsonObject();
	jsonObject->Insert(L"category", this->ToJsonValue(L"design"));

	jsonObject->Insert(L"event_id", this->ToJsonValue(eventId));

	if (!this->area.empty())
	{
		jsonObject->Insert(L"area", this->ToJsonValue(this->area));
	}

	return jsonObject;
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

uint64 GameAnalyticsInterface::GetTimeSinceInit() const
{
	LARGE_INTEGER currentTime;

	if (!QueryPerformanceCounter(&currentTime))
	{
		throw ref new Platform::FailureException(L"Unable to get system time.");
	}

	return (currentTime.QuadPart - this->initializationTime.QuadPart) / 10000000;
}

JsonValue^ GameAnalyticsInterface::ToJsonValue(std::wstring s) const
{
	return JsonValue::CreateStringValue(ref new String(s.c_str()));
}

JsonValue^ GameAnalyticsInterface::ToJsonValue(double d) const
{
	return JsonValue::CreateNumberValue(d);
}

task<JsonObject^> GameAnalyticsInterface::SendGameAnalyticsEvent(const std::wstring & route, JsonObject^ eventObject) const
{
	// Build event JSON.
	JsonArray^ jsonArray = ref new JsonArray();
	jsonArray->Append(eventObject);

	// Add category.
	//jsonObject->Insert(L"category", this->ToJsonValue(category));

	// Add header.
	// TODO: Get device model.
	//auto clientTimestamp = this->serverTimestamp + this->GetTimeSinceInit();

	//json.append(L"\"device\":\"unknown\",");
	//json.append(L"\"v\":\"2\",");
	//json.append(L"\"user_id\":\"" + this->userId + L"\",");
	//json.append(L"\"client_ts\":\"" + this->userId + L"\",");
	//json.append(L"\"session_id\":\"" + this->sessionId + L"\",");
	//json.append(L"\"build\":\"" + this->build + L"\"");

	// Generate MD5 of event data and secret key.
	auto jsonString = jsonArray->Stringify();
	auto secretKeyString = ref new String(this->secretKey.c_str());

	auto alg = MacAlgorithmProvider::OpenAlgorithm(MacAlgorithmNames::HmacSha256);
	auto jsonBuffer = CryptographicBuffer::ConvertStringToBinary(jsonString, BinaryStringEncoding::Utf8);
	auto secretKeyBuffer = CryptographicBuffer::ConvertStringToBinary(secretKeyString, BinaryStringEncoding::Utf8);
	auto hmacKey = alg->CreateKey(secretKeyBuffer);

	auto hashedJsonBuffer = CryptographicEngine::Sign(hmacKey, jsonBuffer);
	auto hashedJsonBase64 = CryptographicBuffer::EncodeToBase64String(hashedJsonBuffer);

	// Build category URL.
	std::wstring relativeUrl = this->gameKey + L"/" + route;
	
	// TODO: Replase by production URL http://api.gameanalytics.com/v2/
	std::wstring absoluteUrl = L"http://sandbox-api.gameanalytics.com/v2/" + relativeUrl;
	auto absoluteUrlString = ref new String(absoluteUrl.c_str());

	// Send event to GameAnalytics.
	auto message = ref new HttpRequestMessage();

	message->RequestUri = ref new Uri(absoluteUrlString);
	message->Method = HttpMethod::Post;
	message->Content = ref new HttpStringContent
		(jsonString,
		Windows::Storage::Streams::UnicodeEncoding::Utf8,
		ref new Platform::String(L"application/json"));
	message->Headers->TryAppendWithoutValidation(L"Authorization", hashedJsonBase64);

	auto response = ref new HttpResponseMessage();

	return create_task(httpClient->SendRequestAsync(message)).then([=](HttpResponseMessage^ response)
	{
		// Validate HTTP status code.
		response->EnsureSuccessStatusCode();
		return create_task(response->Content->ReadAsStringAsync());
	}).then([=](String^ responseBodyAsText){
		JsonObject^ json = JsonObject::Parse(responseBodyAsText);
		return json;

		// Verify response.
		//auto responseString = std::wstring(responseBodyAsText->Data());

		//if (responseString.find(L"\"enabled\":true") == std::string::npos)
		//{
		//	auto message = L"Error sending analytics event: " + responseString;
		//	auto messageString = ref new Platform::String(message.c_str());
		//	throw ref new Platform::FailureException(messageString);
		//}
	});
}
