#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace ty
{
  struct Type : std::enable_shared_from_this<Type>
  {
    virtual bool operator==(const Type &other) const;
    virtual bool operator!=(const Type &other) const;
    virtual bool match(const Type &other) const;
  };

  struct Undefined : Type
  {
    bool operator==(const Type &other) const override;
  };

  struct Void : Type
  {
    bool operator==(const Type &other) const override;
  };

  struct Nil : Type
  {
    bool operator==(const Type &other) const override;
  };

  struct Int : Type
  {
    bool operator==(const Type &other) const override;
  };

  struct String : Type
  {
    bool operator==(const Type &other) const override;
  };

  struct Named : Type
  {
    std::string name;
    Type *type;

    Named(std::string name, Type *type);

    bool operator==(const Type &other) const override;
  };

  struct Array : Type
  {
    Type *type;

    Array(Type *type);

    bool operator==(const Type &other) const override;
  };

  struct Record : Type
  {
    std::string name;
    std::map<std::string, Type *> records;

    Record(std::string name, std::map<std::string, Type *> records = {});

    bool operator==(const Type &other) const override;
  };
}
