#include "FunctionCollector.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

FunctionCollector &FunctionCollector::getInstance() {
    static FunctionCollector instance;
    return instance;
}

FunctionCollector::FunctionCollector() : sourceFilePath("") {}

void FunctionCollector::setSourceFile(const std::string &filePath) {
    sourceFilePath = filePath;
    executeASTTraversal();
}

void FunctionCollector::executeASTTraversal() {
    if (sourceFilePath.empty()) {
        std::cerr << "Error: No source file set for AST traversal.\n";
        return;
    }

    int argc = 2;
    const char *argv[] = { "function_collector", sourceFilePath.c_str() };

    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        return;
    }

    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    MatchFinder Matcher;
    Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), this);
    Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}

const std::set<std::string> &FunctionCollector::getCollectedFunctions() const {
    return userDefinedFunctions;
}

void FunctionCollector::run(const MatchFinder::MatchResult &Result) {
    if (const FunctionDecl *FD = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
        if (FD->isThisDeclarationADefinition()) {
            userDefinedFunctions.insert(FD->getNameAsString());
        }
    }
}

ASTConsumerWithMatcher::ASTConsumerWithMatcher() : Collector(FunctionCollector::getInstance()) {
    Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &Collector);
}

void ASTConsumerWithMatcher::HandleTranslationUnit(ASTContext &Context) {
    Matcher.matchAST(Context);
}

std::unique_ptr<ASTConsumer> MyFrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef) {
    return std::make_unique<ASTConsumerWithMatcher>();
}