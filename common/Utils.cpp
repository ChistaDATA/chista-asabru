#include "Utils.h"

// for string delimiter
vector<string> split(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

pair<string, string> ChopLine(string str)
{
    char buffer[4096];
    ::strcpy(buffer, str.c_str());
    char *ptr = buffer;
    string key;
    string value;
    while (*ptr != 0 && *ptr != ':')
    {
        ptr++;
    }
    if (*ptr == 0)
    {
        return pair<string, string>("", "");
    }
    *ptr++ = 0;
    key = string(buffer);
    value = string(ptr);
    return pair<string, string>(key, value);
}