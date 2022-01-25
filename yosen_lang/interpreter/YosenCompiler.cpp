#include "YosenCompiler.h"
#include "parser/Parser.h"

namespace yosen
{
    static YosenObject* allocate_literal_object(const std::string& type_str, const std::string& value)
    {
        parser::LiteralType type = parser::LiteralValueToken::type_from_string(type_str);

        switch (type)
        {
        case parser::LiteralType::String:
        {
            return allocate_object<YosenString>(value);
        }
        case parser::LiteralType::Integer:
        {
            return allocate_object<YosenInteger>(static_cast<uint64_t>(std::stoi(value)));
        }
        default: throw "Unknown type";
        }
    }

    static size_t get_function_index(const std::string& fn_name, StackFrame& stack_frame)
    {
        auto it = std::find(
            stack_frame.function_names.begin(),
            stack_frame.function_names.end(),
            fn_name
        );

        // If element was found
        if (it != stack_frame.function_names.end())
            return it - stack_frame.function_names.begin();

        return -1;
    }

    void YosenCompiler::debug_print_bytecode(bytecode_t& bytecode)
    {
        auto it = bytecode.begin();

        while (it != bytecode.end())
        {
            switch (*it)
            {
            case opcodes::LOAD:
            {
                it++;
                printf("LOAD 0x%x\n", *it);
                break;
            }
            case opcodes::LOAD_CONST:
            {
                it++;
                printf("LOAD_CONST 0x%x\n", *it);
                break;
            }
            case opcodes::LOAD_PARAM:
            {
                it++;
                printf("LOAD_PARAM 0x%x\n", *it);
                break;
            }
            case opcodes::STORE:
            {
                it++;
                printf("STORE 0x%x\n", *it);
                break;
            }
            case opcodes::REG_LOAD:
            {
                it++;
                printf("REG_LOAD 0x%x\n", *it);
                break;
            }
            case opcodes::REG_STORE:
            {
                it++;
                printf("REG_STORE 0x%x\n", *it);
                break;
            }
            case opcodes::PUSH:
            {
                printf("PUSH\n");
                break;
            }
            case opcodes::CALL:
            {
                it++;
                auto first_op = *it;

                it++;
                auto second_op = *it;
                printf("CALL 0x%x 0x%x\n", first_op, second_op);
                break;
            }
            default:
                printf("0x%x ", *it);
            }

            it++;
        }
    }

    uint32_t YosenCompiler::get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame)
    {
        auto& node = *node_ptr;

        auto& literal_type = node["value_type"].string_value();
        auto& value = node["value"].string_value();

        uint32_t constant_key = 0;

        // Check if the constant has not been found before
        if (stack_frame.constant_keys.find(value) == stack_frame.constant_keys.end())
        {
            // Allocate object for a constant
            auto obj = allocate_literal_object(literal_type, value);

            // Get the next available constant key
            constant_key = (uint32_t)stack_frame.constants.size();

            // Store the key in a constant keys map
            stack_frame.constant_keys.insert({ value, constant_key });

            // Store the object in a constants map
            stack_frame.constants.insert({ constant_key, obj });
        }
        else
            constant_key = stack_frame.constant_keys.at(value);

        return constant_key;
    }

    uint32_t YosenCompiler::get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame)
    {
        auto& node = *node_ptr;

        // Get the variable name
        auto& identifier_value = node["value"].string_value();

        // Make sure the variable exists
        if (stack_frame.var_keys.find(identifier_value) == stack_frame.var_keys.end())
            throw "Undefined variable used";

        // Get the key for the variable in the stack frame
        auto var_key = stack_frame.var_keys.at(identifier_value);

        return var_key;
    }
}