#include "FunctionCollector.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace clang::tooling;
namespace fs = std::filesystem;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

FunctionCollector &FunctionCollector::getInstance() {
    static FunctionCollector instance;
    return instance;
}

FunctionCollector::FunctionCollector() : sourceFilePath("") {}

void FunctionCollector::collectFunctions(const std::string &filePath) {
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
        std::cerr << "Error: Failed to create CommonOptionsParser.\n";
        return;
    }

    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}

const std::set<std::string> &FunctionCollector::getCollectedFunctions() const {
    return userDefinedFunctions;
}

const std::set<std::string> &FunctionCollector::getCollectedFunctions_withMangling() const {
    return userDefinedFunctions_withMangling;
}

void FunctionCollector::run(const MatchFinder::MatchResult &Result) {
    if (const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
        if (Func->isThisDeclarationADefinition()) {
            std::string funcName = Func->getNameAsString();
            userDefinedFunctions.insert(funcName);

            if (funcName == "main")
                return;

            fs::create_directories("output");
            std::ofstream outFile("output/struct.txt", std::ios::app);
            if (!outFile.is_open()) {
                std::cerr << "Error: Could not open output file.\n";
                return;
            }

            std::string mangling, outParams;
            for (const ParmVarDecl *Param : Func->parameters()) {
                mangling += Param->getType().getAsString()[0];
                outParams += "    " + Param->getType().getAsString() + " " + Param->getNameAsString() + ";\n";
            }

            userDefinedFunctions_withMangling.insert(funcName + mangling);

            outFile << "struct " << funcName + mangling << "_Struct {\n";
            outFile << outParams;

            if (!Func->getReturnType()->isVoidType()) {
                outFile << "    " << Func->getReturnType().getAsString() << " return_var;\n";
                outFile << "    bool " << funcName << "_done = false;\n";
            }

            outFile << "};\n\n";
            outFile.close();
        }
    }
}

ASTConsumerWithMatcher::ASTConsumerWithMatcher(FunctionCollector &collector)
    : Collector(collector) {
    Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &Collector);
}

void ASTConsumerWithMatcher::HandleTranslationUnit(ASTContext &Context) {
    Matcher.matchAST(Context);
}

MyFrontendAction::MyFrontendAction() : Collector(FunctionCollector::getInstance()) {}

std::unique_ptr<ASTConsumer> MyFrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
    return std::make_unique<ASTConsumerWithMatcher>(Collector);
}