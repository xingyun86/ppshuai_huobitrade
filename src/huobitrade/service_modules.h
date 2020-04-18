#pragma once

#include <utils.h>

class CServiceModules {
#define NUL_FNAME "NUL"
#define URI_PATH_SPLITTER "/"
#define URI_MARK_SPLITTER "?"
//////////////////////////////////////////
// HTTP METHOD LIST
#define HTTP_METHOD_GET "GET"
#define HTTP_METHOD_PUT "PUT"
#define HTTP_METHOD_POST "POST"
#define HTTP_METHOD_HEAD "HEAD"
#define HTTP_METHOD_TRACE "TRACE"
#define HTTP_METHOD_PATCH "PATCH"
#define HTTP_METHOD_DELETE "DELETE"
#define HTTP_METHOD_OPTIONS "OPTIONS"
//////////////////////////////////////////

//////////////////////////////////////////
// PATH LIST
#define ACCOUNT_ID	"{account-id}"
#define SYMBOLS_INFO "/v1/common/symbols"
#define CURRENCYS_INFO "/v1/common/currencys"
#define TIMESTAMP_INFO "/v1/common/timestamp"
#define ACCOUNTS_INFO "/v1/account/accounts"
#define ACCOUNT_BALANCE "/v1/account/accounts/" ACCOUNT_ID "/balance"
#define ACCOUNT_HISTORY "/v1/account/history"
#define ORDER_HISTORY "/v1/order/orders"
//////////////////////////////////////////

public:
	CServiceModules() :
		schema("https://"),
		host("api.huobi.pro"),
		accesskey("xxxxxx-xxxxxx-xxxxxx-xxxxxx"),
		secretkey("xxxxxx-xxxxxx-xxxxxx-xxxxxx"),
		state_url("https://status.huobigroup.com/api/v2/summary.json") {}
	virtual ~CServiceModules(){}
private:
	std::string schema;
	std::string host;
	std::string accesskey;
	std::string secretkey;
	std::string state_url;
public:
	void service_startup()
	{
		CURL_GLOBAL_INIT(CURL_GLOBAL_DEFAULT);
	}
	void service_cleanup()
	{
		CURL_GLOBAL_EXIT();
	}

	int service_request(
		std::string& resp,
		const std::string& path,
		const std::string& method,
		const std::string& params = "",
		bool signature = true
	)
	{
		CHttpTool httptool;
		std::string req_url;
		if (signature)
		{
			req_url = schema + host + path + URI_MARK_SPLITTER + Huobi::ApiSignature::buildSignaturePath(host, accesskey, secretkey, path, method, params);
		}
		else
		{
			req_url = path;
		}
		httptool.GetTaskMap()->insert(
			std::map<std::string, CDebugTraceBlock>::value_type
			(
				req_url,//std::to_string(std::time(nullptr)),
				CDebugTraceBlock(req_url.data(), NUL_FNAME)
			)
		);
		httptool.request(resp);
		JSON_STR_NULL_2_EMPTY(resp);
		std::cout << resp << std::endl;
		return 0;
	}
	int service_request(
		std::string& resp,
		const std::string& path,
		const std::string& method,
		const std::map<std::string, std::string>& params,
		bool signature = true
	)
	{
		CHttpTool httptool;
		std::string req_url;
		if (signature)
		{
			req_url = schema + host + path + URI_MARK_SPLITTER + Huobi::ApiSignature::buildSignaturePath(host, accesskey, secretkey, path, method, params);
		}
		else
		{
			req_url = path;
		}
		httptool.GetTaskMap()->insert(
			std::map<std::string, CDebugTraceBlock>::value_type
			(
				req_url,//std::to_string(std::time(nullptr)),
				CDebugTraceBlock(req_url.data(), NUL_FNAME)
			)
		);
		httptool.request(resp);
		JSON_STR_NULL_2_EMPTY(resp);
		std::cout << resp << std::endl;
		return 0;
	}

public:

	//获取当前系统状态
	int get_system_state(std::string& resp)
	{
		return service_request(resp, state_url, HTTP_METHOD_GET, "", false);
	}
	//获取交易对信息
	int get_symbols_info(std::string& resp)
	{
		return service_request(resp, SYMBOLS_INFO, HTTP_METHOD_GET);
	}
	//获取币种信息
	int get_currencys_info(std::string& resp)
	{
		return service_request(resp, CURRENCYS_INFO, HTTP_METHOD_GET);
	}
	//获取时间戳信息
	int get_timestamp_info(std::string& resp)
	{
		return service_request(resp, TIMESTAMP_INFO, HTTP_METHOD_GET);
	}
	//获取账户信息
	int get_accounts_info(std::string & resp)
	{
		return service_request(resp, ACCOUNTS_INFO, HTTP_METHOD_GET);
	}
	//获取账户余额
	int get_account_balance(std::string & resp, const std::string & account)
	{
		std::string path = ACCOUNT_BALANCE;
		string_replace_all(path, account, ACCOUNT_ID);
		return service_request(resp, path, HTTP_METHOD_GET);
	}
	//获取账户流水历史
	int get_account_history(std::string& resp, const std::map<std::string, std::string>& params = { {"account-id", ""}, })
	{
		return service_request(resp, ACCOUNT_HISTORY, HTTP_METHOD_GET, params);
	}
	//获取交易流水历史
	int get_order_history(std::string& resp, const std::map<std::string, std::string>& params = { {"symbol", ""}, {"states", "filled"}, })
	{
		return service_request(resp, ORDER_HISTORY, HTTP_METHOD_GET, params);
	}
public:
	static CServiceModules* get_instance() {
		static CServiceModules instance;
		return &instance;
	}
};
