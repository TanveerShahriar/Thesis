#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
namespace fs = std::filesystem;

class FunctionStructDumper : public MatchFinder::MatchCallback {
public:
    FunctionStructDumper(const std::string &outputFile) : outputFile(outputFile) {}

    void run(const MatchFinder::MatchResult &Result) override {
        if (const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
            if (Func->isImplicit() || !Func->isDefined() || Func->getNameAsString() == "main") {
                return; // Ignore built-in, undefined, and main functions
            }

            std::ofstream outFile(outputFile, std::ios::app);
            if (!outFile.is_open()) {
                std::cerr << "Failed to open output file!\n";
                return;
            }

            outFile << "struct " << Func->getNameAsString() << "_Struct {\n";
            for (const ParmVarDecl *Param : Func->parameters()) {
                outFile << "    " << Param->getType().getAsString() << " " << Param->getNameAsString() << ";\n";
            }
            if (!Func->getReturnType()->isVoidType()) {
                outFile << "    " << Func->getReturnType().getAsString() << " return_var;\n";
            }
            outFile << "};\n\n";
            outFile.close();
        }
    }

private:
    std::string outputFile;
};

class FunctionASTConsumer : public ASTConsumer {
public:
    FunctionASTConsumer(const std::string &outputFile) : Handler(outputFile) {
        Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &Handler);
    }
    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context);
    }
private:
    FunctionStructDumper Handler;
    MatchFinder Matcher;
};

class FunctionFrontendAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef) override {
        return std::make_unique<FunctionASTConsumer>(outputFile);
    }

    void EndSourceFileAction() override {
        std::cout << "Structs dumped in " << outputFile << "\n";
    }

private:
    std::string outputFile = "output/struct.cpp";
};

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
    if (argc > 1) {
        fs::create_directories("output");
        std::ofstream outFile("output/struct.cpp");
        outFile.close();

        auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
        if (!ExpectedParser) {
            llvm::errs() << ExpectedParser.takeError();
            return 1;
        }
        CommonOptionsParser &OptionsParser = ExpectedParser.get();

        ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
        return Tool.run(newFrontendActionFactory<FunctionFrontendAction>().get());
    }
    std::cerr << "Usage: " << argv[0] << " <source-file>\n";
    return 1;
}