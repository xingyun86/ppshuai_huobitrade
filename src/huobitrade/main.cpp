#include <cstdio>

#include <service_modules.h>

int main(int argc, char** argv)
{
	std::cout << "hello from huobitrade!" << std::endl;
	std::string resp = "";
	CServiceModules::get_instance()->service_startup();
	CServiceModules::get_instance()->get_system_state(resp);
	CServiceModules::get_instance()->get_symbols_info(resp);
	CServiceModules::get_instance()->get_currencys_info(resp);
	CServiceModules::get_instance()->get_timestamp_info(resp);
	CServiceModules::get_instance()->get_accounts_info(resp);
	rapidjson::Document d_accounts; rapidjson::Value& v_accounts = STRING_2_JSON_VALUE(d_accounts, resp);
	if (!v_accounts.ObjectEmpty())
	{
		if (CHECK_JSON_STR(v_accounts, "status"))
		{
			if (std::string(v_accounts["status"].GetString()).compare("ok") == 0)
			{
				if (CHECK_JSON_ARY(v_accounts, "data"))
				{
					for (rapidjson::SizeType i = 0; i < v_accounts["data"].Size(); i++)
					{
						rapidjson::Value& v_account = v_accounts["data"][i];
						if (CHECK_JSON_STR(v_account, "type")
							&& CHECK_JSON_INT(v_account, "id")
							&& CHECK_JSON_STR(v_account, "subtype")
							&& CHECK_JSON_STR(v_account, "state"))
						{
							if (std::string(v_account["state"].GetString()).compare("working") == 0)
							{
								CServiceModules::get_instance()->get_account_balance(resp, std::to_string(v_account["id"].GetInt()));
								CServiceModules::get_instance()->get_account_history(resp, {
									{"account-id", std::to_string(v_account["id"].GetInt())},
									{"currency", "usdt"},
									{"start-time", std::to_string(std::time(nullptr) - 30 * 24 * 60 * 60)},
									{"end-time", std::to_string(std::time(nullptr))}, }
								);
							}
						}
					}
				}
			}
		}
	}

	CServiceModules::get_instance()->get_order_history(resp, {
		{"symbol", "bsvusdt"},
		{"states", "filled"},
		}
	);

	CServiceModules::get_instance()->service_cleanup();
	return 0;
}