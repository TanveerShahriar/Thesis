#include "FunctionCollector.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// Singleton instance
FunctionCollector &FunctionCollector::getInstance() {
    static FunctionCollector instance;
    return instance;
}

// Constructor does NOT execute AST traversal immediately
FunctionCollector::FunctionCollector() : sourceFilePath("") {}

// Function to set the source file dynamically
void FunctionCollector::setSourceFile(const std::string &filePath) {
    sourceFilePath = filePath;
    executeASTTraversal(); // Run AST traversal after setting file
}

// Function to execute AST traversal
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

// Function to get the collected function set
const std::set<std::string> &FunctionCollector::getCollectedFunctions() const {
    return userDefinedFunctions;
}

// AST Visitor method to collect function names
void FunctionCollector::run(const MatchFinder::MatchResult &Result) {
    if (const FunctionDecl *FD = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
        if (FD->isThisDeclarationADefinition()) { // Ensure it's a function definition, not just a declaration
            userDefinedFunctions.insert(FD->getNameAsString());
        }
    }
}

// ✅ FIX: Implement missing constructor for `ASTConsumerWithMatcher`
ASTConsumerWithMatcher::ASTConsumerWithMatcher() : Collector(FunctionCollector::getInstance()) {
    Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &Collector);
}

void ASTConsumerWithMatcher::HandleTranslationUnit(ASTContext &Context) {
    Matcher.matchAST(Context);
}

// ✅ FIX: Implement `MyFrontendAction::CreateASTConsumer()` to fix linking error
std::unique_ptr<ASTConsumer> MyFrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef) {
    return std::make_unique<ASTConsumerWithMatcher>();
}