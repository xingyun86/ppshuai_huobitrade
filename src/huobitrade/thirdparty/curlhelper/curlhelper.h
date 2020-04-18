#pragma once

#include <string>
#include <map>
#include <curl/curl.h>
#include <openssl/conf.h>

#include <openssl/err.h>

#include <openssl/engine.h>

#include <openssl/ssl.h>

#define CURL_GLOBAL_INIT(X) { \
curl_global_init(X); \
}

#define CURL_GLOBAL_EXIT() { \
curl_global_cleanup(); \
CONF_modules_free(); \
ERR_remove_state(0); \
ENGINE_cleanup(); \
CONF_modules_unload(1); \
ERR_free_strings(); \
EVP_cleanup(); \
CRYPTO_cleanup_all_ex_data(); \
sk_SSL_COMP_free(SSL_COMP_get_compression_methods()); \
}
typedef struct _tagByteData {
#define call_back_data_size 0xffff
	char * p;
	unsigned int s;
	unsigned int v;
	static struct _tagByteData * startup()
	{
		struct _tagByteData * thiz = (struct _tagByteData *)malloc(sizeof(struct _tagByteData));
		if (thiz)
		{
			thiz->init();
		}
		return thiz;
	}
	_tagByteData()
	{
	}
	_tagByteData(char ** _p, unsigned int _s = 0, unsigned int _v = 0)
	{
		init(_p, _s, _v);
	}
	void init(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
	{
		p = _p ? (*_p) : 0; s = _s; v = (!p || !_v) ? call_back_data_size : _v;
		if (!p)
		{
			p = (char *)malloc(v * sizeof(char));
		}
		if (p && v > 0)
		{
			memset(p, 0, v * sizeof(char));
		}
	}
	void copy(const char * _p, unsigned int _s)
	{
		if (_s > 0)
		{
			if (s + _s > v)
			{
				v += _s + 1;
				p = (char *)realloc(p, v * sizeof(char));
				memset(p + s, 0, _s + 1);
			}
			if (p)
			{
				memcpy(p, _p, _s);
				s = _s;
			}
		}
	}
	char * append(char * _p, unsigned int _s)
	{
		if (_s > 0)
		{
			if (s + _s > v)
			{
				v += _s + 1;
				p = (char *)realloc(p, v * sizeof(char));
				memset(p + s, 0, _s + 1);
			}
			if (p)
			{
				memcpy(p + s, _p, _s);
				s += _s;
			}
		}
		return p;
	}
	void clear()
	{
		if (p)
		{
			memset(p, 0, s);
			s = 0;
		}
	}
	void exit(char ** _p)
	{
		if (_p && (*_p))
		{
			free((*_p));
			(*_p) = 0;
		}
		s = v = 0;
	}
	void cleanup()
	{
		exit(&p);
		free(this);
	}
}bytedata, * pbytedata, BYTEDATA, *PBYTEDATA;

//curl 回调处理返回数据函数
__inline static size_t write_native_data_callback(void* p_data, size_t n_size, size_t n_menb, void* p_argv)
{
	pbytedata pcbd = (pbytedata)p_argv;
	if (pcbd && p_data)
	{
		pcbd->append((char*)p_data, n_size * n_menb);
	}
	return n_size * n_menb;
}

//curl 回调处理返回数据函数
__inline static size_t write_data_callback(void* p_data, size_t n_size, size_t n_menb, void* p_argv)
{
	std::string * pstr = reinterpret_cast<std::string*>(p_argv);
	if (pstr && p_data)
	{
		pstr->append((char*)p_data, n_size * n_menb);
	}
	return n_size * n_menb;
}

//////////////////////////////////////////////////////////////////////
//函数功能:传入URL获取JSON字符串
//函数参数:
//		pByteData			返回的JSON数据字符串
//		pRequestUrl			下载地址URL
//		bPostRequest		是否为POST请求
//		pHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		pPostField			发送的POST域数据
//		nProxyType			设置代理类型(CURLPROXY_HTTP, CURLPROXY_HTTPS, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5)
//		pProxyPort			设置请求使用的代理([host]:[port])
//		pProxyUserPwd		设置代理需要的用户密码(user:password)
//		bVerbose			是否为详细日志信息
//		nDelayTime			超时设置，默认为60000毫秒
//返回值:
//		CURLcode
__inline static CURLcode curl_http_data_execute(
	pbytedata pByteData, 
	const char * pRequestUrl, 
	bool bPostRequest = false, 
	const char * pHeaderData = 0, 
	const char * pPostField = 0, 
	curl_proxytype nProxyType = CURLPROXY_HTTP, 
	const char * pProxyPort = 0, 
	const char * pProxyUserPwd = 0, 
	bool bVerbose = false, 
	int nDelayTime = 60000)
{
	CURL * pCurl = 0;
	CURLcode curlCode = CURLE_OK;
	struct curl_slist *plist = 0;
	struct curl_httppost *formpost = 0;
	struct curl_httppost *lastptr = 0;

	pCurl = curl_easy_init();
	if (pCurl)
	{
		// transfer data in text / ASCII format
		//curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);

		// use Location: Luke!
		//curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);

		// We want the referrer field set automatically when following locations
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);

		// Set to explicitly forbid the upcoming transfer's connection to be re-used
		// when done. Do not use this unless you're absolutely sure of this, as it
		// makes the operation slower and is less friendly for the network. 
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 0L);

		// Instruct libcurl to not use any signal/alarm handlers, even when using
		// timeouts. This option is useful for multi-threaded applications.
		// See libcurl-the-guide for more background information. 
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
		
		// Function that will be called to store the output (instead of fwrite). The
		// parameters will use fwrite() syntax, make sure to follow them.
		//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, [](void * p_data, size_t n_size, size_t n_menb, void * p_argv){});
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_native_data_callback);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)pByteData);

		//char *output = curl_easy_escape(pCurl, strRequestURL.c_str(), strRequestURL.length());

		// The full URL to get/put
		curl_easy_setopt(pCurl, CURLOPT_URL, pRequestUrl);

		//设置头部数据
		if (pHeaderData)
		{
			// This points to a linked list of headers, struct curl_slist kind. This
			// list is also used for RTSP (in spite of its name)
			plist = curl_slist_append(plist, pHeaderData);
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		// HTTP POST method
		curl_easy_setopt(pCurl, CURLOPT_POST, bPostRequest ? 1L : 0L);

		// Now specify the POST data
		//设置POST域数据
		if (pPostField)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, pPostField);
		}
		
		if (nDelayTime > 0)
		{
			// Time-out the read operation after this amount of milliseconds
			curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);
		}

		if (pProxyPort)
		{
			// indicates type of proxy. accepted values are CURLPROXY_HTTP (default),
			// CURLPROXY_HTTPS, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A and
			// CURLPROXY_SOCKS5.
			curl_easy_setopt(pCurl, CURLOPT_PROXYTYPE, nProxyType);

			// Port of the proxy, can be set in the proxy string as well with: "[host]:[port]"
			curl_easy_setopt(pCurl, CURLOPT_PROXYPORT, pProxyPort);

			if (pProxyUserPwd)
			{
				// "user:password" to use with proxy.
				curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, pProxyUserPwd);
			}
		}

		curlCode = curl_easy_perform(pCurl);
		//if (mime)
		{
			// Free multipart message. 
			//curl_mime_free(mime);
		}
		curl_easy_cleanup(pCurl);
	}

	return curlCode;
}

//////////////////////////////////////////////////////////////////////
//函数功能:传入URL获取JSON字符串
//函数参数:
//		pByteData			返回的JSON数据字符串
//		pRequestUrl			下载地址URL
//		bPostRequest		是否为POST请求
//		pHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		pFormFieldMap		发送的FORM键值对列表(如若是文件，则值路径间隔符为'/')
//		nProxyType			设置代理类型(CURLPROXY_HTTP, CURLPROXY_HTTPS, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5)
//		pProxyPort			设置请求使用的代理([host]:[port])
//		pProxyUserPwd		设置代理需要的用户密码(user:password)
//		bVerbose			是否为详细日志信息
//		nDelayTime			超时设置，默认为60000毫秒
//返回值:
//		CURLcode
__inline static CURLcode curl_http_form_execute(
	pbytedata pByteData, 
	const char * pRequestUrl, 
	bool bPostRequest = false, 
	const char * pHeaderData = 0, 
	std::map<std::string, std::string> * pFormFieldMap = 0,
	curl_proxytype nProxyType = CURLPROXY_HTTP, 
	const char * pProxyPort = 0, 
	const char * pProxyUserPwd = 0, 
	bool bVerbose = false, 
	int nDelayTime = 60000)
{
	CURL * pCurl = 0;
	CURLcode curlCode = CURLE_OK;
	struct curl_slist *plist = 0;
	struct curl_httppost *formpost = 0;
	struct curl_httppost *lastptr = 0;

	pCurl = curl_easy_init();
	if (pCurl)
	{
		// transfer data in text / ASCII format
		//curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);

		// use Location: Luke!
		//curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);

		// We want the referrer field set automatically when following locations
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);

		// Set to explicitly forbid the upcoming transfer's connection to be re-used
		// when done. Do not use this unless you're absolutely sure of this, as it
		// makes the operation slower and is less friendly for the network. 
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 0L);

		// Instruct libcurl to not use any signal/alarm handlers, even when using
		// timeouts. This option is useful for multi-threaded applications.
		// See libcurl-the-guide for more background information. 
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
		
		// Function that will be called to store the output (instead of fwrite). The
		// parameters will use fwrite() syntax, make sure to follow them.
		//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, [](void * p_data, size_t n_size, size_t n_menb, void * p_argv){});
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_native_data_callback);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)pByteData);

		//char *output = curl_easy_escape(pCurl, strRequestURL.c_str(), strRequestURL.length());
		curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPALIVE, 1L);

		// The full URL to get/put
		curl_easy_setopt(pCurl, CURLOPT_URL, pRequestUrl);

		//设置头部数据
		if (pHeaderData)
		{
			// This points to a linked list of headers, struct curl_slist kind. This
			// list is also used for RTSP (in spite of its name)
			plist = curl_slist_append(plist, pHeaderData);
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		// HTTP POST method
		curl_easy_setopt(pCurl, CURLOPT_POST, bPostRequest ? 1L : 0L);
		
		if (pFormFieldMap)
		{
			for (auto it = pFormFieldMap->begin(); it != pFormFieldMap->end(); it++)
			{
				if (it->second.rfind('/') != std::string::npos)
				{
					// Fill in the file upload field. This makes libcurl load data from
					// the given file name when curl_easy_perform() is called. 
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->first.c_str(),
						CURLFORM_FILENAME, it->second.substr(it->second.rfind('/')).c_str(),
						CURLFORM_FILE, it->second.c_str(),
						CURLFORM_END);
				}
				else
				{
					// Fill in the filename field
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->first.c_str(),
						CURLFORM_COPYCONTENTS, it->second.c_str(),
						CURLFORM_END);
				}
			}
			if (formpost)
			{
				// This points to a linked list of post entries, struct curl_httppost
				curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);
			}
		}
		
		if (nDelayTime > 0)
		{
			// Time-out the read operation after this amount of milliseconds
			curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);
		}

		if (pProxyPort)
		{
			// indicates type of proxy. accepted values are CURLPROXY_HTTP (default),
			// CURLPROXY_HTTPS, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A and CURLPROXY_SOCKS5.
			curl_easy_setopt(pCurl, CURLOPT_PROXYTYPE, nProxyType);

			// Port of the proxy, can be set in the proxy string as well with: "[host]:[port]"
			curl_easy_setopt(pCurl, CURLOPT_PROXYPORT, pProxyPort);

			if (pProxyUserPwd)
			{
				// "user:password" to use with proxy.
				curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, pProxyUserPwd);
			}
		}

		curlCode = curl_easy_perform(pCurl);

		if (formpost)
		{
			curl_formfree(formpost);
		}

		curl_easy_cleanup(pCurl);
	}

	return curlCode;
}

__inline static CURLcode curl_http_startup()
{
	return curl_global_init(CURL_GLOBAL_DEFAULT);
}

__inline static void curl_http_cleanup()
{
	curl_global_cleanup();
	CONF_modules_free();
	ERR_remove_state(0);
	ENGINE_cleanup();
	CONF_modules_unload(1);
	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
}

//////////////////////////////////////////////////////////////////////

__inline static std::string post_form(std::string url, std::map<std::string, std::string> & sv)
{
	std::string result = "";
	const char * pheaderdata = "Expect:";
	CURLcode curlCode = CURLcode::CURLE_OK;
	pbytedata pbd = bytedata::startup();

	curlCode = curl_http_form_execute(pbd, url.c_str(), true, pheaderdata, &sv);

	if (pbd->p)
	{
		result = pbd->p;
	}
	pbd->cleanup();

	return result;
}