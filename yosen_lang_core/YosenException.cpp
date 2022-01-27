#include "YosenException.h"

namespace yosen
{
	YosenException::YosenException(const std::string& reason, const std::string& type)
		: m_reason(reason), m_type(type) {}
	
	std::string YosenException::to_string() const
	{
		return m_type + ": " + m_reason;
	}
	
	std::string YosenException::type() const
	{
		return m_type;
	}
	
	std::string YosenException::reason() const
	{
		return m_reason;
	}
}
