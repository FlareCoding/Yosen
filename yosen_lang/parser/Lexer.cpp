#include "Lexer.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <iomanip>

namespace yosen::parser
{
	void remove_multiline_comments(std::string& str)
	{
		auto start_idx = str.find("/*");
		auto end_idx = str.find("*/");

		while (start_idx != std::string::npos &&
			end_idx != std::string::npos)
		{
			str.erase(start_idx, end_idx - start_idx + 2);

			start_idx = str.find("/*");
			end_idx = str.find("*/");
		}
	}

	std::vector<std::string> remove_new_lines(const std::string& str)
	{
		std::vector<std::string> strings;

		std::regex rgx("\n");
		std::sregex_token_iterator iter(str.begin(), str.end(), rgx, -1);
		std::sregex_token_iterator end;
		for (; iter != end; ++iter)
			strings.push_back(*iter);

		return strings;
	}

	std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
	{
		return ltrim(rtrim(s, t), t);
	}

	std::string unescape(const std::string& str)
	{
		std::string res;
		std::string::const_iterator it = str.begin();
		while (it != str.end())
		{
			char c = *it++;
			if (c == '\\' && it != str.end())
			{
				switch (*it++) {
				case '\\': c = '\\'; break;
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case 'r': c = '\r'; break;
				case 'a': c = '\a'; break;
				case 'b': c = '\b'; break;
				case 'v': c = '\v'; break;
				case '"': c = '\f'; break;
					// The \" is being converted to form feed escape sequence
					// to later be added to the string as a valid quote.
				default:
					continue;
				}
			}
			res += c;
		}

		return res;
	}

	bool is_integer(const std::string& str)
	{
		return std::all_of(str.begin(), str.end(), ::isdigit);
	}

	bool is_float(const std::string& str)
	{
		std::istringstream iss(str);
		float f;
		iss >> std::noskipws >> f;
		return iss.eof() && !iss.fail();
	}

	bool is_boolean(const std::string& str)
	{
		return (str == "true") || (str == "false");
	}

	template <typename T>
	bool IsInMap(const std::string& str, std::unordered_map<std::string, T>& container)
	{
		return (container.find(str) != container.end());
	}

	std::shared_ptr<TokenPool> Lexer::construct_token_pool(std::string& source)
	{
		token_pool = std::make_shared<TokenPool>();

		// Pre-filtering and removing multiline comments
		remove_multiline_comments(source);

		auto lines = remove_new_lines(source);
		size_t lineno = 1;
		for (auto& line : lines)
		{
			// Parse the line if it's not empty
			if (line.find_first_not_of(' ') != std::string::npos)
				parse_line(line, lineno);

			++lineno;
		}

		// Add an EOF token
		token_pool->add(make_token<EOFToken>());

		return token_pool;
	}

	void Lexer::parse_line(const std::string& line, size_t lineno)
	{
		std::string line_copy = line;
		trim(line_copy);
		if (line_copy.find("//") == 0) // Line is a comment
			return;

		auto unescaped_line = unescape(line);

		unsigned counter = 0;
		std::string segment;
		std::stringstream stream_input(unescaped_line);
		while (std::getline(stream_input, segment, '\"'))
		{
			++counter;
			if (counter % 2 == 0)
			{
				if (!segment.empty())
				{
					// Replacing the previously fixed form-feed escape sequences
					// with quotes.
					std::replace(segment.begin(), segment.end(), '\f', '\"');

					auto token = make_token<LiteralValueToken>(LiteralType::String, segment);
					token->lineno = lineno;
					token_pool->add(token);
				}
			}
			else
			{
				std::stringstream stream_segment(segment);
				while (std::getline(stream_segment, segment, ' '))
				{
					if (!segment.empty())
					{
						// Removing the leading and trailing whitespace
						trim(segment);
						parse_segment(segment, lineno);
					}
				}
			}
		}
	}
	
	void Lexer::parse_segment(const std::string& segment, size_t lineno)
	{
		std::regex rx(R"(::|>=|<=|/=|\*=|\-=|\-\-|\+=|\+\+|!=|==|&&|\|\||[!();:=,{}\[\]+*/\-])");
		std::sregex_token_iterator srti(segment.begin(), segment.end(), rx, { -1, 0 });
		std::vector<std::string> tokens;
		std::remove_copy_if(srti, std::sregex_token_iterator(),
			std::back_inserter(tokens),
			[](std::string const& s) { return s.empty(); });

		for (auto& token_str : tokens)
		{
			// Checking if value is null
			if (token_str == "null")
			{
				auto token = make_token<LiteralValueToken>(LiteralType::Null, token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is an integer
			if (is_integer(token_str))
			{
				auto token = make_token<LiteralValueToken>(LiteralType::Integer, token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a float
			if (is_float(token_str))
			{
				auto token = make_token<LiteralValueToken>(LiteralType::Float, token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a boolean
			if (is_boolean(token_str))
			{
				auto token = make_token<LiteralValueToken>(LiteralType::Boolean, token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a double character operator
			if (IsInMap<Operator>(token_str,double_char_operators))
			{
				auto token = make_token<OperatorToken>(double_char_operators[token_str], token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a single character operator
			if (IsInMap<Operator>(token_str,single_char_operators))
			{
				auto token = make_token<OperatorToken>(single_char_operators[token_str], token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a symbol
			if (IsInMap<Symbol>(token_str,symbols))
			{
				auto token = make_token<SymbolToken>(symbols[token_str], token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// Checking if token is a keyword
			if (IsInMap<Keyword>(token_str,keywords))
			{
				auto token = make_token<KeywordToken>(keywords[token_str], token_str);
				token->lineno = lineno;
				token_pool->add(token);
				continue;
			}

			// If no above condition is met, then
			// the token will be considered an identifier.
			// We also need to split the segment by period (to separate member functions).
			std::string tmp;
			std::stringstream stream(token_str);
			std::vector<std::string> segment_tokens;
			while (std::getline(stream, tmp, '.')) {
				segment_tokens.push_back(tmp);
			}

			for (size_t i = 0; i < segment_tokens.size(); i++)
			{
				if (IsInMap<Keyword>(segment_tokens[i],keywords))
				{
					auto kwd_token = make_token<KeywordToken>(keywords[segment_tokens[i]], segment_tokens[i]);
					kwd_token->lineno = lineno;
					token_pool->add(kwd_token);
				}
				else
				{
					auto id_token = make_token<IdentifierToken>(segment_tokens[i]);
					id_token->lineno = lineno;
					token_pool->add(id_token);
				}

				if (i < segment_tokens.size() - 1)
				{
					auto token = make_token<SymbolToken>(Symbol::Period, ".");
					token->lineno = lineno;
					token_pool->add(token);
				}
			}
		}
	}
	
	void Lexer::__debug_print_tokens()
	{
		for (auto& token : token_pool->get_all_tokens())
			printf("%s\n", token->to_string().c_str());
	}
}
