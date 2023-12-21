#pragma once
#include <vector>
#include <map>

namespace tb
{
  template <typename K, typename V>
  class Table
  {
  private:
    std::vector<std::map<K, V>> _map;

  public:
    Table() : _map(std::vector<std::map<K, V>>{std::map<K, V>()}){};

    void insert(K key, V value)
    {
      _map.back()[key] = value;
    }

    V *find(K key)
    {
      for (auto it = _map.rbegin(); it != _map.rend(); ++it)
      {
        auto found = it->find(key);
        if (found != it->end())
        {
          return &found->second;
        }
      }
      return nullptr;
    }

    V *find_top(K key)
    {
      auto top = _map.back();
      auto found = top.find(key);
      if (found != top.end())
      {
        return &found->second;
      }
      else
      {
        return nullptr;
      }
    }

    void enter()
    {
      _map.push_back(std::map<K, V>());
    }

    void exit()
    {
      _map.pop_back();
    }

    auto top_begin()
    {
      return _map.back().begin();
    }

    auto top_end()
    {
      return _map.back().end();
    }
  };
}
