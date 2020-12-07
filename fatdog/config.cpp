#include "config.h"

// fatdog::Config::ConfigVarMap fatdog::Config::m_mapVars;

namespace fatdog
{

void Config::LoadFromYaml(const YAML::Node& root)
{
    if(root.IsMap()) 
    {
        for(auto it = root.begin(); it != root.end(); ++it) 
        {
            std::string key = it->first.Scalar();
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            if(var) 
            {
                if(it->second.IsScalar()) 
                {
                    var->fromString(it->second.Scalar());
                } else 
                {
                    std::stringstream ss;
                    ss << it->second;
                    var->fromString(ss.str());
                }
            }
        }
    }
}

ConfigVarBase::ptr Config::LookupBase(const std::string& name)
{
    auto t = GetDatas().find(name);
    if (t == GetDatas().end())
    {
        return nullptr;
    }

    return t->second;
}

}