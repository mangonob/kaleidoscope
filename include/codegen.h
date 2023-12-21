#pragma once
#include <memory>
#include <map>
#include <functional>
#include <variant>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Function.h>
#include "absyn.h"
#include "types.h"
#include "table.h"

namespace cg
{
  struct TyValue
  {
    llvm::Value *value;
    std::shared_ptr<ty::Type> type;

    TyValue(std::shared_ptr<ty::Type> type = std::make_shared<ty::Undefined>(), llvm::Value *value = nullptr);
  };

  struct Enventry
  {
    virtual ~Enventry() = default;
  };

  struct VarEnventry : Enventry
  {
    llvm::AllocaInst *alloca;
    std::shared_ptr<ty::Type> type;

    VarEnventry(std::shared_ptr<ty::Type> type, llvm::AllocaInst *alloca);
  };

  struct FuncEnventry : Enventry
  {
    std::string name;
    std::shared_ptr<ty::Type> returnType;
    std::vector<std::shared_ptr<ty::Type>> args;

    FuncEnventry(std::string name, std::shared_ptr<ty::Type> returnType, std::vector<std::shared_ptr<ty::Type>> args);
  };

  class AbstractCodeGenerator
  {
  public:
    virtual TyValue visit(absyn::Nil &n) = 0;
    virtual TyValue visit(absyn::Int &i) = 0;
    virtual TyValue visit(absyn::String &s) = 0;
    virtual TyValue visit(absyn::VarExp &var) = 0;
    virtual TyValue visit(absyn::Assign &assign) = 0;
    virtual TyValue visit(absyn::Seq &seq) = 0;
    virtual TyValue visit(absyn::Call &call) = 0;
    virtual TyValue visit(absyn::BinOp &bin) = 0;
    virtual TyValue visit(absyn::RecordExp &record) = 0;
    virtual TyValue visit(absyn::Array &array) = 0;
    virtual TyValue visit(absyn::If &iff) = 0;
    virtual TyValue visit(absyn::While &whil) = 0;
    virtual TyValue visit(absyn::For &forr) = 0;
    virtual TyValue visit(absyn::Break &brk) = 0;
    virtual TyValue visit(absyn::Let &let) = 0;
    virtual TyValue visit(absyn::SimpleVar &var) = 0;
    virtual TyValue visit(absyn::FieldVar &field) = 0;
    virtual TyValue visit(absyn::SubscriptVar &subscript) = 0;
    virtual TyValue visit(absyn::TypeDec &typeDec) = 0;
    virtual TyValue visit(absyn::VarDec &varDec) = 0;
    virtual TyValue visit(absyn::FunctionDec &funcDec) = 0;
  };

  class CodeGenerator : public AbstractCodeGenerator
  {
    int label_id = 0;
    int func_id = 0;

  public:
    virtual TyValue visit(absyn::Nil &n) override;
    virtual TyValue visit(absyn::Int &i) override;
    virtual TyValue visit(absyn::String &s) override;
    virtual TyValue visit(absyn::VarExp &var) override;
    virtual TyValue visit(absyn::Assign &assign) override;
    virtual TyValue visit(absyn::Seq &seq) override;
    virtual TyValue visit(absyn::Call &call) override;
    virtual TyValue visit(absyn::BinOp &bin) override;
    virtual TyValue visit(absyn::RecordExp &record) override;
    virtual TyValue visit(absyn::Array &array) override;
    virtual TyValue visit(absyn::If &iff) override;
    virtual TyValue visit(absyn::While &whil) override;
    virtual TyValue visit(absyn::For &forr) override;
    virtual TyValue visit(absyn::Break &brk) override;
    virtual TyValue visit(absyn::Let &let) override;
    virtual TyValue visit(absyn::SimpleVar &var) override;
    virtual TyValue visit(absyn::FieldVar &field) override;
    virtual TyValue visit(absyn::SubscriptVar &subscript) override;
    virtual TyValue visit(absyn::TypeDec &typeDec) override;
    virtual TyValue visit(absyn::VarDec &varDec) override;
    virtual TyValue visit(absyn::FunctionDec &funcDec) override;

    CodeGenerator();

    std::string newLabel(std::string topic = "");

  protected:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::FunctionPassManager> functionPassManager;
    std::unique_ptr<llvm::FunctionAnalysisManager> functionAnalysisManager;
    tb::Table<std::string, std::shared_ptr<Enventry>> namedValues;
    tb::Table<std::string, std::shared_ptr<ty::Type>> namedTypes;
    std::vector<llvm::BasicBlock *> breaks;

  public:
    std::unique_ptr<llvm::Module> moduler;
    std::unique_ptr<llvm::IRBuilder<>> builder;

  private:
    std::map<std::string, std::function<llvm::Function *()>> libraryFunctionCreator;

    TyValue mkVoid();
    void preprocessTypeDecs(std::vector<absyn::TypeDec *> decs);
    void preprocessFunctionDecs(std::vector<absyn::FunctionDec *> func_decs);
    llvm::Type *type2IRType(const ty::Type *type);
    llvm::Function *createFunction(FuncEnventry &func);
    llvm::Function *requestFunction(std::string funcname);
    void registeLibraryFunction(std::string name, std::function<llvm::Function *()> factory);

  protected:
    void beginScope();
    void endScope();
    void reportError(std::string error, absyn::position pos);
    [[noreturn]] void fatalError(std::string error, absyn::position pos);
  };
}
