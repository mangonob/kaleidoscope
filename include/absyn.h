#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "location.hh"

namespace absyn
{
  template <typename T>
  using ptr = std::shared_ptr<T>;

  template <typename T>
  using ptrs = std::vector<std::shared_ptr<T>>;

  using position = yy::position;

  struct Visitor;
  struct Exp;

  struct ID
  {
    std::string id;
    position pos;

    ID(std::string id, position pos);

    virtual void accept(Visitor &visitor);
  };

  struct Record
  {
    ptr<ID> name;
    ptr<Exp> value;
    position pos;

    Record(
        ptr<ID> name,
        ptr<Exp> value,
        position pos);

    virtual void accept(Visitor &visitor);
  };

  struct Var
  {
    position pos;

    Var(position pos);

    friend Visitor;
    virtual void accept(Visitor &visitor) = 0;
  };

  struct SimpleVar : Var
  {
    ptr<ID> name;

    SimpleVar(ptr<ID> name, position pos);

    void accept(Visitor &visitor) override;
  };

  struct FieldVar : Var
  {
    ptr<Var> var;
    ptr<ID> field;

    FieldVar(ptr<Var> var, ptr<ID> field, position pos);

    void accept(Visitor &visitor) override;
  };

  struct SubscriptVar : Var
  {
    ptr<Var> var;
    ptr<Exp> subscript;

    SubscriptVar(ptr<Var> var, ptr<Exp> subscript, position pos);

    void accept(Visitor &visitor) override;
  };

  struct Field
  {
    ptr<ID> name;
    ptr<ID> type_id;
    position pos;
    bool escape = false;

    Field(
        ptr<ID> name,
        ptr<ID> type_id,
        position pos);

    virtual void accept(Visitor &visitor);
  };

  struct Type
  {
    position pos;

    Type(position pos);

    virtual void accept(Visitor &visitor) = 0;
  };

  struct NamedType : Type
  {
    ptr<ID> named;

    NamedType(ptr<ID> named, position pos);

    void accept(Visitor &visitor) override;
  };

  struct ArrayType : Type
  {
    ptr<ID> array;

    ArrayType(ptr<ID> array, position pos);

    void accept(Visitor &visitor) override;
  };

  struct RecordType : Type
  {
    ptrs<Field> fields;

    RecordType(ptrs<Field> fields, position pos);

    void accept(Visitor &visitor) override;
  };

  struct Dec
  {
    position pos;

    Dec(position pos);

    virtual void accept(Visitor &visitor) = 0;
  };

  struct TypeDec : Dec
  {
    ptr<ID> type_id;
    ptr<Type> type;

    TypeDec(
        ptr<ID> type_id,
        ptr<Type> type,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct VarDec : Dec
  {
    ptr<ID> var;
    ptr<ID> type_id;
    ptr<Exp> exp;
    bool escape = false;

    VarDec(
        ptr<ID> var,
        ptr<ID> type_id,
        ptr<Exp> exp,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct FunctionDec : Dec
  {
    ptr<ID> funcname;
    ptrs<Field> parameters;
    ptr<ID> return_type;
    ptr<Exp> body;

    FunctionDec(
        ptr<ID> funcname,
        ptrs<Field> parameters,
        ptr<ID> return_type,
        ptr<Exp> body,
        position pos);

    void accept(Visitor &visitor) override;
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
    position pos;

    Exp(position pos);

    friend Visitor;
    virtual void accept(Visitor &visitor) = 0;
  };

  struct Nil : Exp
  {
    Nil(position pos);

    void accept(Visitor &visitor) override;
  };

  struct Int : Exp
  {
    int value;

    Int(int value, position pos);

    void accept(Visitor &visitor) override;
  };

  struct String : Exp
  {
    std::string value;

    String(std::string value, position pos);

    void accept(Visitor &visitor) override;
  };

  struct VarExp : Exp
  {
    ptr<Var> var;

    VarExp(ptr<Var> var, position pos);

    void accept(Visitor &visitor) override;
  };

  struct Assign : Exp
  {
    ptr<Var> var;
    ptr<Exp> exp;

    Assign(
        ptr<Var> var,
        ptr<Exp> exp,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct Seq : Exp
  {
    ptrs<Exp> seq;

    Seq(ptrs<Exp> seq, position pos);

    void accept(Visitor &visitor) override;
  };

  struct Call : Exp
  {
    ptr<ID> func;
    ptrs<Exp> args;

    Call(
        ptr<ID> func,
        ptrs<Exp> args,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct BinOp : Exp
  {
    ptr<Exp> rhs;
    ptr<Exp> lhs;
    Oper op;

    BinOp(
        ptr<Exp> lhs,
        ptr<Exp> rhs,
        Oper op,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct RecordExp : Exp
  {
    ptr<ID> type_id;
    ptrs<Record> records;

    RecordExp(
        ptr<ID> type_id,
        ptrs<Record> records,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct Array : Exp
  {
    ptr<ID> type_id;
    ptr<Exp> capacity;
    ptr<Exp> element;

    Array(
        ptr<ID> type_id,
        ptr<Exp> capacity,
        ptr<Exp> element,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct If : Exp
  {
    ptr<Exp> condition;
    ptr<Exp> then;
    ptr<Exp> els;

    If(
        ptr<Exp> condition,
        ptr<Exp> then,
        ptr<Exp> els,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct While : Exp
  {
    ptr<Exp> condition;
    ptr<Exp> body;

    While(
        ptr<Exp> condition,
        ptr<Exp> body,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct For : Exp
  {
    ptr<ID> var;
    ptr<Exp> from;
    ptr<Exp> to;
    ptr<Exp> body;
    bool escape = false;

    For(
        ptr<ID> var,
        ptr<Exp> from,
        ptr<Exp> to,
        ptr<Exp> body,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct Break : Exp
  {
    Break(position pos);

    void accept(Visitor &visitor) override;
  };

  struct Let : Exp
  {
    ptrs<Dec> decs;
    ptr<Exp> body;

    Let(
        ptrs<Dec> decs,
        ptr<Exp> body,
        position pos);

    void accept(Visitor &visitor) override;
  };

  struct Visitor
  {
    virtual void visit(Nil &n) = 0;
    virtual void visit(Int &i) = 0;
    virtual void visit(String &s) = 0;
    virtual void visit(VarExp &var) = 0;
    virtual void visit(Assign &assign) = 0;
    virtual void visit(Seq &seq) = 0;
    virtual void visit(Call &call) = 0;
    virtual void visit(BinOp &bin) = 0;
    virtual void visit(RecordExp &record) = 0;
    virtual void visit(Array &array) = 0;
    virtual void visit(If &iff) = 0;
    virtual void visit(While &whil) = 0;
    virtual void visit(For &forr) = 0;
    virtual void visit(Break &brk) = 0;
    virtual void visit(Let &let) = 0;
    virtual void visit(SimpleVar &var) = 0;
    virtual void visit(FieldVar &field) = 0;
    virtual void visit(SubscriptVar &subscript) = 0;
    virtual void visit(ID &id) = 0;
    virtual void visit(Record &record) = 0;
    virtual void visit(Field &field) = 0;
    virtual void visit(NamedType &named) = 0;
    virtual void visit(ArrayType &arrayType) = 0;
    virtual void visit(RecordType &recordType) = 0;
    virtual void visit(TypeDec &typeDec) = 0;
    virtual void visit(VarDec &varDec) = 0;
    virtual void visit(FunctionDec &funcDec) = 0;
  };

  struct Printer : Visitor
  {
    std::ostream &out;

    Printer(std::ostream &out);

    void visit(Nil &n) override;
    void visit(Int &i) override;
    void visit(String &s) override;
    void visit(VarExp &var) override;
    void visit(Assign &assign) override;
    void visit(Seq &seq) override;
    void visit(Call &call) override;
    void visit(BinOp &bin) override;
    void visit(RecordExp &record) override;
    void visit(Array &array) override;
    void visit(If &iff) override;
    void visit(While &whil) override;
    void visit(For &forr) override;
    void visit(Break &brk) override;
    void visit(Let &let) override;
    void visit(SimpleVar &var) override;
    void visit(FieldVar &field) override;
    void visit(SubscriptVar &subscript) override;
    void visit(ID &id) override;
    void visit(Record &record) override;
    void visit(Field &field) override;
    void visit(NamedType &named) override;
    void visit(ArrayType &arrayType) override;
    void visit(RecordType &recordType) override;
    void visit(TypeDec &typeDec) override;
    void visit(VarDec &varDec) override;
    void visit(FunctionDec &funcDec) override;
  };
}
