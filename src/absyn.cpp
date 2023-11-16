#include <memory>
#include "absyn.h"

using namespace std;
using namespace absyn;

ID::ID(std::string id, yy::position pos) : id(id), pos(pos) {}
std::ostream &ID::print(std::ostream &o, int depth, bool isInline)
{
    return o << this->id;
}

Record::Record(
    ID *name,
    Exp *value,
    yy::position pos)
{
    this->name = shared_ptr<ID>(name);
    this->value = shared_ptr<Exp>(value);
    this->pos = pos;
}

Var::Var(yy::position pos) : pos(pos) {}

std::ostream &Var::print(std::ostream &o, int depth, bool isInline)
{
    return o;
}

Field::Field(
    ID *name,
    ID *type_id,
    yy::position pos,
    bool escape)
{
    this->name = shared_ptr<ID>(name);
    this->type_id = shared_ptr<ID>(type_id);
    this->pos = pos;
    this->escape = escape;
}

Type::Type(yy::position pos) : pos(pos) {}
std::ostream &Type::print(std::ostream &o, int depth, bool isInline)
{
    return o;
}

NamedType::NamedType(ID *named, yy::position pos) : Type::Type(pos)
{
    this->named = shared_ptr<ID>(named);
}

ArrayType::ArrayType(ID *array, yy::position pos) : Type::Type(pos)
{
    this->array = shared_ptr<ID>(array);
}

RecordType::RecordType(std::vector<Field> fields, yy::position pos) : Type::Type(pos), fields(fields) {}

Dec::Dec(yy::position pos) : pos(pos) {}

std::ostream &Dec::print(std::ostream &o, int depth, bool isInline)
{
    return o;
}

TypeDec::TypeDec(
    ID *type_id,
    Type *type,
    yy::position pos) : Dec::Dec(pos)
{
    this->type_id = shared_ptr<ID>(type_id);
    this->type = shared_ptr<Type>(type);
}

VarDec::VarDec(ID *var, ID *type_id, Exp *exp, bool escape, yy::position pos) : Dec::Dec(pos)
{
    this->var = shared_ptr<ID>(var);
    this->type_id = shared_ptr<ID>(type_id);
    this->exp = shared_ptr<Exp>(exp);
    this->escape = escape;
}

FunctionDec::FunctionDec(
    ID *funcname,
    std::vector<Field> parameters,
    ID *return_type,
    Exp *body,
    yy::position pos)
    : Dec::Dec(pos)
{
    this->funcname = shared_ptr<ID>(funcname);
    this->parameters = parameters;
    this->return_type = shared_ptr<ID>(return_type);
    this->body = shared_ptr<Exp>(body);
}

Exp::Exp(yy::position pos) : pos(pos) {}

std::ostream &Exp::print(std::ostream &o, int depth, bool isInline)
{
    return o;
}

Int::Int(int value, yy::position pos) : Exp::Exp(pos), value(value) {}

String::String(std::string value, yy::position pos) : Exp::Exp(pos), value(value) {}

VarExp::VarExp(Var *var, yy::position pos) : Exp::Exp(pos), var(shared_ptr<Var>(var)) {}

Assign::Assign(
    Var *var,
    Exp *exp,
    yy::position pos) : Exp::Exp(pos), var(shared_ptr<Var>(var)), exp(shared_ptr<Exp>(exp)) {}

Call::Call(
    ID *func,
    std::vector<Exp> args,
    yy::position pos) : Exp::Exp(pos), func(shared_ptr<ID>(func)), args(args) {}

BinOp::BinOp(
    Exp *rhs,
    Exp *lhs,
    Oper op,
    yy::position pos) : Exp::Exp(pos), rhs(shared_ptr<Exp>(rhs)), lhs(shared_ptr<Exp>(lhs)), op(op) {}

RecordExp::RecordExp(
    ID *type_id,
    std::vector<Record> records,
    yy::position pos) : Exp::Exp(pos), type_id(shared_ptr<ID>(type_id)), records(records) {}

Array::Array(
    ID *type_id,
    Exp *capacity,
    Exp *element,
    yy::position pos) : Exp::Exp(pos)
{
    this->type_id = shared_ptr<ID>(type_id);
    this->capacity = shared_ptr<Exp>(capacity);
    this->element = shared_ptr<Exp>(element);
}

If::If(
    Exp *condition,
    Exp *then,
    Exp *els,
    yy::position pos) : Exp::Exp(pos)
{
    this->condition = shared_ptr<Exp>(condition);
    this->then = shared_ptr<Exp>(then);
    this->els = shared_ptr<Exp>(els);
}

While::While(
    Exp *condition,
    Exp *body,
    yy::position pos) : Exp::Exp(pos)
{
    this->condition = shared_ptr<Exp>(condition);
    this->body = shared_ptr<Exp>(body);
}

For::For(
    Var *var,
    Exp *from,
    Exp *to,
    Exp *body,
    bool escape,
    yy::position pos) : Exp::Exp(pos)
{
    this->var = shared_ptr<Var>(var);
    this->from = shared_ptr<Exp>(from);
    this->to = shared_ptr<Exp>(to);
    this->body = shared_ptr<Exp>(body);
}

Let::Let(
    std::vector<Dec> decs,
    Exp *body,
    yy::position pos) : Exp::Exp(pos)
{
    this->decs = decs;
    this->body = shared_ptr<Exp>(body);
}
