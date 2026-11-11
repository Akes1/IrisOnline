#pragma  once

//#define assert(exp) {if(!(exp)) __asm{int 0x03}}

//#include "BasicString.h"
#include "curl.h"
#include <tchar.h>

/* Default values used in twitcurl */
//class cString;

namespace twitCurlDefaults
{
	/* Constants */
	const int TWITCURL_DEFAULT_BUFFSIZE = 1024;
	static cString TWITCURL_COLON = ":";
	static cString OAUTH_AMPERSAND  = "&";
	const char TWITCURL_EOS = '\0';

	/* Miscellaneous data used to build twitter URLs*/
	static cString TWITCURL_STATUSSTRING = "status=";
	static cString TWITCURL_TEXTSTRING = "text=";
	static cString TWITCURL_QUERYSTRING = "query=";  
	static cString TWITCURL_SEARCHQUERYSTRING = "?q=";      
	static cString TWITCURL_SCREENNAME = "?screen_name=";
	static cString TWITCURL_USERID = "?user_id=";
	static cString TWITCURL_EXTENSIONFORMAT = ".xml";
	static cString TWITCURL_TARGETSCREENNAME = "?target_screen_name=";
	static cString TWITCURL_TARGETUSERID = "?target_id=";
	static cString TWITCURL_LISTCOUNT = "?count=";

};

/* Default twitter URLs */
namespace twitterDefaults
{

	/* Search URLs */
	static cString TWITCURL_SEARCH_URL = "http://search.twitter.com/search.atom";

	/* Status URLs */
	static cString TWITCURL_STATUSUPDATE_URL = "http://twitter.com/statuses/update.xml";
	static cString TWITCURL_STATUSSHOW_URL = "http://twitter.com/statuses/show/";
	static cString TWITCURL_STATUSDESTROY_URL = "http://twitter.com/statuses/destroy/";
	static cString TWITCURL_STATUSRETWEET_URL = "http://api.twitter.com/1/statuses/retweet/";

	/* Timeline URLs */
	static cString TWITCURL_PUBLIC_TIMELINE_URL		= "http://twitter.com/statuses/public_timeline.xml";
	static cString TWITCURL_FEATURED_USERS_URL		= "http://twitter.com/statuses/featured.xml";
	static cString TWITCURL_FRIENDS_TIMELINE_URL	= "http://twitter.com/statuses/friends_timeline.xml";
	static cString TWITCURL_MENTIONS_URL			= "http://twitter.com/statuses/mentions.xml";
	static cString TWITCURL_USERTIMELINE_URL		= "http://twitter.com/statuses/user_timeline.xml";
	static cString TWITCURL_HOME_TIMELINE_URL		= "http://api.twitter.com/1/statuses/home_timeline.xml";

	/* Users URLs */
	static cString TWITCURL_SHOWUSERS_URL = "http://twitter.com/users/show.xml";
	static cString TWITCURL_SHOWFRIENDS_URL = "http://twitter.com/statuses/friends.xml";
	static cString TWITCURL_SHOWFOLLOWERS_URL = "http://twitter.com/statuses/followers.xml";
	/* Direct messages URLs */
	static cString TWITCURL_DIRECTMESSAGES_URL = "http://twitter.com/direct_messages.xml";
	static cString TWITCURL_DIRECTMESSAGENEW_URL = "http://twitter.com/direct_messages/new.xml";
	static cString TWITCURL_DIRECTMESSAGESSENT_URL = "http://twitter.com/direct_messages/sent.xml";
	static cString TWITCURL_DIRECTMESSAGEDESTROY_URL = "http://twitter.com/direct_messages/destroy/";

	/* Friendships URLs */
	static cString TWITCURL_FRIENDSHIPSCREATE_URL = "http://twitter.com/friendships/create.xml";
	static cString TWITCURL_FRIENDSHIPSDESTROY_URL = "http://twitter.com/friendships/destroy.xml";
	static cString TWITCURL_FRIENDSHIPSSHOW_URL = "http://twitter.com/friendships/show.xml";
	/* Social graphs URLs */
	static cString TWITCURL_FRIENDSIDS_URL = "http://twitter.com/friends/ids.xml";
	static cString TWITCURL_FOLLOWERSIDS_URL = "http://twitter.com/followers/ids.xml";

	/* Account URLs */
	static cString TWITCURL_ACCOUNTRATELIMIT_URL = "http://twitter.com/account/rate_limit_status.xml";
	static cString TWITCURL_ACCOUNTCREDENTIALS_URL = "http://twitter.com/account/verify_credentials.xml";

	/* Favorites URLs */
	static cString TWITCURL_FAVORITESGET_URL = "http://twitter.com/favorites.xml";
	static cString TWITCURL_FAVORITECREATE_URL = "http://twitter.com/favorites/create/";
	static cString TWITCURL_FAVORITEDESTROY_URL = "http://twitter.com/favorites/destroy/";

	/* Block URLs */
	static cString TWITCURL_BLOCKSCREATE_URL = "http://twitter.com/blocks/create/";
	static cString TWITCURL_BLOCKSDESTROY_URL = "http://twitter.com/blocks/destroy/";

	/* Saved Search URLs */
	static cString TWITCURL_SAVEDSEARCHGET_URL = "http://twitter.com/saved_searches.xml";
	static cString TWITCURL_SAVEDSEARCHSHOW_URL = "http://twitter.com/saved_searches/show/";
	static cString TWITCURL_SAVEDSEARCHCREATE_URL = "http://twitter.com/saved_searches/create.xml";
	static cString TWITCURL_SAVEDSEARCHDESTROY_URL = "http://twitter.com/saved_searches/destroy/";

	/* Notification URLs */
	static cString TWITCURL_FOLLOW_URL = "http://twitter.com/notifications/follow/";
};

/* twitCurl class */
class twitCurl
{
public:
	twitCurl();
	~twitCurl();

	/* Twitter login APIs, set once and forget */
	cString& getTwitterUsername();
	cString& getTwitterPassword();
	void setTwitterUsername( cString& userName );
	void setTwitterPassword( cString& passWord );

	/* Twitter search APIs */
	bool search( cString& query );

	/* Twitter status APIs */
	bool statusUpdate( LPCSTR newStatus  );
	bool statusShowById( cString& statusId );
	bool statusDestroyById( cString& statusId );
	bool statusRetweet( cString& statusId );

	/* Twitter timeline APIs */
	bool timelinePublicGet();
	bool timelineFriendsGet( unsigned char nCnt = 0);
	bool timelineHomeGet( unsigned char nCnt = 0);
		bool timelineUserGet( cString& userInfo,bool isUserId = false, unsigned char nCheckCnt = 10 );
		bool featuredUsersGet();
		bool mentionsGet();

		/* Twitter user APIs */
		bool userGet( cString& userInfo, bool isUserId = false );
		bool friendsGet( cString userInfo = "", bool isUserId = false );
		bool followersGet( cString userInfo = "", bool isUserId = false );

		/* Twitter direct message APIs */
		bool directMessageGet();
		bool directMessageSend( cString& userInfo, cString& dMsg, bool isUserId = false );
		bool directMessageGetSent();
		bool directMessageDestroyById( cString& dMsgId );

		/* Twitter friendships APIs */
		bool friendshipCreate( cString& userInfo, bool isUserId = false );
		bool friendshipDestroy( cString& userInfo, bool isUserId = false );
		bool friendshipShow( cString& userInfo, bool isUserId = false );

		/* Twitter social graphs APIs */
		bool friendsIdsGet( cString& userInfo, bool isUserId = false );
		bool followersIdsGet( cString& userInfo, bool isUserId = false );

		/* Twitter account APIs */
		bool accountRateLimitGet();
		bool accountVerifyCredentialsGet();

		/* Twitter favorites APIs */
		bool favoriteGet();
		bool favoriteCreate( cString& statusId );
		bool favoriteDestroy( cString& statusId );

		/* Twitter block APIs */
		bool blockCreate( cString& userInfo );
		bool blockDestroy( cString& userInfo );

		/* Twitter search APIs */
		bool savedSearchGet();
		bool savedSearchCreate( cString& query );
		bool savedSearchShow( cString& searchId );
		bool savedSearchDestroy( cString& searchId );

		/* Notification APIs */
		bool follow( cString& statusId );

		/* cURL APIs */
		bool isCurlInit();
		void getLastWebResponse( cString& outWebResp );
		void getLastCurlError( cString& outErrResp );

		/* Internal cURL related methods */
		int saveLastWebResponse( char*& data, size_t size );
		static int curlCallback( char* data, size_t size, size_t nmemb, twitCurl* pTwitCurlObj );

		bool OAuthRequestGet( cString& url );
		bool OAuthRequestPost( cString& url, cString& param );
		bool Redirect( cString& url );

		/* cURL proxy APIs */
		cString& getProxyServerIp();
		cString& getProxyServerPort();
		cString& getProxyUserName();
		cString& getProxyPassword();
		void setProxyServerIp( cString& proxyServerIp );
		void setProxyServerPort( cString& proxyServerPort );
		void setProxyUserName( cString& proxyUserName );
		void setProxyPassword( cString& proxyPassword );

	private:
		/* cURL data */
		CURL* m_curlHandle;
		char m_errorBuffer[twitCurlDefaults::TWITCURL_DEFAULT_BUFFSIZE];
		cString m_callbackData;

		/* cURL flags */
		bool m_curlProxyParamsSet;
		bool m_curlLoginParamsSet;
		bool m_curlCallbackParamsSet;

		/* cURL proxy data */
		cString m_proxyServerIp;
		cString m_proxyServerPort;
		cString m_proxyUserName;
		cString m_proxyPassword;

		/* Twitter data */
		cString m_twitterUsername;
		cString m_twitterPassword;

		/* Private methods */
		void clearCurlCallbackBuffers();
		void prepareCurlProxy();
		void prepareCurlCallback();
		void prepareCurlUserPass();
		void prepareStandardParams();
		bool performGet( const cString& getUrl );
		bool performDelete( const cString& deleteUrl );
		bool performPost( const cString& postUrl, const cString dataStr = "" );
	};


	/* Private functions */
	static void utilMakeCurlParams( cString& outStr, cString& inParam1, cString& inParam2 );
	static void utilMakeUrlForUser( cString& outUrl, cString& baseUrl, cString& userInfo, bool isUserId );

