#include <memory>
#include <regex>
#include "absyn.h"
#include "utils.h"
#include "codegen.h"

using namespace std;
using namespace absyn;

ID::ID(std::string id, position pos) : id(id), pos(pos) {}

static string escape(string s)
{
    string res = s;
    res = regex_replace(res, regex("\a"), "\\a");
    res = regex_replace(res, regex("\b"), "\\b");
    res = regex_replace(res, regex("\f"), "\\f");
    res = regex_replace(res, regex("\n"), "\\n");
    res = regex_replace(res, regex("\r"), "\\r");
    res = regex_replace(res, regex("\t"), "\\t");
    res = regex_replace(res, regex("\v"), "\\v");
    res = regex_replace(res, regex("\\\\"), "\\\\");
    res = regex_replace(res, regex("\'"), "\\\'");
    res = regex_replace(res, regex("\""), "\\\"");
    return res;
}

void ID::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Record::Record(
    ptr<ID> name,
    ptr<Exp> value,
    position pos)
{
    this->name = name;
    this->value = value;
    this->pos = pos;
}

void Record::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Var::Var(position pos) : pos(pos) {}

SimpleVar::SimpleVar(ptr<ID> name, position pos) : Var::Var(pos)
{
    this->name = name;
}

void SimpleVar::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

FieldVar::FieldVar(ptr<Var> var, ptr<ID> field, position pos) : Var::Var(pos)
{
    this->var = var;
    this->field = field;
}

void FieldVar::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

SubscriptVar::SubscriptVar(ptr<Var> var, ptr<Exp> subscript, position pos) : Var::Var(pos)
{
    this->var = var;
    this->subscript = subscript;
}

void SubscriptVar::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Field::Field(
    ptr<ID> name,
    ptr<ID> type_id,
    position pos)
{
    this->name = name;
    this->type_id = type_id;
    this->pos = pos;
}

void Field::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Type::Type(position pos) : pos(pos) {}

NamedType::NamedType(ptr<ID> named, position pos) : Type::Type(pos)
{
    this->named = named;
}

void NamedType::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

ArrayType::ArrayType(ptr<ID> array, position pos) : Type::Type(pos)
{
    this->array = array;
}

void ArrayType::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

RecordType::RecordType(ptrs<Field> fields, position pos) : Type::Type(pos)
{
    this->fields = fields;
}

void RecordType::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Dec::Dec(position pos) : pos(pos) {}

TypeDec::TypeDec(
    ptr<ID> type_id,
    ptr<Type> type,
    position pos) : Dec::Dec(pos)
{
    this->type_id = type_id;
    this->type = type;
}

void TypeDec::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

VarDec::VarDec(
    ptr<ID> var,
    ptr<ID> type_id,
    ptr<Exp> exp,
    position pos) : Dec::Dec(pos)
{
    this->var = var;
    this->type_id = type_id;
    this->exp = exp;
}

void VarDec::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

FunctionDec::FunctionDec(
    ptr<ID> funcname,
    ptrs<Field> parameters,
    ptr<ID> return_type,
    ptr<Exp> body,
    position pos)
    : Dec::Dec(pos)
{
    this->funcname = funcname;
    this->parameters = parameters;
    this->return_type = return_type;
    this->body = body;
}

void FunctionDec::accept(Visitor &visitor)
{
    visitor.visit(*this);
}

Exp::Exp(position pos) : pos(pos) {}

Nil::Nil(position pos) : Exp::Exp(pos) {}

void Nil::accept(Visitor &v)
{
    v.visit(*this);
}

Int::Int(int value, position pos) : Exp::Exp(pos), value(value) {}

void Int::accept(Visitor &v)
{
    v.visit(*this);
}

String::String(std::string value, position pos) : Exp::Exp(pos), value(value) {}

void String::accept(Visitor &v)
{
    v.visit(*this);
}

VarExp::VarExp(ptr<Var> var, position pos) : Exp::Exp(pos), var(var) {}

void VarExp::accept(Visitor &v)
{
    v.visit(*this);
}

Assign::Assign(
    ptr<Var> var,
    ptr<Exp> exp,
    position pos) : Exp::Exp(pos), var(var), exp(exp) {}

void Assign::accept(Visitor &v)
{
    v.visit(*this);
}

Seq::Seq(ptrs<Exp> seq, position pos) : Exp::Exp(pos), seq(seq) {}

void Seq::accept(Visitor &v)
{
    v.visit(*this);
}

Call::Call(
    ptr<ID> func,
    ptrs<Exp> args,
    position pos) : Exp::Exp(pos), func(func), args(args) {}

void Call::accept(Visitor &v)
{
    v.visit(*this);
}

BinOp::BinOp(
    ptr<Exp> lhs,
    ptr<Exp> rhs,
    Oper op,
    position pos) : Exp::Exp(pos), lhs(lhs), rhs(rhs), op(op) {}

void BinOp::accept(Visitor &v)
{
    v.visit(*this);
}

RecordExp::RecordExp(
    ptr<ID> type_id,
    ptrs<Record> records,
    position pos) : Exp::Exp(pos), type_id(type_id), records(records) {}

void RecordExp::accept(Visitor &v)
{
    v.visit(*this);
}

Array::Array(
    ptr<ID> type_id,
    ptr<Exp> capacity,
    ptr<Exp> element,
    position pos) : Exp::Exp(pos)
{
    this->type_id = type_id;
    this->capacity = capacity;
    this->element = element;
}

void Array::accept(Visitor &v)
{
    v.visit(*this);
}

If::If(
    ptr<Exp> condition,
    ptr<Exp> then,
    ptr<Exp> els,
    position pos) : Exp::Exp(pos)
{
    this->condition = condition;
    this->then = then;
    this->els = els;
}

void If::accept(Visitor &v)
{
    v.visit(*this);
}

While::While(
    ptr<Exp> condition,
    ptr<Exp> body,
    position pos) : Exp::Exp(pos)
{
    this->condition = condition;
    this->body = body;
}

void While::accept(Visitor &v)
{
    v.visit(*this);
}

For::For(
    ptr<ID> var,
    ptr<Exp> from,
    ptr<Exp> to,
    ptr<Exp> body,
    position pos) : Exp::Exp(pos)
{
    this->var = var;
    this->from = from;
    this->to = to;
    this->body = body;
}

void For::accept(Visitor &v)
{
    v.visit(*this);
}

Break::Break(position pos) : Exp::Exp(pos) {}

void Break::accept(Visitor &v)
{
    v.visit(*this);
}

Let::Let(
    ptrs<Dec> decs,
    ptr<Exp> body,
    position pos) : Exp::Exp(pos)
{
    this->decs = decs;
    this->body = body;
}

void Let::accept(Visitor &v)
{
    v.visit(*this);
}

/// Implementations of Printer

Printer::Printer(std::ostream &out) : out(out) {}

void Printer::visit(Nil &n)
{
    this->out << "null";
}

void Printer::visit(Int &i)
{
    this->out << i.value;
}

void Printer::visit(String &s)
{
    this->out << "\"" << escape(s.value) << "\"";
}

void Printer::visit(VarExp &var)
{
    var.var->accept(*this);
}

void Printer::visit(Assign &assign)
{
    this->out << "{\"type\":\"Assign\",\"var\":";
    assign.var->accept(*this);
    this->out << ",\"exp\":";
    assign.exp->accept(*this);
    this->out << "}";
}

void Printer::visit(Seq &seq)
{
    this->out << "[";
    for (auto iter = seq.seq.begin(); iter != seq.seq.end(); ++iter)
    {
        (*iter)->accept(*this);
        if (iter != seq.seq.end() - 1)
            this->out << ", ";
    }
    this->out << "]";
}

void Printer::visit(Call &call)
{
    this->out << "{\"type\":\"Call\",\"func\":";
    call.func->accept(*this);
    if (!call.args.empty())
    {
        this->out << ",\"args\":[";
        for (auto iter = call.args.begin(); iter != call.args.end(); ++iter)
        {
            (*iter)->accept(*this);
            if (iter != call.args.end() - 1)
                this->out << ", ";
        }
        this->out << "]";
    }
    this->out << "}";
}

void Printer::visit(BinOp &bin)
{
    this->out << "{\"type\":\"BinOp\",\"op\":\"";
    switch (bin.op)
    {
    case Oper::plusOp:
        this->out << "ADD";
        break;
    case Oper::minusOp:
        this->out << "SUB";
        break;
    case Oper::timesOp:
        this->out << "MUL";
        break;
    case Oper::divideOp:
        this->out << "DIV";
        break;
    case Oper::eqOp:
        this->out << "EQ";
        break;
    case Oper::neqOp:
        this->out << "NE";
        break;
    case Oper::ltOp:
        this->out << "LT";
        break;
    case Oper::leOp:
        this->out << "LE";
        break;
    case Oper::gtOp:
        this->out << "GT";
        break;
    case Oper::geOp:
        this->out << "GE";
        break;
    }
    this->out << "\",\"lhs\":";
    bin.lhs->accept(*this);
    this->out << ",\"rhs\":";
    bin.rhs->accept(*this);
    this->out << "}";
}

void Printer::visit(RecordExp &record)
{
    this->out << "{\"type\":\"RecordExp\",\"type_id\":";
    record.type_id->accept(*this);
    if (!record.records.empty())
    {
        this->out << ",\"records\":[";
        for (auto iter = record.records.begin(); iter != record.records.end(); ++iter)
        {
            (*iter)->accept(*this);
            if (iter != record.records.end() - 1)
                this->out << ",";
        }
        this->out << "]";
    }
    this->out << "}";
}

void Printer::visit(Array &array)
{
    this->out << "{\"type\":\"Array\",\"capacity\":";
    array.capacity->accept(*this);
    this->out << ",\"element\":";
    array.element->accept(*this);
    this->out << "}";
}

void Printer::visit(If &iff)
{
    this->out << "{\"type\":\"If\",\"condition\":";
    iff.condition->accept(*this);
    this->out << ",\"then\":";
    iff.then->accept(*this);
    if (iff.els)
    {
        this->out << ",\"else\":";
        iff.els->accept(*this);
    }
    this->out << "}";
}

void Printer::visit(While &whil)
{
    this->out << "{\"type\":\"While\",\"condition\":";
    whil.condition->accept(*this);
    this->out << ",\"body\":";
    whil.body->accept(*this);
    this->out << "}";
}

void Printer::visit(For &forr)
{
    this->out << "{\"type\":\"For\",\"var\":";
    forr.var->accept(*this);
    this->out << ",\"from\":";
    forr.from->accept(*this);
    this->out << ",\"to\":";
    forr.to->accept(*this);
    this->out << ",\"body\":";
    forr.body->accept(*this);
    this->out << "}";
}

void Printer::visit(Break &brk)
{
    this->out << "{\"type\":\"Break\"}" << endl;
}

void Printer::visit(Let &let)
{
    this->out << "{\"type\":\"Let\"";
    if (!let.decs.empty())
    {
        this->out << ",\"decs\":[";
        for (auto iter = let.decs.begin(); iter != let.decs.end(); ++iter)
        {
            (*iter)->accept(*this);
            if (iter != let.decs.end() - 1)
                this->out << ",";
        }
        this->out << "]";
    }
    this->out << ",\"body\":";
    let.body->accept(*this);
    this->out << "}";
}

void Printer::visit(SimpleVar &var)
{
    this->out << "{\"type\":\"simpleVar\",\"name\":";
    var.name->accept(*this);
    this->out << "}";
}

void Printer::visit(FieldVar &field)
{
    this->out << "{\"type\":\"FieldVar\",\"var\":";
    field.var->accept(*this);
    this->out << ",\"field\":";
    field.field->accept(*this);
    this->out << "}";
}

void Printer::visit(SubscriptVar &subscript)
{
    this->out << "{\"type\":\"SubscriptVar\",\"var\":";
    subscript.var->accept(*this);
    this->out << ",\"subscript\":";
    subscript.subscript->accept(*this);
    this->out << "}";
}

void Printer::visit(ID &id)
{
    this->out << "\"" << id.id << "\"";
}

void Printer::visit(Record &record)
{
    this->out << "{\"type\":\"Record\",\"name\":";
    record.name->accept(*this);
    this->out << ",\"value\":";
    record.value->accept(*this);
    this->out << "}";
}

void Printer::visit(Field &field)
{
    this->out << "{\"type\":\"Field\",\"name\":";
    field.name->accept(*this);
    this->out << ",\"type_id\":";
    field.type_id->accept(*this);
    this->out << "}";
}

void Printer::visit(NamedType &named)
{
    this->out << "{\"type\":\"NamedType\",\"named\":";
    named.named->accept(*this);
    this->out << "}";
}

void Printer::visit(ArrayType &arrayType)
{
    this->out << "{\"type\":\"ArrayType\",\"array\":";
    arrayType.array->accept(*this);
    this->out << "}";
}

void Printer::visit(RecordType &recordType)
{
    this->out << "{\"type\":\"RecordType\",\"fields\":[";
    for (auto iter = recordType.fields.begin(); iter != recordType.fields.end(); ++iter)
    {
        (*iter)->accept(*this);
        if (iter != recordType.fields.end() - 1)
            this->out << ",";
    }
    this->out << "]}";
}

void Printer::visit(TypeDec &typeDec)
{
    this->out << "{\"type\":\"TypeDec\",\"type_id\":";
    typeDec.type_id->accept(*this);
    this->out << ",\"type\":";
    typeDec.type->accept(*this);
    this->out << "}";
}

void Printer::visit(VarDec &varDec)
{
    this->out << "{\"type\":\"VarDec\",\"var\":";
    varDec.var->accept(*this);
    if (varDec.type_id)
    {
        this->out << ",\"type_id\":";
        varDec.type_id->accept(*this);
    }
    this->out << ",\"exp\":";
    varDec.exp->accept(*this);
    this->out << "}";
}

void Printer::visit(FunctionDec &funcDec)
{
    this->out << "{\"type\":\"FunctionDec\",\"funcname\":";
    funcDec.funcname->accept(*this);
    if (funcDec.return_type)
    {
        this->out << ",\"return_type\":";
        funcDec.return_type->accept(*this);
    }
    this->out << ",\"parameters\":[";
    for (auto iter = funcDec.parameters.begin(); iter != funcDec.parameters.end(); ++iter)
    {
        (*iter)->accept(*this);
        if (iter != funcDec.parameters.end() - 1)
            this->out << ",";
    }
    this->out << "],\"body\":";
    funcDec.body->accept(*this);
    this->out << "}";
}

bool absyn::isRelOp(Oper op)
{
    switch (op)
    {
    case Oper::eqOp:
    case Oper::neqOp:
    case Oper::ltOp:
    case Oper::leOp:
    case Oper::gtOp:
    case Oper::geOp:
        return true;
    default:
        return false;
    }
}

bool absyn::isArithOp(Oper op)
{
    switch (op)
    {
    case Oper::plusOp:
    case Oper::minusOp:
    case Oper::timesOp:
    case Oper::divideOp:
        return true;
    default:
        return false;
    }
}

/// Accept function serial of AbstractCodeGenerator.

cg::TyValue Nil::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Int::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue String::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue VarExp::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Assign::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Seq::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Call::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue BinOp::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue RecordExp::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Array::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue If::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue While::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue For::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Break::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue Let::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue SimpleVar::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue FieldVar::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue SubscriptVar::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue TypeDec::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue VarDec::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
cg::TyValue FunctionDec::accept(cg::AbstractCodeGenerator &generator) { return generator.visit(*this); }
