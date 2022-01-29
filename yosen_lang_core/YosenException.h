#pragma once
#include <YosenCore.h>
#include <string>

namespace yosen
{
    class YosenException
    {
    public:
        YOSENAPI YosenException(const std::string& reason = "exception", const std::string& type = "GeneralException");

        YOSENAPI std::string to_string() const;

        YOSENAPI std::string type() const;
        YOSENAPI std::string reason() const;

    protected:
        std::string m_type;
        std::string m_reason;
    };

    class ParserException : public YosenException
    {
    public:
        YOSENAPI ParserException(const std::string& reason)
        {
            m_reason = reason;
            m_type = "ParserException";
        }
    };

    class CompilerException : public YosenException
    {
    public:
        YOSENAPI CompilerException(const std::string& reason)
        {
            m_reason = reason;
            m_type = "CompilerException";
        }
    };

    class RuntimeException : public YosenException
    {
    public:
        YOSENAPI RuntimeException(const std::string& reason)
        {
            m_reason = reason;
            m_type = "RuntimeException";
        }
    };

    class KeyboardInterruptException : public YosenException
    {
    public:
        YOSENAPI KeyboardInterruptException()
        {
            m_reason = "Thread terminated with Ctrl+C";
            m_type = "KeyboardInterruptException";
        }
    };
}
