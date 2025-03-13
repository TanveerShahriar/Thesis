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
    static FunctionCollector &getInstance(); // Singleton instance

    void run(const MatchFinder::MatchResult &Result) override;
    const std::set<std::string> &getCollectedFunctions() const;
    
    void setSourceFile(const std::string &filePath); // Set source file dynamically

private:
    FunctionCollector(); // Private constructor for singleton
    void executeASTTraversal(); // Runs AST matching

    std::set<std::string> userDefinedFunctions;
    std::string sourceFilePath; // Store source file path

    friend class ASTConsumerWithMatcher;
};

class ASTConsumerWithMatcher : public ASTConsumer {  // FIX: Declare properly
public:
    FunctionCollector &Collector;
    MatchFinder Matcher;

    ASTConsumerWithMatcher(); // Declare constructor
    void HandleTranslationUnit(ASTContext &Context) override;
};

class MyFrontendAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef) override;
};

#endif // FUNCTION_COLLECTOR_H