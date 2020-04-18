#pragma once

class CHttpTool
{
public:
	CHttpTool() :m_nProcessingStatus(0), m_nLoadingStatus(0) {};
	~CHttpTool() {};

private:
	std::map<std::string, CDebugTraceBlock> m_TaskMap;
	int m_nProcessingStatus;
	int m_nLoadingStatus;
	std::thread task_thread;

public:
	std::map<std::string, CDebugTraceBlock>* GetTaskMap() { return &m_TaskMap; }
	int IsLoading() { return m_nLoadingStatus; }
	void SetLoading(int nLoadingStatus) { m_nLoadingStatus = nLoadingStatus; }
	int IsProcessing() { return m_nProcessingStatus; }
	void SetProcessing(int nProcessingStatus) { m_nProcessingStatus = nProcessingStatus; }
public:
	static void NotifyUpdate(void* p)
	{
		CHttpTool* thiz = reinterpret_cast<CHttpTool*>(p);
		if (thiz)
		{
		}
	}
	static int IsProcessing(void* p)
	{
		CHttpTool* thiz = reinterpret_cast<CHttpTool*>(p);
		if (thiz)
		{
			return thiz->IsProcessing();
		}
		return (-1);
	}
	static std::map<std::string, CDebugTraceBlock>* GetTaskMap(void* p)
	{
		CHttpTool* thiz = reinterpret_cast<CHttpTool*>(p);
		if (thiz)
		{
			return &thiz->m_TaskMap;
		}
		return 0;
	}
	void request(std::string& resp_data)
	{
		CHttpTool* thiz = reinterpret_cast<CHttpTool*>(this);
		if (thiz)
		{
			thiz->SetLoading(1);
			thiz->SetProcessing(1);
			/*for (auto it = thiz->GetTaskMap()->begin(); thiz->IsLoading() && it != thiz->GetTaskMap()->end(); it++)
			{
				std::string status = "";
				switch (it->second.state)
				{
				case TASKSTATETYPE::TSTYPE_NULL:
				{
					status = ("未开始下载");
				}
				break;
				case TASKSTATETYPE::TSTYPE_WAITING:
				{
					status = ("等待下载中");
				}
				break;
				case TASKSTATETYPE::TSTYPE_PROCESSING:
				{
					status = ("正在下载中");
				}
				break;
				case TASKSTATETYPE::TSTYPE_FINISH:
				{
					status = ("下载完成");
				}
				break;
				case TASKSTATETYPE::TSTYPE_FAILED:
				{
					status = ("下载失败");
				}
				break;
				default:
					break;
				}
			}*/
			async_start(resp_data, &CHttpTool::GetTaskMap, &CHttpTool::NotifyUpdate, &CHttpTool::IsProcessing, thiz);
			thiz->SetProcessing(0);
			thiz->SetLoading(0);
		}
	}
public:
	static CHttpTool* get_instance() {
		static CHttpTool instance;
		return &instance;
	}
};
