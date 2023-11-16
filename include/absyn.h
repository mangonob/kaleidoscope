#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "location.hh"

namespace absyn
{
  struct Exp;

  struct ID
  {
    std::string id;
    yy::position pos;

    ID(std::string id, yy::position pos);

    virtual std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct Record
  {
    std::shared_ptr<ID> name;
    std::shared_ptr<Exp> value;
    yy::position pos;

    Record(
        ID *name,
        Exp *value,
        yy::position pos);
  };

  struct Var
  {
    yy::position pos;

    Var(yy::position pos);

    virtual std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct SimpleVar : Var
  {
    std::shared_ptr<ID> name;

    SimpleVar(ID *name, yy::position pos);
  };

  struct FieldVar : Var
  {
    std::shared_ptr<Var> var;
    std::shared_ptr<ID> field;

    FieldVar(Var *var, ID *field, yy::position pos);
  };

  struct SubscriptVar : Var
  {
    std::shared_ptr<Var> var;
    std::shared_ptr<Exp> subscript;

    SubscriptVar(Var *var, Exp *subscript);
  };

  struct Field
  {
    std::shared_ptr<ID> name;
    std::shared_ptr<ID> type_id;
    yy::position pos;
    bool escape;

    Field(
        ID *name,
        ID *type_id,
        yy::position pos,
        bool escape);

    std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct Type
  {
    yy::position pos;

    Type(yy::position pos);

    virtual std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct NamedType : Type
  {
    std::shared_ptr<ID> named;

    NamedType(ID *named, yy::position pos);
  };

  struct ArrayType : Type
  {
    std::shared_ptr<ID> array;

    ArrayType(ID *array, yy::position pos);
  };

  struct RecordType : Type
  {
    std::vector<Field> fields;

    RecordType(std::vector<Field> fields, yy::position pos);
  };

  struct Dec
  {
    yy::position pos;

    Dec(yy::position pos);

    virtual std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct TypeDec : Dec
  {
    std::shared_ptr<ID> type_id;
    std::shared_ptr<Type> type;

    TypeDec(
        ID *type_id,
        Type *type,
        yy::position pos);
  };

  struct VarDec : Dec
  {
    std::shared_ptr<ID> var;
    std::shared_ptr<ID> type_id;
    std::shared_ptr<Exp> exp;
    bool escape;

    VarDec(
        ID *var,
        ID *type_id,
        Exp *exp,
        bool escape,
        yy::position pos);
  };

  struct FunctionDec : Dec
  {
    std::shared_ptr<ID> funcname;
    std::vector<Field> parameters;
    std::shared_ptr<ID> return_type;
    std::shared_ptr<Exp> body;

    FunctionDec(
        ID *funcname,
        std::vector<Field> parameters,
        ID *return_type,
        Exp *body,
        yy::position pos);
  };

  typedef enum
  {
    plusOp,
    minusOp,
    timesOp,
    divideOp,
    eqOp,
    neqOp,
    ltOp,
    leOp,
    gtOp,
    geOp,
  } Oper;

  struct Exp
  {
    yy::position pos;

    Exp(yy::position pos);

    virtual std::ostream &print(std::ostream &o, int depth, bool isInline);
  };

  struct Nil : Exp
  {
  };

  struct Int : Exp
  {
    int value;

    Int(int value, yy::position pos);
  };

  struct String : Exp
  {
    std::string value;

    String(std::string value, yy::position pos);
  };

  struct VarExp : Exp
  {
    std::shared_ptr<Var> var;

    VarExp(Var *var, yy::position pos);
  };

  struct Assign : Exp
  {
    std::shared_ptr<Var> var;
    std::shared_ptr<Exp> exp;

    Assign(
        Var *var,
        Exp *exp,
        yy::position pos);
  };

  struct Call : Exp
  {
    std::shared_ptr<ID> func;
    std::vector<Exp> args;

    Call(
        ID *func,
        std::vector<Exp> args,
        yy::position pos);
  };

  struct BinOp : Exp
  {
    std::shared_ptr<Exp> rhs;
    std::shared_ptr<Exp> lhs;
    Oper op;

    BinOp(
        Exp *rhs,
        Exp *lhs,
        Oper op,
        yy::position pos);
  };

  struct RecordExp : Exp
  {
    std::shared_ptr<ID> type_id;
    std::vector<Record> records;

    RecordExp(
        ID *type_id,
        std::vector<Record> records,
        yy::position pos);
  };

  struct Array : Exp
  {
    std::shared_ptr<ID> type_id;
    std::shared_ptr<Exp> capacity;
    std::shared_ptr<Exp> element;

    Array(
        ID *type_id,
        Exp *capacity,
        Exp *element,
        yy::position pos);
  };

  struct If : Exp
  {
    std::shared_ptr<Exp> condition;
    std::shared_ptr<Exp> then;
    std::shared_ptr<Exp> els;

    If(
        Exp *condition,
        Exp *then,
        Exp *els,
        yy::position pos);
  };

  struct While : Exp
  {
    std::shared_ptr<Exp> condition;
    std::shared_ptr<Exp> body;

    While(
        Exp *condition,
        Exp *body,
        yy::position pos);
  };

  struct For : Exp
  {
    std::shared_ptr<Var> var;
    std::shared_ptr<Exp> from;
    std::shared_ptr<Exp> to;
    std::shared_ptr<Exp> body;
    bool escape;

    For(
        Var *var,
        Exp *from,
        Exp *to,
        Exp *body,
        bool escape,
        yy::position pos);
  };

  struct Let : Exp
  {
    std::vector<Dec> decs;
    std::shared_ptr<Exp> body;

    Let(
        std::vector<Dec> decs,
        Exp *body,
        yy::position pos);
  };
}
