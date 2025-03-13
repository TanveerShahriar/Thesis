#ifndef FUNCTION_COLLECTOR_H
#define FUNCTION_COLLECTOR_H

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <set>
#include <string>

using namespace clang;
using namespace clang::ast_matchers;

class FunctionCollector : public MatchFinder::MatchCallback {
public:
    static FunctionCollector &getInstance(); 

    void run(const MatchFinder::MatchResult &Result) override;
    const std::set<std::string> &getCollectedFunctions() const;
    
    void setSourceFile(const std::string &filePath); 

private:
    FunctionCollector(); 
    void executeASTTraversal(); 

    std::set<std::string> userDefinedFunctions;
    std::string sourceFilePath; 

    friend class ASTConsumerWithMatcher;
};

class ASTConsumerWithMatcher : public ASTConsumer { 
public:
    FunctionCollector &Collector;
    MatchFinder Matcher;

    ASTConsumerWithMatcher();
    void HandleTranslationUnit(ASTContext &Context) override;
};

class MyFrontendAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef) override;
};

#endif