#include "DisableAnalytics.h"
#include "HookManager.h"
#include "Logger.h"

gethostbyname_t DisableAnalytics::Original_gethostbyname = NULL;
getaddrinfo_t DisableAnalytics::Original_getaddrinfo = NULL;
std::list<std::string> DisableAnalytics::URL_Blacklist = {
	"api.amplitude.com",
	"api.uca.cloud.unity3d.com",
	"config.uca.cloud.unity3d.com",
	"perf-events.cloud.unity3d.com",
	"public.cloud.unity3d.com",
	"cdp.cloud.unity3d.com",
	"data-optout-service.uca.cloud.unity3d.com",
	"oculus.com",
	"oculuscdn.com",
	"facebook-hardware.com",
	"facebook.net",
	"facebook.com",
	"graph.facebook.com",
	"fbcdn.com",
	"fbsbx.com",
	"fbcdn.net",
	"fb.me",
	"fb.com",
	"crashlytics.com"
};

void DisableAnalytics::Setup()
{
	HMODULE wsock32 = GetModuleHandle("wsock32.dll");
	if (wsock32 != NULL)
	{
		Original_gethostbyname = (gethostbyname_t)GetProcAddress(wsock32, "gethostbyname");
		if (Original_gethostbyname != NULL)
			HookManager::Hook(&(LPVOID&)Original_gethostbyname, Hooked_gethostbyname);
		else
			Logger::DebugLogWarning("Failed to GetProcAddress ( gethostbyname ) for [ DisableAnalytics.cpp | DisableAnalytics::Setup ]");
	}
	else
		Logger::DebugLogWarning("Failed to GetModuleHandle ( wsock32.dll ) for [ DisableAnalytics.cpp | DisableAnalytics::Setup ]");

	HMODULE ws2_32 = GetModuleHandle("ws2_32");
	if (ws2_32 != NULL)
	{
		Original_getaddrinfo = (getaddrinfo_t)GetProcAddress(ws2_32, "getaddrinfo");
		if (Original_getaddrinfo != NULL)
			HookManager::Hook(&(LPVOID&)Original_getaddrinfo, Hooked_getaddrinfo);
		else
			Logger::DebugLogWarning("Failed to GetProcAddress ( getaddrinfo ) for [ DisableAnalytics.cpp | DisableAnalytics::Setup ]");
	}
	else
		Logger::DebugLogWarning("Failed to GetModuleHandle ( ws2_32 ) for [ DisableAnalytics.cpp | DisableAnalytics::Setup ]");
}

bool DisableAnalytics::CheckBlacklist(std::string url)
{
	bool url_found = (std::find(URL_Blacklist.begin(), URL_Blacklist.end(), url) != URL_Blacklist.end());
	if (url_found)
		Logger::DebugLog("Analytics URL Blocked: " + url);
	return url_found;
}

void* DisableAnalytics::Hooked_gethostbyname(const char* name)
{
	if (CheckBlacklist(name))
		return NULL;
	return Original_gethostbyname(name);
}

int DisableAnalytics::Hooked_getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, void* pHints, void* ppResult)
{
	if (CheckBlacklist(pNodeName))
		return WSAHOST_NOT_FOUND;
	return Original_getaddrinfo(pNodeName, pServiceName, pHints, ppResult);
}
