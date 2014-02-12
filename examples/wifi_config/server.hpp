#pragma once

#include <map>
#include <vector>
#include <string>
#include <mongcpp.h>

#include "exec.hpp"

class GPIOServer: public mongoose::MongooseServer {
public:
	GPIOServer(): mongoose::MongooseServer() {}
	virtual ~GPIOServer() {}
protected:
	virtual bool handleEvent(mongoose::ServerHandlingEvent eventCode, 
							 mongoose::MongooseConnection &connection, 
							 const mongoose::MongooseRequest &request, 
							 mongoose::MongooseResponse &response)
	{
		bool res = false;
		if (eventCode == MG_NEW_REQUEST) {
			res = true;
			handleInfo(request,response );
		}

		return res;
	}

	void handleInfo(const mongoose::MongooseRequest &request, 
					mongoose::MongooseResponse &response)
	{
		auto s = request.getQueryString();
		auto m = parseQueryString(s);

		if (m.find("networkName") != m.end() &&  m.find("encryptionType") != m.end())
		{
			setWPA(m["networkName"], m["key"], encType(m["encryptionType"]));
		}

		response.setStatus(200);
		response.setConnectionAlive(false);
		response.setCacheDisabled();
		response.setContentType("text/html");
		response.addContent(generateInfoContent(request, m["networkName"]));
		response.write();
	}

	const std::string generateInfoContent(const mongoose::MongooseRequest &request, const std::string &cur = std::string()) 
	{
		std::string result;
		auto essidlist = iwlist();
		std::string networks;

		std::string curNetwork;
		if (cur.empty())
		{
			curNetwork = exec("/sbin/iwconfig wlan0");
			std::string::size_type pos = curNetwork.find("ESSID:\"");
			if (pos != std::string::npos)
			{
				pos += strlen("ESSID:\"");
				curNetwork=curNetwork.substr(pos,curNetwork.find('\"', pos+1)-pos);
			}
		}
		else
			curNetwork = cur;

		for(auto it = essidlist.begin(); it != essidlist.end(); ++it)
		{
			networks += "<option";
			if (*it == curNetwork)
				networks += " selected";
			networks += (">" +*it+"</option>");
		}

		std::ifstream t("/home/pi/test/index.html");
		std::stringstream buffer;
		buffer << t.rdbuf();
		result = buffer.str();
		result.replace(result.find("<!--CHANGE-->"),strlen("<!--CHANGE-->"), networks);

		return result;
	}
private:
	static void tokenize(const std::string& str, std::vector<std::string>& tokens,
		const std::string& delimiters = " ", bool trimEmpty = false)
	{
		std::string::size_type pos, lastPos = 0;
		while(true)
		{
			pos = str.find_first_of(delimiters, lastPos);
			if(pos == std::string::npos)
			{
				pos = str.length();

				if(pos != lastPos || !trimEmpty)
					tokens.push_back(urldecode(std::string(str.data()+lastPos,pos-lastPos)));

				break;
			}
			else
			{
				if(pos != lastPos || !trimEmpty)
					tokens.push_back(urldecode(std::string(str.data()+lastPos,pos-lastPos)));
			}

			lastPos = pos + 1;
		}
	};

	static std::map<std::string, std::string> parseQueryString(const std::string &query)
	{
		std::vector<std::string> vec;
		std::map<std::string, std::string> res;

		tokenize(query, vec, "&");
		std::for_each(vec.begin(), vec.end(), [&](const std::string& s)
		{
			std::vector<std::string> vec;
			tokenize(s, vec, "=");
			if (vec.size()==2)
				res[vec[0]] = vec[1];
		});

		return res;
	}

	static std::string urldecode(const std::string &src)
	{
		std::string dst;
		dst.reserve(src.length());
		char a, b;
		for(size_t i = 0, n = src.length(); i < n; ++i)
		{
			a = src[i];
			switch (a)
			{
			case '%':
				a = src[i++];
				b = src[i++];
				if (a && b && isxdigit(a) && isxdigit(b))
				{
					if (a >= 'a')
						a -= 'A'-'a';
					if (a >= 'A')
						a -= ('A' - 10);
					else
						a -= '0';
					if (b >= 'a')
						b -= 'A'-'a';
					if (b >= 'A')
						b -= ('A' - 10);
					else
						b -= '0';
					dst.push_back(16*a+b);
				}
			case '+':
				dst.push_back(' ');
				break;
			default:
				dst.push_back(a);
				break;
			}
		}
		return dst;
	}
};
