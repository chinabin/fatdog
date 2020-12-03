#ifndef __FATDOG_CONFIG_H__
#define __FATDOG_CONFIG_H__

#include <boost/lexical_cast.hpp>
#include <string>

#include "log.h"

/*
 * with link https://github.com/jbeder/yaml-cpp/wiki/Tutorial , you can get a turorial about yaml-cpp.
 * 
 * in a yaml file
 *  when you see ':', it means map
 *  when you see '-', it means sequence
 *  when you see '[]', it means sequence too
 * */

namespace fatdog
{

    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &val)
        {
            return boost::lexical_cast<T>(val);
        }
    };

    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar
    {
    public:
        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
            : m_name(name), m_val(default_value), m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        ~ConfigVar() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }
        std::string getTypeName() const { return typeid(T).name(); }
        const T getValue() const { return m_val; }
        void setValue(const T &v) { m_val = v; }

    public:
        std::string ToString()
        {
            try
            {
                return ToStr()(m_val);
            }
            catch (std::exception &e)
            {
                Logger::ptr logger(new Logger());
                FATDOG_LOG_INFO(logger) << "ConfigVar::toString exception"
                                        << e.what() << " convert: " << typeid(m_val).name() << " to string";
            }
        }

        T FromString(const std::string &str)
        {
            try
            {
                m_val = FromStr()(str);
                return getValue();
            }
            catch (std::exception &e)
            {
                Logger::ptr logger(new Logger());
                FATDOG_LOG_INFO(logger) << "ConfigVar::toString exception"
                                        << e.what() << " convert: string to " << typeid(m_val).name()
                                        << " - " << str;
            }
        }

    private:
        T m_val;
        std::string m_name;
        std::string m_description;
    };

} // namespace fatdog

#endif
