#include <algorithm>
#include <vector>
#include <map>
#include "types.h"

using namespace ty;
using namespace std;

bool Type::operator==(const Type &other) const
{
  return this == &other;
}

bool Type::operator!=(const Type &other) const
{
  return !this->operator==(other);
}

bool Type::match(const Type &other) const
{
  return *this == other;
}

bool Undefined::operator==(const Type &other) const
{
  return false;
}

bool Int::operator==(const Type &other) const
{
  return dynamic_cast<const Int *>(&other) != nullptr;
}

bool String::operator==(const Type &other) const
{
  return dynamic_cast<const String *>(&other) != nullptr;
}

bool Nil::operator==(const Type &other) const
{
  return dynamic_cast<const Nil *>(&other) != nullptr;
}

bool Void::operator==(const Type &other) const
{
  return dynamic_cast<const Void *>(&other) != nullptr;
}

static inline const Array *isArray(const Type *ptr)
{
  return dynamic_cast<const Array *>(ptr);
}

static inline const Record *isRecord(const Type *ptr)
{
  return dynamic_cast<const Record *>(ptr);
}

static inline const Named *isNamed(const Type *ptr)
{
  return dynamic_cast<const Named *>(ptr);
}

static bool deepCompare(const Type *lhs, const Type *rhs)
{
  vector<pair<const Type *, const Type *>> stack{{lhs, rhs}};
  map<const Type *, const Type *> visited;

  auto isVisited = [&visited](const Type *lhs, const Type *rhs) -> bool
  { return visited[lhs] == rhs || visited[rhs] == lhs; };

  auto visit = [&visited](const Type *lhs, const Type *rhs)
  { visited[lhs] = rhs; };

  while (!stack.empty())
  {
    auto lhs = stack.back().first;
    auto rhs = stack.back().second;
    stack.pop_back();

    if (lhs == nullptr && rhs == nullptr)
    {
      continue;
    }
    else if (lhs == nullptr || rhs == nullptr)
    {
      return false;
    }
    if (isArray(lhs) && isArray(rhs))
    {
      if (isVisited(lhs, rhs) || lhs == rhs)
      {
        continue;
      }
      else
      {
        visit(lhs, rhs);
        auto lhs_arr = isArray(lhs);
        auto rhs_arr = isArray(rhs);
        stack.push_back({lhs_arr->type, rhs_arr->type});
      }
    }
    else if (isNamed(lhs) && isNamed(rhs))
    {

      if (isVisited(lhs, rhs) || lhs == rhs)
      {
        continue;
      }
      else
      {
        auto l_named = isNamed(lhs);
        auto r_named = isNamed(rhs);

        if (l_named->name != r_named->name)
        {
          return false;
        }
        else
        {
          visit(lhs, rhs);
          stack.push_back({l_named->type, r_named->type});
        }
      }
    }
    else if (isRecord(lhs) && isRecord(rhs))
    {
      if (isVisited(lhs, rhs) || lhs == rhs)
      {
        continue;
      }
      else
      {
        visit(lhs, rhs);
        auto lhs_rec = isRecord(lhs);
        auto rhs_rec = isRecord(rhs);

        if (lhs_rec->records.size() != rhs_rec->records.size())
        {
          return false;
        }
        else
        {
          for (auto pair : lhs_rec->records)
          {
            auto found = rhs_rec->records.find(pair.first);

            if (found == rhs_rec->records.end())
            {
              return false;
            }
            else
            {
              visit(pair.second, found->second);
            }
          }
        }
      }
    }
    else
    {
      if (*lhs != *rhs)
        return false;
    }
  }

  return true;
}

Named::Named(std::string name, Type *type) : name(name), type(type) {}

bool Named::operator==(const Type &other) const
{
  auto other_named = dynamic_cast<const Named *>(&other);
  if (other_named == nullptr)
    return false;

  return deepCompare(this, other_named);
}

Array::Array(Type *type) : type(type) {}

bool Array::operator==(const Type &other) const
{
  auto other_array = dynamic_cast<const Array *>(&other);
  if (other_array == nullptr)
    return false;

  return deepCompare(this, other_array);
}

Record::Record(std::string name, std::map<std::string, Type *> records) : name(name), records(records) {}

bool Record::operator==(const Type &other) const
{
  auto other_record = dynamic_cast<const Record *>(&other);
  if (other_record == nullptr)
    return false;

  return deepCompare(this, other_record);
}
