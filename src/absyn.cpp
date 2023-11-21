#include <memory>
#include "absyn.h"
#include "utils.h"

using namespace std;
using namespace absyn;

ID::ID(std::string id, position pos) : id(id), pos(pos) {}

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

Printer::Printer(std::ostream &out, int depth, bool isInline) : out(out)
{
    this->depth = depth;
    this->isInline = isInline;
}

void Printer::visit(Nil &n)
{
    this->out << "Nil";
}

void Printer::visit(Int &i)
{
    this->out << "Int<";
    this->out << i.value;
    this->out << ">";
}

void Printer::visit(String &s)
{
    this->out << "String<";
    this->out << s.value;
    this->out << ">";
}

void Printer::visit(VarExp &var)
{
    var.var->accept(*this);
}

void Printer::visit(Assign &assign)
{
    this->out << "Assign<";
    assign.var->accept(*this);
    this->out << ", ";
    assign.exp->accept(*this);
    this->out << ">";
}

void Printer::visit(Seq &seq)
{
    for (auto iter = seq.seq.begin(); iter != seq.seq.end(); ++iter)
    {
        this->depth++;
        (*iter)->accept(*this);
        this->depth--;
        if (iter != seq.seq.end() - 1)
            this->out << endl;
    }
}

void Printer::visit(Call &call)
{
    this->out << "Call<";
    call.func->accept(*this);
    for (auto iter = call.args.begin(); iter != call.args.end(); ++iter)
    {
        this->out << ", ";
        (*iter)->accept(*this);
    }
    this->out << ">";
}

void Printer::visit(BinOp &bin)
{
    switch (bin.op)
    {
    case plusOp:
        this->out << "ADD";
        break;
    case minusOp:
        this->out << "SUB";
        break;
    case timesOp:
        this->out << "MUL";
        break;
    case divideOp:
        this->out << "DIV";
        break;
    case eqOp:
        this->out << "EQ";
        break;
    case neqOp:
        this->out << "NE";
        break;
    case ltOp:
        this->out << "LT";
        break;
    case leOp:
        this->out << "LE";
        break;
    case gtOp:
        this->out << "GT";
        break;
    case geOp:
        this->out << "GE";
        break;
    }

    this->out << "<";
    bin.lhs->accept(*this);
    this->out << ", ";
    bin.rhs->accept(*this);
    this->out << ">";
}

void Printer::visit(RecordExp &record)
{
    this->out << "RecordExp<";
    record.type_id->accept(*this);
    for (auto rcd : record.records)
    {
        this->out << ",";
        rcd->accept(*this);
    }
    this->out << ">";
}

void Printer::visit(Array &array)
{
    this->out << "Array<";
    array.capacity->accept(*this);
    this->out << ", ";
    array.element->accept(*this);
    this->out << ">";
}

void Printer::visit(If &iff)
{
    this->out << "If<";
    iff.condition->accept(*this);
    this->out << ", ";
    iff.then->accept(*this);
    if (iff.els)
    {
        this->out << ", ";
        iff.els->accept(*this);
    }
    this->out << ">";
}

void Printer::visit(While &whil)
{
    this->out << "While<";
    whil.condition->accept(*this);
    this->out << ", ";
    whil.body->accept(*this);
    this->out << ">";
}

void Printer::visit(For &forr)
{
    this->out << "For<";
    forr.var->accept(*this);
    this->out << ",";
    forr.from->accept(*this);
    this->out << ",";
    forr.to->accept(*this);
    this->out << ">" << endl;
    this->depth++;
    forr.body->accept(*this);
    this->depth--;
}

void Printer::visit(Break &brk)
{
    this->out << "Break";
}

void Printer::visit(Let &let)
{
    printIndent();
    this->out << "Let" << endl;
    for (auto dec : let.decs)
    {
        this->depth++;
        printIndent();
        dec->accept(*this);
        this->depth--;
        this->out << endl;
    }
    printIndent();
    this->out << "IN" << endl;
    this->depth++;
    let.body->accept(*this);
    this->depth--;
    this->out << endl;
    printIndent();
    this->out << "END";
}

void Printer::visit(SimpleVar &var)
{
    this->out << "Var<";
    var.name->accept(*this);
    this->out << ">";
}

void Printer::visit(FieldVar &field)
{
    this->out << "Field<";
    field.var->accept(*this);
    this->out << ", ";
    field.field->accept(*this);
    this->out << ">";
}

void Printer::visit(SubscriptVar &subscript)
{
    this->out << "Subscript<";
    subscript.var->accept(*this);
    this->out << ", ";
    subscript.subscript->accept(*this);
    this->out << ">";
}

void Printer::visit(ID &id)
{
    this->out << id.id;
}

void Printer::visit(Record &record)
{
    this->out << "Record<";
    record.name->accept(*this);
    this->out << ",";
    record.value->accept(*this);
    this->out << ">";
}

void Printer::visit(Field &field)
{
    this->out << "Field: ";
    field.name->accept(*this);
    this->out << "<";
    field.type_id->accept(*this);
    this->out << ">";
}

void Printer::visit(NamedType &named)
{
    this->out << "<";
    named.named->accept(*this);
    this->out << ">";
}

void Printer::visit(ArrayType &arrayType)
{
    this->out << "Array<";
    arrayType.array->accept(*this);
    this->out << "Array>";
}

void Printer::visit(RecordType &recordType)
{
    this->out << "Record<";
    for (auto field : recordType.fields)
    {
        field->accept(*this);
    }
    this->out << ">";
}

void Printer::visit(TypeDec &typeDec)
{
    this->out << "@Type<";
    typeDec.type_id->accept(*this);
    this->out << ", ";
    typeDec.type->accept(*this);
    this->out << ">";
}

void Printer::visit(VarDec &varDec)
{
    this->out << "@Var<";
    varDec.var->accept(*this);
    if (varDec.type_id)
    {
        this->out << ", ";
        varDec.type_id->accept(*this);
    }
    this->out << ", ";
    varDec.exp->accept(*this);
    this->out << ">";
}

void Printer::visit(FunctionDec &funcDec)
{
    this->out << "@Function<";
    funcDec.funcname->accept(*this);
    if (funcDec.return_type)
    {
        this->out << ":";
        funcDec.return_type->accept(*this);
    }
    for (auto param : funcDec.parameters)
    {
        this->out << ", ";
        param->accept(*this);
    }
    this->out << ">" << endl;
    this->depth++;
    funcDec.body->accept(*this);
    this->depth--;
}

void Printer::printIndent()
{
    if (isInline || depth == 0)
        return;

    for (int i = 0; i < depth; ++i)
    {
        this->out << indent;
    }
}

void Printer::changeLineOrSeparate(std::string sep)
{
    if (isInline)
    {
        this->out << sep;
    }
    else
    {
        this->out << endl;
    }
}
