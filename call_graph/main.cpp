#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/CommandLine.h>
#include <fstream>
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
                return; // Silent failure
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

class FunctionRewriter : public MatchFinder::MatchCallback {
public:
    FunctionRewriter(Rewriter &R) : TheRewriter(R) {}

    void run(const MatchFinder::MatchResult &Result) override {
        if (const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
            if (Func->isImplicit() || !Func->isDefined() || Func->getNameAsString() == "main") {
                return;
            }

            // 1. Rewrite return type to void
            SourceLocation ReturnTypeStart = Func->getReturnTypeSourceRange().getBegin();
            TheRewriter.ReplaceText(ReturnTypeStart, Func->getReturnType().getAsString().length(), "void");

            if (auto FTL = Func->getTypeSourceInfo()->getTypeLoc().getAs<FunctionTypeLoc>()) {
                SourceLocation LParenLoc = FTL.getLParenLoc();
                SourceLocation RParenLoc = FTL.getRParenLoc();
                if (LParenLoc.isValid() && RParenLoc.isValid() && LParenLoc < RParenLoc) {
                    TheRewriter.ReplaceText(SourceRange(LParenLoc, RParenLoc),
                                            "(int thread_idx, int param_index)");
                }
            }

            // 3. Modify return statements if the original return type was not void
            if (!Func->getReturnType()->isVoidType()) {
                if (const CompoundStmt *Body = dyn_cast<CompoundStmt>(Func->getBody())) {
                    for (auto *Stmt : Body->body()) {
                        if (const ReturnStmt *RetStmt = dyn_cast<ReturnStmt>(Stmt)) {
                            SourceLocation RetStart = RetStmt->getBeginLoc();
                            std::string ReturnReplacement = Func->getNameAsString() + "_params[param_index].return_var = ";
                            TheRewriter.ReplaceText(SourceRange(RetStart, RetStart.getLocWithOffset(6)),
                                                    ReturnReplacement);
                        }
                    }
                }
            }
        }
    }

private:
    Rewriter &TheRewriter;
};

class FunctionASTConsumer : public ASTConsumer {
public:
    FunctionASTConsumer(Rewriter &R, const std::string &outputFile)
        : StructDumper(outputFile), FuncRewriter(R) {
        Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &StructDumper);
        Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &FuncRewriter);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context);
    }

private:
    FunctionStructDumper StructDumper;
    FunctionRewriter FuncRewriter;
    MatchFinder Matcher;
};

class FunctionFrontendAction : public ASTFrontendAction {
public:
    FunctionFrontendAction() : outputFile("output/struct.cpp") {}

    void EndSourceFileAction() override {
        // Save the changes to the input source file
        std::error_code EC;
        llvm::raw_fd_ostream OutFile(SourceFilePath, EC, llvm::sys::fs::OF_Text);
        if (!EC) {
            TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(OutFile);
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        SourceFilePath = file.str();
        return std::make_unique<FunctionASTConsumer>(TheRewriter, outputFile);
    }

private:
    Rewriter TheRewriter;
    std::string SourceFilePath;
    std::string outputFile;
};

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
    if (argc > 1) {
        fs::create_directories("output");
        std::ofstream outFile("output/struct.cpp");
        outFile.close();

        auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
        if (!ExpectedParser) {
            return 1;
        }
        CommonOptionsParser &OptionsParser = ExpectedParser.get();

        ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
        return Tool.run(newFrontendActionFactory<FunctionFrontendAction>().get());
    }
    return 1;
}