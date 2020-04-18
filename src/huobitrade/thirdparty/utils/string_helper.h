#pragma once

#include <regex>

__inline static bool string_regex_valid(std::string data, std::string pattern)
{
    return std::regex_match(data, std::regex(pattern));
}
__inline static size_t string_regex_replace_all(std::string& result, std::string& data, std::string replace, std::string pattern, std::regex_constants::match_flag_type matchflagtype = std::regex_constants::match_default)
{
    try
    {
        data = std::regex_replace(data, std::regex(pattern), replace, matchflagtype);
    }
    catch (const std::exception& e)
    {
        result = e.what();
    }
    return data.length();
}
__inline static size_t string_regex_find(std::string& result, std::vector<std::vector<std::string>>& svv, std::string& data, std::string pattern)
{
    std::smatch smatch;
    std::string::const_iterator ite = data.end();
    std::string::const_iterator itb = data.begin();
    try
    {
        //如果匹配成功
        while (std::regex_search(itb, ite, smatch, std::regex(pattern)))
        {
            if (smatch.size() > 1)
            {
                for (size_t stidx = svv.size() + 1; stidx < smatch.size(); stidx++)
                {
                    svv.push_back(std::vector<std::string>());
                }
                for (size_t stidx = 1; stidx < smatch.size(); stidx++)
                {
                    svv.at(stidx - 1).push_back(std::string(smatch[stidx].first, smatch[stidx].second));
                    itb = smatch[stidx].second;
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        result = e.what();
    }
    return svv.size();
}
__inline static std::string::size_type string_reader(std::string& result, std::string strData,
    std::string strStart, std::string strFinal, std::string::size_type stPos = 0,
    bool bTakeStart = false, bool bTakeFinal = false)
{
    std::string::size_type stRetPos = std::string::npos;
    std::string::size_type stStartPos = stPos;
    std::string::size_type stFinalPos = std::string::npos;

    stStartPos = strData.find(strStart, stStartPos);
    if (stStartPos != std::string::npos)
    {
        stRetPos = stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
        if (stFinalPos != std::string::npos)
        {
            if (!bTakeStart)
            {
                stStartPos += strStart.length();
            }
            if (bTakeFinal)
            {
                stFinalPos += strFinal.length();
            }
            result = strData.substr(stStartPos, stFinalPos - stStartPos);
        }
    }
    return stRetPos;
}
__inline static std::string string_reader(std::string strData,
    std::string strStart, std::string strFinal,
    bool bTakeStart = false, bool bTakeFinal = false)
{
    std::string strRet = ("");
    std::string::size_type stStartPos = std::string::npos;
    std::string::size_type stFinalPos = std::string::npos;
    stStartPos = strData.find(strStart);
    if (stStartPos != std::string::npos)
    {
        stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
        if (stFinalPos != std::string::npos)
        {
            if (!bTakeStart)
            {
                stStartPos += strStart.length();
            }
            if (bTakeFinal)
            {
                stFinalPos += strFinal.length();
            }
            strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
        }
    }
    return strRet;
}
__inline static std::string string_replace_all(std::string& strData, std::string strDst, std::string strSrc, std::string::size_type stPos = 0)
{
    while ((stPos = strData.find(strSrc, stPos)) != std::string::npos)
    {
        strData.replace(stPos, strSrc.length(), strDst);
    }
    return strData;
}
__inline static size_t string_split_to_vector(std::vector<std::string>& sv, std::string strData, std::string strSplitter, std::string::size_type stPos = 0)
{
    std::string strTmp = ("");
    std::string::size_type stIdx = 0;
    std::string::size_type stSize = strData.length();
    while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
    {
        strTmp = strData.substr(stIdx, stPos - stIdx);
        if (!strTmp.length())
        {
            strTmp = ("");
        }
        sv.push_back(strTmp);
        stIdx = stPos + strSplitter.length();
    }
    if (stIdx < stSize)
    {
        strTmp = strData.substr(stIdx, stSize - stIdx);
        if (!strTmp.length())
        {
            strTmp = ("");
        }
        sv.push_back(strTmp);
    }
    return sv.size();
}