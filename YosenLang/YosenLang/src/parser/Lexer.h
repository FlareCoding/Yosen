#pragma once
#include "TokenPool.h"

namespace yosen::parser
{
	class Lexer
	{
	public:
		Lexer() = default;
		~Lexer() = default;

		std::shared_ptr<TokenPool> construct_token_pool(std::string& source);

		void __debug_print_tokens();

	private:
		void parse_line(const std::string& line, size_t lineno);
		void parse_segment(const std::string& segment, size_t lineno);

	private:
		std::shared_ptr<TokenPool> token_pool;

		std::unordered_map<std::string, Keyword> keywords = {
			{ "var",		Keyword::Var	},
			{ "func",		Keyword::Func	},
			{ "return",		Keyword::Return },
			{ "while",		Keyword::While	},
			{ "for",		Keyword::For	},
			{ "break",		Keyword::Break	},
			{ "if",			Keyword::If		},
			{ "elseif",		Keyword::Elif	},
			{ "else",		Keyword::Else	},
			{ "class",		Keyword::Class	},
			{ "this",		Keyword::This	},
			{ "new",		Keyword::New	},
		};

		std::unordered_map<std::string, Operator> single_char_operators = {
			{ "=",		Operator::Assignment	},
			{ "!",		Operator::Not			},
			{ "+",		Operator::Add			},
			{ "-",		Operator::Sub			},
			{ "*",		Operator::Mul			},
			{ "/",		Operator::Div			},
			{ "<",		Operator::LessThan		},
			{ ">",		Operator::GreaterThan	},
		};

		std::unordered_map<std::string, Operator> double_char_operators = {
			{ "<=",		Operator::LessThanOrEqual			},
			{ ">=",		Operator::GreaterThanOrEqual		},
			{ "==",		Operator::Equequ					},
			{ "!=",		Operator::Notequ					},
			{ "&&",		Operator::And						},
			{ "||",		Operator::Or						},
			{ "++",		Operator::Increment					},
			{ "--",		Operator::Decrement					},
			{ "+=",		Operator::AdditionAssignment		},
			{ "-=",		Operator::SubtractionAssignment		},
			{ "*=",		Operator::MultiplicationAssignment	},
			{ "/=",		Operator::DivisionAssignment		},
		};

		std::unordered_map<std::string, Symbol> symbols = {
			{ "{",		Symbol::BraceOpen			},
			{ "}",		Symbol::BraceClose			},
			{ "[",		Symbol::BracketOpen			},
			{ "]",		Symbol::BracketClose		},
			{ "(",		Symbol::ParenthesisOpen		},
			{ ")",		Symbol::ParenthesisClose	},
			{ ",",		Symbol::Comma				},
			{ ".",		Symbol::Period				},
			{ ":",		Symbol::Colon				},
			{ ";",		Symbol::Semicolon			},
		};
	};
}
