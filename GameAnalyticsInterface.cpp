#include "pch.h"

#include <ppltasks.h>
#include <string>

#include "GameAnalyticsInterface.h"

using namespace GameAnalytics;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;


GameAnalyticsInterface::GameAnalyticsInterface(std::wstring gameKey, std::wstring secretKey)
	: httpClient(ref new Windows::Web::Http::HttpClient()),
	gameKey(gameKey),
	secretKey(secretKey),
	build(this->GetAppVersion()),
	sessionId(this->GenerateSessionId()),
	userId(this->GetHardwareId())
{
}

void GameAnalyticsInterface::SendDesignEvent(std::wstring eventId)
{
	// Build parameter map.
	auto parameters = std::map<std::wstring, std::wstring>();
	parameters.insert(std::pair<std::wstring, std::wstring>(L"eventId", eventId));

	// Send event.
	this->SendGameAnalyticsEvent(L"design", parameters);
}

void GameAnalyticsInterface::SendGameAnalyticsEvent(std::wstring category, std::map<std::wstring, std::wstring> parameters)
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
	auto jsonAndSecretKey = json + this->secretKey;
	auto jsonAndSecretKeyString = ref new String(jsonAndSecretKey.c_str());

	auto alg = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Md5);
	auto buff = CryptographicBuffer::ConvertStringToBinary(jsonAndSecretKeyString, BinaryStringEncoding::Utf8);
	auto hashed = alg->HashData(buff);
	auto digest = CryptographicBuffer::EncodeToHexString(hashed);

	// Build category URL.
	std::wstring relativeUrl = this->gameKey + L"/" + category;
	std::wstring absoluteUrl = L"http://api.gameanalytics.com/1/" + relativeUrl;
	auto absoluteUrlString = ref new String(absoluteUrl.c_str());

	// Send event to GameAnalytics.
	auto message = ref new HttpRequestMessage();

	message->RequestUri = ref new Uri(absoluteUrlString);
	message->Method = HttpMethod::Post;
	message->Content = ref new HttpStringContent
		(ref new Platform::String(json.c_str()),
		Windows::Storage::Streams::UnicodeEncoding::Utf8,
		ref new Platform::String(L"application/json"));
	message->Headers->TryAppendWithoutValidation(L"Authorization", digest);

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

void GameAnalyticsInterface::SetBuild(std::wstring build)
{
	this->build = build;
}

void GameAnalyticsInterface::SetUserId(std::wstring userId)
{
	this->userId = userId;
}

std::wstring GameAnalyticsInterface::GetAppVersion()
{
	auto thisPackage = Windows::ApplicationModel::Package::Current;
	auto version = thisPackage->Id->Version;
	return std::wstring(std::to_wstring(version.Major)
		+ L"." + std::to_wstring(version.Minor)
		+ L"." + std::to_wstring(version.Build)
		+ L"." + std::to_wstring(version.Revision));
}

std::wstring GameAnalyticsInterface::GetHardwareId()
{
	auto packageSpecificToken = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
	auto hardwareId = packageSpecificToken->Id;
	auto hardwareIdString = CryptographicBuffer::EncodeToHexString(hardwareId);
	return std::wstring(hardwareIdString->Data());
}

std::wstring GameAnalyticsInterface::GenerateSessionId()
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
