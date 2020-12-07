#include "../fatdog/config.h"
#include "../fatdog/log.h"

#include <iostream>
#include <map>
#include <list>
#include <yaml-cpp/yaml.h>

static std::map<YAML::NodeType::value, std::string> m = {
    {YAML::NodeType::Undefined, "Undefined"},
    {YAML::NodeType::Null, "Null"},
    {YAML::NodeType::Scalar, "Scalar"},
    {YAML::NodeType::Sequence, "Sequence"},
    {YAML::NodeType::Map, "Map"},
};

void print_yaml(const YAML::Node &node, int level)
{
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << std::string(level * 4, ' ')
                            << "I am: " << m[node.Type()] << " size = " << node.size();
    if (node.IsScalar())
    {
        FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << std::string(level * 4, ' ')
                                << node.Scalar() << " - " << m[node.Type()] << " - " << level;
    }
    else if (node.IsNull())
    {
        FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << std::string(level * 4, ' ')
                                << "NULL - " << m[node.Type()] << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin();
             it != node.end(); ++it)
        {
            FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << std::string(level * 4, ' ')
                                    << it->first << " - " << m[it->second.Type()] << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << std::string(level * 4, ' ')
                                    << i << " - " << m[node[i].Type()] << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test()
{
    // YAML::Node root = YAML::LoadFile("/home/ubuntu/fatdog/tests/test.yml");
    // print_yaml(root, 0);
    // FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << root.Scalar();

    // FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << root["test"].IsDefined();
    // FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << root["logs"].IsDefined();
    // FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << root;

    fatdog::ConfigVar<int> t("lala", 1, "test");
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << t.toString();
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << t.fromString("789");
}

void test1()
{
    auto t1 = fatdog::Config::lookUp("size", (int)18, "hahahaha");
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << t1->toString();
    auto t2 = fatdog::Config::lookUp("size", (float)18, "hahahaha");

    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "lulala~";
    FATDOG_LOG_INFO(FATDOG_LOG_NAME("system")) << "gua gua gua";
    std::cout << LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/ubuntu/fatdog/tests/test.yml");
    fatdog::Config::LoadFromYaml(root);
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "lulala~";
    FATDOG_LOG_INFO(FATDOG_LOG_NAME("system")) << "gua gua gua";
    std::cout << LoggerMgr::GetInstance()->toYamlString() << std::endl;
}

int main()
{
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "hello, test_config";
    test1();

    return 0;
}
