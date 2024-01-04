#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Transforms/Utils.h>
#include <memory>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include "codegen.h"
#include "absyn.h"
#include "types.h"
#define _String std::make_shared<ty::String>()
#define _Int std::make_shared<ty::Int>()
#define _Void std::make_shared<ty::Void>()
#define _Func(n, rt, ...) std::make_shared<cg::FuncEnventry>(n, rt, std::vector<std::shared_ptr<ty::Type>>{__VA_ARGS__})

using namespace cg;
using namespace llvm;
using namespace absyn;
using namespace std;

TyValue::TyValue(std::shared_ptr<ty::Type> type, llvm::Value *value) : type(type), value(value) {}

CodeGenerator::CodeGenerator()
    : AbstractCodeGenerator::AbstractCodeGenerator(), namedValues(), namedTypes(), breaks(), libraryFunctionCreator()
{
  context = make_unique<LLVMContext>();
  builder = make_unique<IRBuilder<>>(*context);
  moduler = make_unique<Module>("main module", *context);
  functionPassManager = make_unique<legacy::FunctionPassManager>(moduler.get());
  functionPassManager->add(createPromoteMemoryToRegisterPass());
  functionAnalysisManager = make_unique<FunctionAnalysisManager>();

  auto mainFuncType = FunctionType::get(llvm::Type::getVoidTy(*context), false);
  auto mainFunc = Function::Create(mainFuncType, GlobalValue::LinkageTypes::ExternalLinkage, "main", this->moduler.get());
  auto entry = BasicBlock::Create(*context, "", mainFunc);
  builder->SetInsertPoint(entry);

  namedTypes.insert("int", make_shared<ty::Int>());
  namedTypes.insert("string", make_shared<ty::String>());
  namedTypes.insert("nil", make_shared<ty::Nil>());
  namedTypes.insert("void", make_shared<ty::Void>());

  namedValues.insert("print", _Func("print", _Void, _String));
  namedValues.insert("flush", _Func("flush", _Void));
  namedValues.insert("getchar", _Func("getchar", _String));
  namedValues.insert("ord", _Func("ord", _Int, _String));
  namedValues.insert("chr", _Func("chr", _String, _Int));
  namedValues.insert("size", _Func("size", _Int, _String));
  namedValues.insert("substring", _Func("substring", _String, _String, _Int, _Int));
  namedValues.insert("concat", _Func("concat", _String, _String, _String));
  namedValues.insert("not", _Func("not", _Int, _Int));
  namedValues.insert("exit", _Func("exit", _Void, _Int));
  namedValues.insert("string_compare", _Func("string_compare", _Int, _String, _String));

  for (auto iter = namedValues.top_begin(); iter != namedValues.top_end(); iter++)
  {
    auto f_enventry = dynamic_cast<FuncEnventry *>(iter->second.get());
    registeLibraryFunction(f_enventry->name, [this, f_enventry]()
                           { return createFunction(*f_enventry); });
  }

  registeLibraryFunction(
      "malloc",
      [this]()
      { return Function::Create(
            FunctionType::get(builder->getPtrTy(), {builder->getInt64Ty()}, false),
            Function::ExternalLinkage, "malloc", moduler.get()); });

  registeLibraryFunction(
      "array_initialize",
      [this]()
      {
        return Function::Create(
            FunctionType::get(
                builder->getVoidTy(),
                {builder->getPtrTy(), builder->getPtrTy(), builder->getInt64Ty(), builder->getInt64Ty()},
                false),
            Function::ExternalLinkage, "array_initialize", moduler.get());
      });
}

void CodeGenerator::registeLibraryFunction(std::string name, std::function<llvm::Function *()> factory)
{
  libraryFunctionCreator.insert({name, factory});
}

llvm::Function *CodeGenerator::requestFunction(std::string funcname)
{
  auto func = moduler->getFunction(funcname);
  if (!func)
  {
    auto found = libraryFunctionCreator.find(funcname);
    if (found != libraryFunctionCreator.end())
    {
      func = found->second();
      libraryFunctionCreator.erase(funcname);
    }
  }

  return func;
}

static const ty::Type *actualTy(const ty::Type *ty_ptr)
{
  auto curr = ty_ptr;
  while (auto named = dynamic_cast<const ty::Named *>(curr))
  {
    if (named->type == ty_ptr)
      return ty_ptr;
    else
      curr = named->type;
  }
  return curr;
}

namespace ty
{
  bool match(const ty::Type &lhs, const ty::Type &rhs)
  {
    auto ac_lhs = actualTy(&lhs);
    auto ac_rhs = actualTy(&rhs);
    if (dynamic_cast<const ty::Record *>(&lhs))
    {
      return ac_rhs->match(*ac_lhs) || ac_rhs->match(ty::Nil());
    }
    else
    {
      return ac_lhs->match(*ac_rhs);
    }
  }

  bool mismatch(const ty::Type &lhs, const ty::Type &rhs)
  {
    return !match(lhs, rhs);
  }
}

TyValue CodeGenerator::visit(Nil &n)
{
  auto value = ConstantPointerNull::get(builder->getPtrTy());
  return TyValue(make_shared<ty::Nil>(), value);
}

TyValue CodeGenerator::visit(Int &i)
{
  auto value = builder->getInt64(i.value);
  return TyValue(make_shared<ty::Int>(), value);
}

TyValue CodeGenerator::visit(String &s)
{
  auto value = builder->CreateGlobalStringPtr(StringRef(s.value));
  return TyValue(make_shared<ty::String>(), value);
}

TyValue CodeGenerator::visit(VarExp &var)
{
  auto v = var.var->accept(*this);
  return TyValue(v.type, builder->CreateLoad(type2IRType(v.type.get()), v.value));
}

TyValue CodeGenerator::visit(Assign &assign)
{
  auto var = assign.var->accept(*this);
  auto exp = assign.exp->accept(*this);

  if (!match(*var.type, *exp.type))
    fatalError("unmatched type assignment", assign.exp->pos);

  builder->CreateStore(exp.value, var.value);
  return mkVoid();
}

TyValue CodeGenerator::visit(Seq &seq)
{
  for (auto iter = seq.seq.begin(); iter != seq.seq.end(); ++iter)
  {
    auto tyValue = (*iter)->accept(*this);
    if (iter == seq.seq.end() - 1)
      return tyValue;
  }
  return mkVoid();
}

TyValue CodeGenerator::visit(Call &call)
{
  auto found = namedValues.find(call.func->id);
  if (!found)
    fatalError("function " + call.func->id + " not found", call.func->pos);

  auto f_enventry = dynamic_cast<FuncEnventry *>(found->get());
  if (!f_enventry)
    fatalError(call.func->id + " is not a function", call.func->pos);

  auto func = requestFunction(f_enventry->name);
  assert(func);

  if (call.args.size() != f_enventry->args.size())
  {
    ostringstream o;
    o << "error number of parameters excepted " << f_enventry->args.size() << " actual" << call.args.size();
    fatalError(o.str(), call.func->pos);
  }

  auto iter = f_enventry->args.begin();
  vector<llvm::Value *> params;
  for (auto arg : call.args)
  {
    auto arg_tyvalue = arg->accept(*this);
    if (mismatch(*iter->get(), *arg_tyvalue.type))
    {
      fatalError("unmatched parameter type", arg->pos);
    }
    else
    {
      params.push_back(arg_tyvalue.value);
    }
    iter++;
  }

  auto result = builder->CreateCall(func, params);
  if (f_enventry->returnType)
  {
    return TyValue(f_enventry->returnType, result);
  }
  else
  {
    return mkVoid();
  }
}

static CmpInst::Predicate op2icmp(Oper op)
{
  switch (op)
  {
  case Oper::eqOp:
    return CmpInst::ICMP_EQ;
    break;
  case Oper::neqOp:
    return CmpInst::ICMP_NE;
    break;
  case Oper::ltOp:
    return CmpInst::ICMP_SLT;
    break;
  case Oper::leOp:
    return CmpInst::ICMP_SLE;
    break;
  case Oper::gtOp:
    return CmpInst::ICMP_SGT;
    break;
  case Oper::geOp:
    return CmpInst::ICMP_SGE;
    break;
  default:
    return CmpInst::BAD_ICMP_PREDICATE;
    break;
  }
}

TyValue CodeGenerator::visit(BinOp &bin)
{
  if (isArithOp(bin.op))
  {
    auto LHS = bin.lhs->accept(*this);

    if (ty::mismatch(*LHS.type, ty::Int()))
    {
      fatalError("bad type of lhs", bin.lhs->pos);
    }

    auto RHS = bin.rhs->accept(*this);
    if (ty::mismatch(*RHS.type, ty::Int()))
    {
      fatalError("bad type of rhs", bin.rhs->pos);
    }

    Value *result = nullptr;

    switch (bin.op)
    {
    case Oper::plusOp:
      result = builder->CreateAdd(LHS.value, RHS.value, "plustemp");
      break;
    case Oper::minusOp:
      result = builder->CreateSub(LHS.value, RHS.value, "minustemp");
      break;
    case Oper::timesOp:
      result = builder->CreateMul(LHS.value, RHS.value, "timestemp");
      break;
    case Oper::divideOp:
      result = builder->CreateSDiv(LHS.value, RHS.value, "dividetemp");
      break;
    default:
      assert(0 && "bad operator");
    }

    return TyValue(make_shared<ty::Int>(), result);
  }
  else
  {
    assert(isRelOp(bin.op));
    auto LHS = bin.lhs->accept(*this);
    if (match(*LHS.type, ty::String()))
    {
      auto RHS = bin.rhs->accept(*this);
      if (match(*RHS.type, ty::String()))
      {
        auto func = requestFunction("string_compare");
        auto cmp = builder->CreateCall(func, {LHS.value, RHS.value});
        auto b = builder->CreateICmp(op2icmp(bin.op), cmp, builder->getInt64(0));
        return TyValue(make_shared<ty::Int>(), builder->CreateIntCast(b, builder->getInt64Ty(), false));
      }
      else
      {
        // "textcontent compare to other type" >= 42
        fatalError("unmatched type", bin.rhs->pos);
      }
    }
    else if (match(*LHS.type, ty::Int()))
    {
      auto RHS = bin.rhs->accept(*this);
      if (match(*RHS.type, ty::Int()))
      {
        auto b = builder->CreateICmp(op2icmp(bin.op), LHS.value, RHS.value);
        return TyValue(make_shared<ty::Int>(), builder->CreateIntCast(b, builder->getInt64Ty(), false));
      }
      else
      {
        fatalError("unmatched type", bin.rhs->pos);
      }
    }
    else if (dynamic_cast<const ty::Array *>(actualTy(LHS.type.get())) || dynamic_cast<const ty::Record *>(actualTy(LHS.type.get())))
    {
      if (bin.op == Oper::eqOp || bin.op == Oper::neqOp)
      {
        auto RHS = bin.rhs->accept(*this);
        if (match(*LHS.type, *RHS.type))
        {
          auto li = builder->CreatePtrToInt(LHS.value, builder->getInt64Ty());
          auto ri = builder->CreatePtrToInt(RHS.value, builder->getInt64Ty());
          auto b = builder->CreateICmp(op2icmp(bin.op), li, ri);
          return TyValue(make_shared<ty::Int>(), builder->CreateIntCast(b, builder->getInt64Ty(), false));
        }
        else
        {
          fatalError("unmatched type", bin.rhs->pos);
        }
      }
      else
      {
        fatalError("bad operator for reference type", bin.pos);
      }
    }
    else
    {
      fatalError("bad type to compare", bin.lhs->pos);
    }
  }
}

TyValue CodeGenerator::visit(RecordExp &record)
{
  auto found_type = namedTypes.find(record.type_id->id);
  if (!found_type)
    fatalError("type " + record.type_id->id + " is undefined", record.pos);

  auto ty = actualTy(found_type->get());
  auto record_ty = dynamic_cast<const ty::Record *>(ty);
  if (!record_ty)
    fatalError("type " + record.type_id->id + " is not a record type", record.pos);

  auto struct_ty = StructType::getTypeByName(*context, record.type_id->id);
  assert(struct_ty);
  auto _malloc = requestFunction("malloc");
  assert(_malloc);
  auto sz = moduler->getDataLayout().getTypeAllocSize(struct_ty);
  auto value = builder->CreateCall(_malloc, {builder->CreateTypeSize(builder->getInt64Ty(), sz)});

  for (auto &rcd : record.records)
  {
    auto rcd_ty = record_ty->records.find(rcd->name->id);
    if (rcd_ty == record_ty->records.end())
    {
      fatalError("field " + rcd->name->id + " is undefined", rcd->pos);
    }
    auto record_value = rcd->value->accept(*this);
    if (match(*rcd_ty->second, *record_value.type))
    {
      auto offset = distance(record_ty->records.begin(), rcd_ty);
      auto record_ptr = builder->CreateStructGEP(struct_ty, value, offset);
      builder->CreateStore(record_value.value, record_ptr);
    }
    else
    {
      fatalError("type of field " + rcd->name->id + " is not matched", rcd->value->pos);
    }
  }

  return TyValue(make_shared<ty::Record>(*record_ty), value);
}

TyValue CodeGenerator::visit(Array &array)
{
  auto found_type = namedTypes.find(array.type_id->id);
  if (!found_type)
    fatalError("type " + array.type_id->id + " is undefined", array.pos);

  auto ty = actualTy(found_type->get());
  auto array_ty = dynamic_cast<const ty::Array *>(ty);
  if (!array_ty)
    fatalError("type " + array.type_id->id + " is not a array type", array.pos);

  auto CAPACITY = array.capacity->accept(*this);
  if (mismatch(*CAPACITY.type, ty::Int()))
    fatalError("capacity of array must be int type", array.capacity->pos);

  auto ELEMENT = array.element->accept(*this);
  if (mismatch(*array_ty->type, *ELEMENT.type))
    fatalError("type of element is not matched", array.element->pos);

  auto elem_ir_ty = type2IRType(array_ty->type);
  auto sz = moduler->getDataLayout().getTypeAllocSize(elem_ir_ty);
  auto array_size = builder->CreateMul(
      builder->CreateTypeSize(builder->getInt64Ty(), sz),
      CAPACITY.value);

  auto _malloc = requestFunction("malloc");
  assert(_malloc);
  auto array_ref = builder->CreateCall(_malloc, {array_size});
  auto _array_initialize = requestFunction("array_initialize");
  auto e_ptr = builder->CreateAlloca(elem_ir_ty);
  builder->CreateStore(ELEMENT.value, e_ptr);
  builder->CreateCall(_array_initialize, {array_ref, e_ptr, CAPACITY.value, builder->CreateTypeSize(builder->getInt64Ty(), sz)});
  return TyValue(make_shared<ty::Array>(*array_ty), array_ref);
}

TyValue CodeGenerator::visit(If &iff)
{
  auto COND = iff.condition->accept(*this);

  if (!COND.value)
    return TyValue();

  if (ty::mismatch(*COND.type, ty::Int()))
    fatalError("if condition must be int type", iff.condition->pos);

  auto func = builder->GetInsertBlock()->getParent();
  auto thenB = BasicBlock::Create(*context, newLabel("then"), func);

  if (iff.els)
  {
    auto elseB = BasicBlock::Create(*context, newLabel("else"));
    auto mergeB = BasicBlock::Create(*context, newLabel("merge"));
    builder->CreateCondBr(COND.value, thenB, elseB);

    builder->SetInsertPoint(thenB);
    auto THEN = iff.then->accept(*this);
    if (!THEN.value)
      return TyValue();
    builder->CreateBr(mergeB);
    thenB = builder->GetInsertBlock();

    func->insert(func->end(), elseB);
    builder->SetInsertPoint(elseB);
    auto ELSE = iff.els->accept(*this);
    if (!ELSE.value)
      return TyValue();
    builder->CreateBr(mergeB);
    elseB = builder->GetInsertBlock();

    func->insert(func->end(), mergeB);
    builder->SetInsertPoint(mergeB);
    auto phi = builder->CreatePHI(type2IRType(THEN.type.get()), 2);
    phi->addIncoming(THEN.value, thenB);
    phi->addIncoming(ELSE.value, elseB);
    // TODO check type
    return TyValue(THEN.type, phi);
  }
  else
  {
    auto mergeB = BasicBlock::Create(*context, newLabel());
    builder->CreateCondBr(COND.value, thenB, mergeB);

    builder->SetInsertPoint(thenB);
    auto THEN = iff.then->accept(*this);
    if (!THEN.value)
      return TyValue();
    builder->CreateBr(mergeB);
    thenB = builder->GetInsertBlock();

    func->insert(func->end(), mergeB);
    builder->SetInsertPoint(mergeB);
    return mkVoid();
  }
}

TyValue CodeGenerator::visit(While &whil)
{
  auto func = builder->GetInsertBlock()->getParent();
  auto loopB = BasicBlock::Create(*context, newLabel(), func);
  builder->CreateBr(loopB);
  builder->SetInsertPoint(loopB);
  auto bodyB = BasicBlock::Create(*context, newLabel());
  auto endB = BasicBlock::Create(*context, newLabel());
  auto COND = whil.condition->accept(*this);

  if (!COND.value)
    return TyValue();
  if (ty::mismatch(*COND.type, ty::Int()))
    fatalError("while condition must be int type", whil.condition->pos);

  builder->CreateCondBr(COND.value, bodyB, endB);
  func->insert(func->end(), bodyB);
  builder->SetInsertPoint(bodyB);
  breaks.push_back(endB);
  whil.body->accept(*this);
  breaks.pop_back();
  func->insert(func->end(), endB);
  builder->SetInsertPoint(endB);
  return mkVoid();
}

TyValue CodeGenerator::visit(For &forr)
{
  auto FROM = forr.from->accept(*this);
  if (!FROM.value)
    return TyValue();
  if (ty::mismatch(*FROM.type, ty::Int()))
    fatalError("lower of range must be int type", forr.from->pos);

  auto TO = forr.to->accept(*this);
  if (!TO.value)
    return TyValue();
  if (ty::mismatch(*TO.type, ty::Int()))
    fatalError("upper of range must be int type", forr.from->pos);

  beginScope();
  auto alloca = builder->CreateAlloca(builder->getInt64Ty(), nullptr, forr.var->id);
  namedValues.insert(forr.var->id, make_shared<VarEnventry>(make_shared<ty::Int>(), alloca));
  builder->CreateStore(FROM.value, alloca);
  auto func = builder->GetInsertBlock()->getParent();
  auto loopB = BasicBlock::Create(*context, newLabel("loop"), func);
  auto bodyB = BasicBlock::Create(*context, newLabel("body"));
  auto endB = BasicBlock::Create(*context, newLabel("end"));
  builder->CreateBr(loopB);

  builder->SetInsertPoint(loopB);
  auto var = builder->CreateLoad(alloca->getAllocatedType(), alloca, forr.var->id);
  auto cmp = builder->CreateICmpSLT(var, TO.value, "loopcond");
  builder->CreateCondBr(cmp, bodyB, endB);

  func->insert(func->end(), bodyB);
  builder->SetInsertPoint(bodyB);
  breaks.push_back(endB);
  forr.body->accept(*this);
  breaks.pop_back();
  auto currVar = builder->CreateLoad(alloca->getAllocatedType(), alloca, "currentvar");
  auto nextVar = builder->CreateAdd(currVar, builder->getInt64(1), "nextvar");
  builder->CreateStore(nextVar, alloca);
  builder->CreateBr(loopB);
  endScope();

  func->insert(func->end(), endB);
  builder->SetInsertPoint(endB);

  return mkVoid();
}

TyValue CodeGenerator::visit(Break &brk)
{
  if (!breaks.empty())
  {
    auto br = builder->CreateBr(breaks.back());
    return TyValue(make_shared<ty::Void>(), br);
  }
  else
  {
    fatalError("unexcepted break", brk.pos);
  }
}

void CodeGenerator::preprocessTypeDecs(vector<TypeDec *> decs)
{
  map<string, set<ty::Type **>> delayedInjections;

  auto injectOrDelay = [&delayedInjections, this](string name, ty::Type **ptr)
  {
    if (auto ty = namedTypes.find(name))
    {
      *ptr = ty->get();
    }
    else
    {
      auto found = delayedInjections.find(name);
      if (found != delayedInjections.end())
        found->second.insert(ptr);
      else
        delayedInjections.insert({name, {ptr}});
    }
  };

  for (auto dec : decs)
  {
    auto tid = dec->type_id->id;
    if (auto named = dynamic_cast<NamedType *>(dec->type.get()))
    {
      auto namedTy = make_shared<ty::Named>(tid, nullptr);
      namedTypes.insert(tid, namedTy);
      injectOrDelay(named->named->id, &namedTy->type);
    }
    else if (auto array = dynamic_cast<absyn::ArrayType *>(dec->type.get()))
    {
      auto arrayTy = make_shared<ty::Array>(nullptr);
      namedTypes.insert(tid, arrayTy);
      injectOrDelay(array->array->id, &arrayTy->type);
    }
    else if (auto record = dynamic_cast<RecordType *>(dec->type.get()))
    {
      auto recordTy = make_shared<ty::Record>(tid);
      namedTypes.insert(tid, recordTy);
      auto &records = recordTy->records;
      for (auto field : record->fields)
      {
        records.insert({field->name->id, nullptr});
        injectOrDelay(field->type_id->id, &records[field->name->id]);
      }
    }
    else
    {
      assert(0 && "unrecognized type in AST");
    }
  }

  for (auto pair : delayedInjections)
    if (auto ty = namedTypes.find(pair.first))
      for (auto ptr : pair.second)
        *ptr = ty->get();
}

llvm::Type *CodeGenerator::type2IRType(const ty::Type *ty)
{
  auto type = actualTy(ty);

  if (type->match(ty::Int()))
  {
    return builder->getInt64Ty();
  }
  else if (type->match(ty::String()))
  {
    return builder->getPtrTy();
  }
  else if (type->match(ty::Void()))
  {
    return builder->getVoidTy();
  }
  else if (type->match(ty::Nil()))
  {
    return builder->getPtrTy();
  }
  else
  {
    return builder->getPtrTy();
  }
}

void CodeGenerator::preprocessFunctionDecs(vector<FunctionDec *> func_decs)
{
  for (auto f_dec : func_decs)
  {
    if (namedValues.find_top(f_dec->funcname->id))
      fatalError("redeclare function " + f_dec->funcname->id, f_dec->pos);

    vector<shared_ptr<ty::Type>> args;
    shared_ptr<ty::Type> returnType = nullptr;
    ostringstream funcname_o;
    funcname_o << f_dec->funcname->id << "_" << ++this->func_id;

    for (auto field : f_dec->parameters)
    {
      if (auto ty = namedTypes.find(field->type_id->id))
      {
        args.push_back(*ty);
      }
      else
      {
        fatalError("undefined type " + field->type_id->id, field->pos);
      }
    }

    if (f_dec->return_type)
    {
      if (auto rt = namedTypes.find(f_dec->return_type->id))
      {
        returnType = *rt;
      }
      else
      {
        fatalError("undefined type " + f_dec->return_type->id, f_dec->return_type->pos);
      }
    }

    auto f_enventry = make_shared<FuncEnventry>(funcname_o.str(), returnType, args);
    namedValues.insert(f_dec->funcname->id, f_enventry);
    auto func = createFunction(*f_enventry);
    auto param_iter = f_dec->parameters.begin();
    for (auto &arg : func->args())
    {
      arg.setName((*param_iter)->name->id);
      param_iter++;
    }
  }
}

llvm::Function *CodeGenerator::createFunction(FuncEnventry &func)
{
  vector<llvm::Type *> paramTypes;
  for (auto ty : func.args)
    paramTypes.push_back(type2IRType(ty.get()));

  llvm::Type *returnType = builder->getVoidTy();
  if (func.returnType)
    returnType = type2IRType(func.returnType.get());

  auto funcType = FunctionType::get(returnType, paramTypes, false);
  return Function::Create(funcType, Function::ExternalLinkage, func.name, moduler.get());
}

TyValue CodeGenerator::visit(Let &let)
{
  beginScope();

  ptrs<Dec> decs;
  vector<TypeDec *> ty_decs;
  vector<FunctionDec *> func_decs;
  copy_if(let.decs.begin(), let.decs.end(), back_inserter(decs), [](auto ptr)
          { return dynamic_cast<TypeDec *>(ptr.get()); });
  transform(decs.begin(), decs.end(), back_inserter(ty_decs), [](auto ptr)
            { return dynamic_cast<TypeDec *>(ptr.get()); });
  decs.clear();
  copy_if(let.decs.begin(), let.decs.end(), back_inserter(decs), [](auto ptr)
          { return dynamic_cast<FunctionDec *>(ptr.get()); });
  transform(decs.begin(), decs.end(), back_inserter(func_decs), [](auto ptr)
            { return dynamic_cast<FunctionDec *>(ptr.get()); });

  preprocessTypeDecs(ty_decs);
  preprocessFunctionDecs(func_decs);

  for (auto &dec : let.decs)
    dec->accept(*this);
  auto BODY = let.body->accept(*this);
  endScope();

  return BODY;
}

TyValue CodeGenerator::visit(SimpleVar &var)
{
  auto found = namedValues.find(var.name->id);
  if (!found)
    fatalError("variable " + var.name->id + " is undefiend.", var.name->pos);

  auto var_enventry = dynamic_cast<VarEnventry *>(found->get());
  if (!var_enventry)
    fatalError(var.name->id + " is not a name of variable", var.name->pos);

  auto alloca = var_enventry->alloca;
  return TyValue(var_enventry->type, alloca);
}

TyValue CodeGenerator::visit(FieldVar &field)
{
  auto var = field.var->accept(*this);
  if (auto record = dynamic_cast<ty::Record *>(var.type.get()))
  {
    auto struct_ty = StructType::getTypeByName(*context, record->name);
    assert(struct_ty);
    auto found = record->records.find(field.field->id);
    if (found == record->records.end())
      fatalError("record type has no field " + field.field->id, field.field->pos);
    auto index = distance(record->records.begin(), found);
    auto base = builder->CreateLoad(type2IRType(var.type.get()), var.value);
    return TyValue(found->second->shared_from_this(), builder->CreateStructGEP(struct_ty, base, index));
  }
  else
  {
    fatalError("bad field access on a not record type", field.pos);
  }
}

TyValue CodeGenerator::visit(SubscriptVar &subscript)
{
  auto var = subscript.var->accept(*this);
  if (auto array = dynamic_cast<ty::Array *>(var.type.get()))
  {
    auto subs = subscript.subscript->accept(*this);
    if (!subs.type->match(ty::Int()))
      fatalError("subscript of array is not int type", subscript.subscript->pos);

    auto base = builder->CreateLoad(type2IRType(var.type.get()), var.value);
    return TyValue(array->type->shared_from_this(), builder->CreateInBoundsGEP(type2IRType(array->type), base, {subs.value}));
  }
  else
  {
    fatalError("bad element access on a not array type", subscript.pos);
  }
}

TyValue CodeGenerator::visit(TypeDec &typeDec)
{
  if (auto _ = dynamic_cast<absyn::RecordType *>(typeDec.type.get()))
  {
    auto found = namedTypes.find(typeDec.type_id->id);
    assert(found);
    auto record_ty = dynamic_pointer_cast<ty::Record>(*found);
    vector<llvm::Type *> elements;
    for (auto record : record_ty->records)
    {
      auto ty = actualTy(record.second);
      elements.push_back(type2IRType(ty));
    }
    auto struct_ty = StructType::create(*context, elements, typeDec.type_id->id);
  }
  return mkVoid();
}

TyValue CodeGenerator::visit(VarDec &varDec)
{
  if (varDec.type_id)
  {
    if (auto found = namedTypes.find(varDec.type_id->id))
    {
      auto alloca = builder->CreateAlloca(type2IRType(found->get()), nullptr, varDec.var->id);
      auto exp = varDec.exp->accept(*this);
      if (match(**found, *exp.type))
      {
        auto var = make_shared<VarEnventry>(exp.type, alloca);
        builder->CreateStore(exp.value, alloca);
        namedValues.insert(varDec.var->id, var);
      }
      else
      {
        fatalError("unmatched type for var declare " + varDec.type_id->id, varDec.exp->pos);
      }
    }
    else
    {
      fatalError("undeclared type " + varDec.type_id->id, varDec.type_id->pos);
    }
  }
  else
  {
    auto exp = varDec.exp->accept(*this);
    auto alloca = builder->CreateAlloca(type2IRType(exp.type.get()), nullptr, varDec.var->id);
    auto var = make_shared<VarEnventry>(exp.type, alloca);
    builder->CreateStore(exp.value, alloca);
    namedValues.insert(varDec.var->id, var);
  }

  return mkVoid();
}

TyValue CodeGenerator::visit(FunctionDec &funcDec)
{
  auto found = namedValues.find(funcDec.funcname->id);
  assert(found && "function declare not found");
  auto f_enventry = dynamic_cast<FuncEnventry *>(found->get());
  assert(f_enventry && "declare is not a function");
  Function *func = moduler->getFunction(f_enventry->name);
  assert(func && "function not found in ir");
  auto func_entry = BasicBlock::Create(*context, "", func);
  auto saved = builder->GetInsertBlock();
  builder->SetInsertPoint(func_entry);
  beginScope();
  assert(func->arg_size() == f_enventry->args.size());
  auto arg_iter = f_enventry->args.begin();
  for (auto &arg : func->args())
  {
    auto alloca = builder->CreateAlloca(arg.getType(), nullptr, arg.getName());
    builder->CreateStore(&arg, alloca);
    namedValues.insert(f_enventry->name, make_shared<VarEnventry>(*arg_iter, alloca));
    arg_iter++;
  }
  auto body = funcDec.body->accept(*this);
  endScope();
  if (f_enventry->returnType)
  {
    if (match(*f_enventry->returnType, *body.type))
    {
      builder->CreateRet(body.value);
    }
    else
    {
      fatalError("function " + funcDec.funcname->id + " mismatch return type", funcDec.body->pos);
    }
  }
  builder->SetInsertPoint(saved);
  return TyValue();
}

string CodeGenerator::newLabel(string topic)
{
  ostringstream o;
  o << "L" << this->label_id++;
  return topic.empty() ? o.str() : o.str() + "_" + topic;
}

void CodeGenerator::optimize()
{
  for (auto &func : moduler->getFunctionList())
  {
    functionPassManager->run(func);
  }
}

TyValue CodeGenerator::mkVoid()
{
  return TyValue(make_shared<ty::Void>(), UndefValue::get(builder->getVoidTy()));
}

void CodeGenerator::beginScope()
{
  namedValues.enter();
  namedTypes.enter();
}

void CodeGenerator::endScope()
{
  namedTypes.exit();
  namedValues.exit();
}

void CodeGenerator::reportError(std::string error, absyn::position pos)
{
  std::cerr << error << " (row: " << pos.line
            << ", column: " << pos.column << ")." << endl;
}

void CodeGenerator::fatalError(std::string error, absyn::position pos)
{
  reportError(error, pos);
  exit(1);
}

VarEnventry::VarEnventry(std::shared_ptr<ty::Type> type, llvm::AllocaInst *alloca)
    : type(type) { this->alloca = alloca; }

FuncEnventry::FuncEnventry(
    std::string name,
    std::shared_ptr<ty::Type> returnType,
    std::vector<std::shared_ptr<ty::Type>> args) : name(name), returnType(returnType), args(args)
{
}
