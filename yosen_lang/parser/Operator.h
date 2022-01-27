#pragma once

namespace yosen::parser
{
	enum class Operator
	{
		Unknown = -1,
		Add,
		Sub,
		Mul,
		Div,
		Assignment,
		AdditionAssignment,
		SubtractionAssignment,
		MultiplicationAssignment,
		DivisionAssignment,
		Increment,
		Decrement,
		LessThan,
		GreaterThan,
		LessThanOrEqual,
		GreaterThanOrEqual,
		Equequ,
		Notequ,
		Not,
		Or,
		And,
		BitOr,
		BitAnd,
		Namespace,
	};
}
