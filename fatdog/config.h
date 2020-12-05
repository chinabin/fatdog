#ifndef __FATDOG_CONFIG_H__
#define __FATDOG_CONFIG_H__

#include <boost/lexical_cast.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <yaml-cpp/yaml.h>

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

    template <>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> vec;

            for (int i = 0; i < node.size(); ++i)
            {
                auto n = node[i];

                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].as<std::string>());
                ld.formatter = n["formatter"].as<std::string>();

                for (int j = 0; j < n["appender"].size(); ++j)
                {
                    LogAppenderDefine lad;
                    auto na = n["appender"][j];
                    std::string appenderType = na["type"].as<std::string>();
                    if (appenderType == "FileLogAppender")
                    {
                        lad.type = 1;
                        lad.level = LogLevel::FromString(na["level"].as<std::string>());
                        lad.formatter = na["formatter"].as<std::string>();
                        lad.file = na["file"].as<std::string>();
                    }
                    else if(appenderType == "StdoutLogAppender")
                    {
                        lad.type = 2;
                        lad.level = LogLevel::FromString(na["level"].as<std::string>());
                        lad.formatter = na["formatter"].as<std::string>();
                    }
                    ld.appenders.push_back(lad);
                }
                vec.insert(ld);
            }

            return vec;
        }
    };

    template <>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                YAML::Node n;
                n["name"] = i.name;
                n["level"] = LogLevel::ToString(i.level);
                n["formatter"] = i.formatter;

                for (auto &a : i.appenders)
                {
                    YAML::Node na;
                    if (a.type == 1)
                    {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    }
                    else if (a.type == 2)
                    {
                        na["type"] = "StdoutLogAppender";
                    }
                    na["level"] = LogLevel::ToString(a.level);
                    na["formatter"] = a.formatter;
                    n["appenders"].push_back(na);
                }
                node.push_back(n);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &description = "")
            : m_name(name), m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string &val) = 0;
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
            : ConfigVarBase(name, description), m_val(default_value)
        {
        }

        ~ConfigVar() {}

        std::string getTypeName() const override { return typeid(T).name(); }
        const T getValue() const { return m_val; }
        void setValue(const T &v) { m_val = v; }

    public:
        std::string toString() override
        {
            try
            {
                return ToStr()(m_val);
            }
            catch (std::exception &e)
            {
                Logger::ptr logger(new Logger());
                FATDOG_LOG_INFO(logger) << "ConfigVar::toString exception "
                                        << e.what() << " convert: " << typeid(m_val).name() << " to string";
            }
            return "";
        }

        bool fromString(const std::string &str) override
        {
            try
            {
                setValue(FromStr()(str));
                return true;
            }
            catch (std::exception &e)
            {
                Logger::ptr logger(new Logger());
                FATDOG_LOG_INFO(logger) << "ConfigVar::toString exception " 
                                        << e.what() << " convert: string to " << typeid(m_val).name()
                                        << " - " << str;
            }
            return false;
        }

    private:
        T m_val;
    };

    class Config
    {
    public:
        typedef std::shared_ptr<Config> ptr;

        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template <class T>
        static typename ConfigVar<T>::ptr lookUp(const std::string &name,
                                                 const T &default_value, const std::string &description = "")
        {
            Logger::ptr logger(new Logger());
            auto t = m_mapVars.find(name);
            if (t != m_mapVars.end())
            {
                auto v = std::dynamic_pointer_cast<ConfigVar<T>>(t->second);
                if (v)
                {
                    FATDOG_LOG_INFO(logger) << "Lookup name=" << name << " exists";
                    return v;
                }
                else
                {
                    FATDOG_LOG_INFO(logger) << "Lookup name=" << name << " exists but type not "
                                            << typeid(T).name() << " real_type=" << t->second->getTypeName()
                                            << " and val is " << t->second->toString();
                    return nullptr;
                }
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            m_mapVars[name] = v;

            return v;
        }

        template <class T>
        static typename ConfigVar<T>::ptr lookUp(const std::string &name)
        {
            auto t = m_mapVars.find(name);
            if (t == m_mapVars.end())
            {
                return nullptr;
            }

            return std::dynamic_pointer_cast<ConfigVar<T>>(t->second);
        }
        // private:
        static ConfigVarMap m_mapVars;
    };
} // namespace fatdog

#endif
