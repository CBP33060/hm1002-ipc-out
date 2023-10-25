#ifndef __INI_CONFIG_H__
#define __INI_CONFIG_H__
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace INI
{
	class CINIConfig
	{
	private:
		std::string value_;

	public:
		CINIConfig() : value_()
		{}

		CINIConfig(const std::string &value) : value_(value)
		{}
		CINIConfig(const CINIConfig &field) : value_(field.value_)
		{}

		~CINIConfig()
		{}

		template<typename T>
		T as() const
		{
			return static_cast<T>(*this);
		}

		CINIConfig &operator=(const char *value)
		{
			value_ = std::string(value);
			return *this;
		}

		CINIConfig &operator=(const std::string &value)
		{
			value_ = value;
			return *this;
		}

		CINIConfig &operator=(const CINIConfig &field)
		{
			value_ = field.value_;
			return *this;
		}

		CINIConfig &operator=(const int value)
		{
			std::stringstream ss;
			ss << value;
			value_ = ss.str();
			return *this;
		}

		CINIConfig &operator=(const unsigned int value)
		{
			std::stringstream ss;
			ss << value;
			value_ = ss.str();
			return *this;
		}

		CINIConfig &operator=(const double value)
		{
			std::stringstream ss;
			ss << value;
			value_ = ss.str();
			return *this;
		}

		CINIConfig &operator=(const float value)
		{
			std::stringstream ss;
			ss << value;
			value_ = ss.str();
			return *this;
		}

		CINIConfig &operator=(const bool value)
		{
			if (value)
				value_ = "true";
			else
				value_ = "false";
			return *this;
		}

		explicit operator const char *() const
		{
			return value_.c_str();
		}

		explicit operator std::string() const
		{
			return value_;
		}

		explicit operator int() const
		{
			char *endptr;
			
			int result = std::strtol(value_.c_str(), &endptr, 10);
			if (*endptr == '\0')
				return result;
			
			result = std::strtol(value_.c_str(), &endptr, 8);
			if (*endptr == '\0')
				return result;
			
			result = std::strtol(value_.c_str(), &endptr, 16);
			if (*endptr == '\0')
				return result;

			throw std::invalid_argument("field is not an int");
		}

		explicit operator unsigned int() const
		{
			char *endptr;
			
			int result = std::strtoul(value_.c_str(), &endptr, 10);
			if (*endptr == '\0')
				return result;
			
			result = std::strtoul(value_.c_str(), &endptr, 8);
			if (*endptr == '\0')
				return result;
			
			result = std::strtoul(value_.c_str(), &endptr, 16);
			if (*endptr == '\0')
				return result;

			throw std::invalid_argument("field is not an unsigned int");
		}

		explicit operator float() const
		{
			return std::stof(value_);
		}

		explicit operator double() const
		{
			return std::stod(value_);
		}

		explicit operator bool() const
		{
			std::string str(value_);
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);

			if (str == "TRUE")
				return true;
			else if (str == "FALSE")
				return false;
			else
				throw std::invalid_argument("field is not a bool");
		}
	};

	class CININode : public std::map<std::string, CINIConfig>
	{
	public:
		CININode(){}
		~CININode(){}
	};

	class CINIFile : public std::map<std::string, CININode>
	{
	private:
		char fieldSep_;
		char comment_;

	public:
		CINIFile() : CINIFile('=', '#'){}

		CINIFile(const char fieldSep, const char comment)
			: fieldSep_(fieldSep), comment_(comment){}

		CINIFile(const std::string &filename,
			const char fieldSep = '=',
			const char comment = '#')
			: CINIFile(fieldSep, comment){ load(filename); }

		CINIFile(std::istream &is,
			const char fieldSep = '=',
			const char comment = '#')
			: CINIFile(fieldSep, comment){ decode(is); }

		~CINIFile(){ }

		void setFieldSep(const char sep){ fieldSep_ = sep; }
		void setCommentChar(const char comment){ comment_ = comment; }

		void decode(std::istream &is)
		{
			clear();
			int lineNo = 0;
			CININode *iniNode = NULL;
			
			while (!is.eof() && !is.fail())
			{
				std::string line;
				std::getline(is, line, '\n');
				++lineNo;

				if (line.size() == 0)
					continue;
				
				if (line[0] == comment_)
					continue;
				if (line[0] == '[')
				{
					std::size_t pos = line.find("]");
					if (pos == std::string::npos)
					{
						std::stringstream ss;
						ss << "l" << lineNo
							<< ": ini parsing failed, section not closed";
						throw std::logic_error(ss.str());
					}
					
					if (pos == 1)
					{
						std::stringstream ss;
						ss << "l" << lineNo
							<< ": ini parsing failed, section is empty";
						throw std::logic_error(ss.str());
					}
					
					if (pos + 1 != line.length())
					{
						std::stringstream ss;
						ss << "l" << lineNo
							<< ": ini parsing failed, no end of line after "
							"section";
						throw std::logic_error(ss.str());
					}

					std::string secName = line.substr(1, pos - 1);
					iniNode = &((*this)[secName]);
				}
				else
				{
					if (iniNode == NULL)
					{
						std::stringstream ss;
						ss << "l" << lineNo
							<< ": ini parsing failed, field has no section";
						throw std::logic_error(ss.str());
					}

					std::size_t pos = line.find(fieldSep_);
					if (pos == std::string::npos)
					{
						std::stringstream ss;
						ss << "l" << lineNo
							<< ": ini parsing failed, no '=' found";
						throw std::logic_error(ss.str());
					}
					
					std::string name = line.substr(0, pos);
					std::string value = line.substr(pos + 1, std::string::npos);
					(*iniNode)[name] = value;
				}
			}
		}

		void decode(const std::string &content)
		{
			std::istringstream ss(content);
			decode(ss);
		}

		void load(const std::string &fileName)
		{
			std::ifstream is(fileName.c_str());
			decode(is);
		}

		void encode(std::ostream &os) const
		{
			for (const auto &filePair : *this)
			{
				os << "[" << filePair.first << "]" << std::endl;
				for (const auto &secPair : filePair.second)
					os << secPair.first << fieldSep_
					<< secPair.second.as<std::string>() << std::endl;
			}
		}

		std::string encode() const
		{
			std::ostringstream ss;
			encode(ss);
			return ss.str();
		}

		void save(const std::string &fileName) const
		{
			std::ofstream os(fileName.c_str());
			encode(os);
		}
	};
}
#endif //__INI_CONFIG_H__
