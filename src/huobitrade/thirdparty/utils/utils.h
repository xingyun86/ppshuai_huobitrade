#pragma once
#include <map>
#include <vector>
#include <string>
#include <thread>

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRING		std::string
#define	TO_TSTRING	std::to_string
#else
#define TSTRING		std::wstring
#define	TO_TSTRING	std::to_wstring
#endif

#define MAX_ITEM_TEXT_NUM	MAXBYTE

class CItemData {
public:
	int iImage;
	std::vector<TSTRING> vTexts;
	CItemData(int iImage, std::vector<TSTRING> vTexts)
	{
		this->iImage = iImage;
		vTexts.assign(vTexts.begin(), vTexts.end());
	}
};


///////////////////////////////////////////////////////////////////////////////
#ifndef CURL_STATICLIB
//#define CURL_STATICLIB
#if defined(WIN32) || defined(_WIN32)
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "wsock32.lib")
#pragma comment (lib, "wldap32.lib")
#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "normaliz.lib")
//#pragma comment (lib, "libeay32.lib")
//#pragma comment (lib, "ssleay32.lib")
//#pragma comment (lib, "zlibstatic.lib")
//#pragma comment (lib, "libssh2.lib")
//#pragma comment (lib, "libcares.lib")
#pragma comment (lib, "libcurl.lib")
#endif
#endif
#include <macros.h>
#include <curlhelper.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/multi.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/ssl.h>

#define CURL_GLOBAL_INIT(X)	{ \
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

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
libcurls old enough to not have this symbol. It is _not_ defined to zero in
a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif

#define MAX_DOWNLOAD_CONNECTS	4

#define TASKMAP_CLEANUP(T, M)	{ \
	CURLM * pcurlm = (CURLM *)M; \
	std::map<std::string, CDebugTraceBlock> * ptaskmap = (std::map<std::string, CDebugTraceBlock> *)T; \
	for (auto it = ptaskmap->begin(); it != ptaskmap->end(); it++) \
	{ \
		if (it->second.pcurl != 0) \
		{ \
			if(pcurlm) \
			{ \
				curl_multi_remove_handle(pcurlm, it->second.pcurl); \
			} \
			curl_easy_cleanup(it->second.pcurl); \
			it->second.pcurl = 0; \
		} \
		if (it->second.pfile != 0) \
		{ \
			fclose(it->second.pfile); \
			it->second.pfile = 0; \
		} \
	} \
}

typedef enum TASKSTATETYPE {
	TSTYPE_NULL = 0,//0-����δ����
	TSTYPE_WAITING = 1,//1-����ȴ�������
	TSTYPE_CANCEL = 2,//2-������ȡ��
	TSTYPE_PROCESSING = 3,//3-�������ڴ�����
	TSTYPE_FINISH = 4,//4-���������
	TSTYPE_FAILED = 5,//5-������ʧ��
};
class CDebugTraceBlock
{
public:
	CDebugTraceBlock(const char* url, const char* fname, const char * method = "GET") {
		this->index = 0;
		this->pcurl = 0;
		this->pfile = 0;
		this->url = url;
		this->fname = fname;
		this->state = TSTYPE_NULL;
		this->request_method = method;
	}
	~CDebugTraceBlock() {}
public:
	int index;
	CURL* pcurl;
	FILE* pfile;
	std::string resp_data;
	std::string request_method;
	std::string postfields;
	std::string headerslist;
	std::string url;
	std::string fname;
	TASKSTATETYPE state;
};

typedef void(*PFUN_NotifyUpdate)(void* thiz);
typedef int(*PFUN_IsProcessing)(void* thiz);
typedef std::map<std::string, CDebugTraceBlock>* (*PFUN_GetTaskMap)(void* thiz);

__inline static
void dump(const char* text, int index, unsigned char* ptr, size_t size, char nohex)
{
	size_t i = 0;
	size_t c = 0;

	unsigned int width = 0x10;

	if (nohex)
		/* without the hex output, we can fit more on screen */
		width = 0x40;

	fprintf(stdout, "%d %s, %lu bytes (0x%lx)\n",
		index, text, (unsigned long)size, (unsigned long)size);

	for (i = 0; i < size; i += width) {

		fprintf(stdout, "%4.4lx: ", (unsigned long)i);

		if (!nohex) {
			/* hex not disabled, show it */
			for (c = 0; c < width; c++)
				if (i + c < size)
					fprintf(stdout, "%02x ", ptr[i + c]);
				else
					fprintf(stdout, "   ");
		}

		for (c = 0; (c < width) && (i + c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
				ptr[i + c + 1] == 0x0A) {
				i += (c + 2 - width);
				break;
			}
			fprintf(stdout, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
				ptr[i + c + 2] == 0x0A) {
				i += (c + 3 - width);
				break;
			}
		}
		fprintf(stdout, "\n"); /* newline */
	}
}

__inline static
int debug_trace(CURL* handle, curl_infotype type, char* data, size_t size, void* userp)
{
	const char* text = 0;
	CDebugTraceBlock* pdtb = (CDebugTraceBlock*)userp;
	int index = pdtb->index;
	(void)handle; /* prevent compiler warning */

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== %d Info: %s", index, data);
		/* FALLTHROUGH */
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, index, (unsigned char*)data, size, 1);
	return 0;
}

__inline static
size_t cb(char* d, size_t n, size_t l, void* p)
{
	/* take care of the data here, ignored in this example */
	(void)d;
	(void)p;
	return n * l;
}

__inline static
void init_task(CURLM* pCurlM, CDebugTraceBlock* pDTB)
{
	pDTB->pcurl = curl_easy_init();
	pDTB->pfile = fopen(pDTB->fname.c_str(), "wb");
	curl_slist* slist = nullptr;
	if (pDTB->headerslist.size() > 0)
	{
		curl_slist_append(slist, pDTB->headerslist.data());
	}

	// We want the referrer field set automatically when following locations
	curl_easy_setopt(pDTB->pcurl, CURLOPT_AUTOREFERER, 1L);

	// Set to explicitly forbid the upcoming transfer's connection to be re-used
	// when done. Do not use this unless you're absolutely sure of this, as it
	// makes the operation slower and is less friendly for the network. 
	curl_easy_setopt(pDTB->pcurl, CURLOPT_FORBID_REUSE, 0L);

	// Instruct libcurl to not use any signal/alarm handlers, even when using
	// timeouts. This option is useful for multi-threaded applications.
	// See libcurl-the-guide for more background information. 
	curl_easy_setopt(pDTB->pcurl, CURLOPT_NOSIGNAL, 1L);
	//ָ������ʽ
	curl_easy_setopt(pDTB->pcurl, CURLOPT_CUSTOMREQUEST, pDTB->request_method.data());
	curl_easy_setopt(pDTB->pcurl, CURLOPT_POSTFIELDS, pDTB->postfields.data());
	curl_easy_setopt(pDTB->pcurl, CURLOPT_POSTFIELDSIZE, pDTB->postfields.size());
	curl_easy_setopt(pDTB->pcurl, CURLOPT_HTTPHEADER, slist);//set head
	curl_easy_setopt(pDTB->pcurl, CURLOPT_VERBOSE, 1L);
	//�������ַ
	curl_easy_setopt(pDTB->pcurl, CURLOPT_URL, pDTB->url.data());
	//�ѵ��õ���ַ����curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pDTB);��ѯ����
	curl_easy_setopt(pDTB->pcurl, CURLOPT_PRIVATE, pDTB);
	//���õ���cb����ʱ����cb�����ĵ��ĸ�����
	//curl_easy_setopt(pDTB->pcurl, CURLOPT_WRITEDATA, pDTB->pfile);
	//curl_easy_setopt(pDTB->pcurl, CURLOPT_WRITEFUNCTION, write_native_data_callback);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_WRITEFUNCTION, write_data_callback);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_WRITEDATA, (void*)&pDTB->resp_data);

#if defined(_DEBUG) || defined(DEBUG)
	// please be verbose
	curl_easy_setopt(pDTB->pcurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_DEBUGFUNCTION, debug_trace);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_DEBUGDATA, pDTB);
#else
	/* please be verbose */
	curl_easy_setopt(pDTB->pcurl, CURLOPT_VERBOSE, 0L);
#endif

	/* HTTP/1.1 please */
	curl_easy_setopt(pDTB->pcurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	/* we use a self-signed test server, skip verification during debugging */
	curl_easy_setopt(pDTB->pcurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(pDTB->pcurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);

#if (CURLPIPE_MULTIPLEX > 0)
	/* wait for pipe connection to confirm */
	curl_easy_setopt(pDTB->pcurl, CURLOPT_PIPEWAIT, 1L);
#endif
	//����useragent
	curl_easy_setopt(pDTB->pcurl, CURLOPT_USERAGENT, "PPSAHUIDOWNLOADTOOL/99.99.99");

	curl_multi_add_handle(pCurlM, pDTB->pcurl);
}

__inline static
int async_start(std::string & resp_data, PFUN_GetTaskMap pfnGetTaskMap, PFUN_NotifyUpdate pfnNotifyUpdate, PFUN_IsProcessing pfnIsProcessing, void* thiz)
{
	CURLM* pCurlM = 0;
	CURLMsg* msg = 0;
	long L = (0);
	int U = (0);
	int M, Q = -1;
	fd_set R, W, E = { 0 };
	struct timeval T = { 0 };
	int result = EXIT_FAILURE;
	std::map<std::string, CDebugTraceBlock>* pTaskMap = pfnGetTaskMap(thiz);
	if (pTaskMap)
	{
		pCurlM = curl_multi_init();

		/* we can optionally limit the total amount of connections this multi handle uses */
		curl_multi_setopt(pCurlM, CURLMOPT_MAXCONNECTS, (long)MAX_DOWNLOAD_CONNECTS);

		curl_multi_setopt(pCurlM, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

		for (auto it = pTaskMap->begin(); it != pTaskMap->end(); it++)
		{
			if (!pfnIsProcessing(thiz))
			{
				TASKMAP_CLEANUP(pTaskMap, 0);
				goto __LEAVE_CLEAN__;
			}
			if (it->second.state == TASKSTATETYPE::TSTYPE_NULL)
			{
				it->second.state = TASKSTATETYPE::TSTYPE_WAITING;
			}
			// ���δ��ʼ, ��������ض���
			if (it->second.state == TASKSTATETYPE::TSTYPE_WAITING)
			{
				if (U < MAX_DOWNLOAD_CONNECTS)
				{
					it->second.state = TASKSTATETYPE::TSTYPE_PROCESSING;
					init_task(pCurlM, &it->second);
					U++; 
				}
			}
		}

		while (U > 0) {

			if (!pfnIsProcessing(thiz))
			{
				TASKMAP_CLEANUP(pTaskMap, pCurlM);
				goto __LEAVE_CLEAN__;
			}

			// ��������������б�
			/*if (U < MAX_DOWNLOAD_CONNECTS)
			{
				for (auto it = pTaskMap->begin(); it != pTaskMap->end(); it++)
				{
					if (!pfnIsProcessing(thiz))
					{
						TASKMAP_CLEANUP(pTaskMap, pCurlM);
						goto __LEAVE_CLEAN__;
					}
					// ���δ��ʼ, ��������ض���
					if (it->second.state == TASKSTATETYPE::TSTYPE_WAITING)
					{
						if (U < MAX_DOWNLOAD_CONNECTS)
						{
							it->second.state = TASKSTATETYPE::TSTYPE_PROCESSING;
							init_task(pCurlM, &it->second);
							// just to prevent it from remaining at 0 if there are more URLs to get
							U++;
						}
					}
				}
			}
			// ���ȡ���������б�
			for (auto it = pTaskMap->begin(); it != pTaskMap->end(); )
			{
				if (!pfnIsProcessing(thiz))
				{
					TASKMAP_CLEANUP(pTaskMap, pCurlM);
					goto __LEAVE_CLEAN__;
				}
				// �����ȡ��, ��������ض���
				if (it->second.state == TASKSTATETYPE::TSTYPE_CANCEL)
				{
					it->second.state = TASKSTATETYPE::TSTYPE_NULL;
					curl_multi_remove_handle(pCurlM, it->second.pcurl);
					curl_easy_cleanup(it->second.pcurl);
					it->second.pcurl = 0;
					fclose(it->second.pfile);
					it->second.pfile = 0;
					it = pTaskMap->erase(it);
				}
				else
				{
					it++;
				}
			}*/

			//ִ�в������󣬷���������������
			if (curl_multi_perform(pCurlM, &U) == CURLM_CALL_MULTI_PERFORM)
			{
				if (curl_multi_timeout(pCurlM, &L))
				{
					fprintf(stderr, "E: curl_multi_timeout\n");
					goto __LEAVE_CLEAN__;
				}
				if (L <= 0)
				{
					//L = 100;
				}
				FD_ZERO(&R); FD_ZERO(&W); FD_ZERO(&E);
				//��ȡcm��Ҫ�������ļ�����������,������ص�M����-1��
				//��Ҫ��һ��Ȼ���ٴε���curl_multi_perform,�ȶ�ã�
				//��������60���룬��������������Լ����ض������²��������ҵ�һ�����ʵ�ֵ
				if (curl_multi_fdset(pCurlM, &R, &W, &E, &M))
				{
					fprintf(stderr, "E: curl_multi_fdset\n");
					goto __LEAVE_CLEAN__;
				}

				if (M == -1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(L));
				}
				else
				{
					T.tv_sec = L / 1000;
					T.tv_usec = (L % 1000) * 1000;
					/////////////////////////////////////////////////////////////////
					//ȷ��һ�������׽ӿڵ�״̬,�ɲ�ѯ���Ŀɶ��ԡ���д�Լ�����״̬��Ϣ��
					//����ֵ��
					//select()���÷��ش��ھ���״̬�����Ѿ�������fd_set�ṹ�е�������������
					//�����ʱ�򷵻�0������Ļ�������SOCKET_ERROR(-1)����
					//Ӧ�ó����ͨ��WSAGetLastError()��ȡ��Ӧ������롣
					if (select(M + 1, &R, &W, &E, &T) == (-1))
					{
						fprintf(stderr, "E: select(%d,%ld): %d: %s\n", M + 1, L, errno, strerror(errno));
						goto __LEAVE_CLEAN__;
					}
				}
				continue;
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////
			//��ȡ��ǰ������cURL����ش�����Ϣ
			//��ѯ���������Ƿ񵥶��Ĵ����߳�������Ϣ����Ϣ���ء�
			//��Ϣ���ܰ�������ӵ����Ĵ����̷߳��صĴ��������ֻ�Ǵ����߳���û�����֮��ı��档
			//�ظ����������������ÿ�ζ��᷵��һ���µĽ����ֱ����ʱû�и�����Ϣ����ʱ��
			//FALSE ������һ���źŷ��ء�ͨ��msgs_in_queue���ص�����ָ�������������κ��������ú󣬻�ʣ�����Ϣ����
			//Warning	���ص���Դָ������ݵ���curl_multi_remove_handle()�󽫲�����ڡ�
			while ((msg = curl_multi_info_read(pCurlM, &Q)))
			{
				CDebugTraceBlock* pDTB = 0;
				CURLcode curlcode = CURLE_OK;

				curlcode = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pDTB);

				if (msg->msg == CURLMSG_DONE)
				{
					fprintf(stdout, "R: %d - %s <%s>\n", msg->data.result, curl_easy_strerror(msg->data.result), pDTB->url.c_str());
					pTaskMap->at(pDTB->url).state = TASKSTATETYPE::TSTYPE_FINISH;
				}
				else
				{
					fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
					pTaskMap->at(pDTB->url).state = TASKSTATETYPE::TSTYPE_FAILED;
				}
				if (curlcode == CURLE_OK)
				{
					pTaskMap->at(pDTB->url).pcurl = 0;
					fclose(pTaskMap->at(pDTB->url).pfile);
					pTaskMap->at(pDTB->url).pfile = 0;
					resp_data.assign(pDTB->resp_data.data(), pDTB->resp_data.size());
					fprintf(stdout, "Response: %.*s\n", pDTB->resp_data.size(), pDTB->resp_data.data());
					if (pfnNotifyUpdate)
					{
						pfnNotifyUpdate(thiz);
					}
					pTaskMap->erase(pDTB->url);
				}
				curl_multi_remove_handle(pCurlM, msg->easy_handle);
				curl_easy_cleanup(msg->easy_handle);
			}
		}

		result = EXIT_SUCCESS;

	__LEAVE_CLEAN__:

		curl_multi_cleanup(pCurlM);
		pCurlM = 0;
	}

	return result;
}

#include <base64.h>
#include <httptool.h>
#include <string_helper.h>
#include <jsonhelper.h>
#include <openssl/hmac.h>
#include <huobi_signature.h>