#ifndef __FATDOG_CONFIG_H__
#define __FATDOG_CONFIG_H__

#include <boost/lexical_cast.hpp>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
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
        typedef std::function<void(const T &old_value, const T &new_value)> on_change_cb;

        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
            : ConfigVarBase(name, description), m_val(default_value)
        {
        }

        ~ConfigVar() {}

        std::string getTypeName() const override { return typeid(T).name(); }
        const T getValue() const { return m_val; }
        void setValue(const T &v)
        {
            if (v == m_val)
            {
                return;
            }
            for (auto &i : m_cbs)
            {
                i.second(m_val, v);
            }

            m_val = v;
        }

        uint64_t addListener(on_change_cb cb)
        {
            static uint64_t s_fun_id = 0;
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }

        void delListener(uint64_t key)
        {
            m_cbs.erase(key);
        }

        on_change_cb getListener(uint64_t key)
        {
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

    public:
        std::string toString() override
        {
            try
            {
                return ToStr()(m_val);
            }
            catch (std::exception &e)
            {
                FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "ConfigVar::toString exception "
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
                FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "ConfigVar::toString exception "
                                                   << e.what() << " convert: string to " << typeid(m_val).name()
                                                   << " - " << str;
            }
            return false;
        }

    private:
        T m_val;

        std::map<uint64_t, on_change_cb> m_cbs;
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
            auto t = GetDatas().find(name);
            if (t != GetDatas().end())
            {
                auto v = std::dynamic_pointer_cast<ConfigVar<T>>(t->second);
                if (v)
                {
                    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "Lookup name=" << name << " exists";
                    return v;
                }
                else
                {
                    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                                                       << typeid(T).name() << " real_type=" << t->second->getTypeName()
                                                       << " and val is " << t->second->toString();
                    return nullptr;
                }
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = v;

            return v;
        }

        template <class T>
        static typename ConfigVar<T>::ptr lookUp(const std::string &name)
        {
            auto t = GetDatas().find(name);
            if (t == GetDatas().end())
            {
                return nullptr;
            }

            return std::dynamic_pointer_cast<ConfigVar<T>>(t->second);
        }

        // 将已经保存的配置项(字符串)加载为配置
        static void LoadFromYaml(const YAML::Node &root);

        // 返回已经保存的配置项的基类指针
        static ConfigVarBase::ptr LookupBase(const std::string &name);

    private:
        // static ConfigVarMap m_mapVars;
        static ConfigVarMap &GetDatas()
        {
            static ConfigVarMap s_datas;
            return s_datas;
        }
    };
} // namespace fatdog

#endif
