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
                return;
            }

            std::ofstream outFile(outputFile, std::ios::app);
            if (!outFile.is_open()) {
                return;
            }

            outFile << "struct " << Func->getNameAsString() << "_Struct {\n";
            for (const ParmVarDecl *Param : Func->parameters()) {
                outFile << "    " << Param->getType().getAsString() << " " << Param->getNameAsString() << ";\n";
            }

            if (!Func->getReturnType()->isVoidType()) {
                outFile << "    " << Func->getReturnType().getAsString() << " return_var;\n";
                outFile << "    bool " << Func->getNameAsString() << "_done = false;\n";
            }

            outFile << "};\n\n";
            outFile.close();
        }
    }

private:
    std::string outputFile;
};

class FunctionRewriter : public MatchFinder::MatchCallback, public RecursiveASTVisitor<FunctionRewriter> {
public:
    FunctionRewriter(Rewriter &R) : TheRewriter(R), CurrentFunction(nullptr) {}

    void run(const MatchFinder::MatchResult &Result) override {
        if (const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
            if (Func->isImplicit() || !Func->isDefined() || Func->getNameAsString() == "main") {
                return;
            }

            CurrentFunction = Func;

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

            // 2. Modify return statements if the original return type was not void
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

            // 3. Traverse function body to replace parameters and function calls
            CurrentFunction = Func;
            TraverseDecl(const_cast<FunctionDecl *>(Func));
            CurrentFunction = nullptr;
        }
    }

    // Visitor function to replace parameter references in the function body
    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (!CurrentFunction) return true;

        if (const ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(DRE->getDecl())) {
            std::string functionName = CurrentFunction->getNameAsString();
            std::string paramName = PVD->getNameAsString();

            std::string replacement = functionName + "_params[param_index]." + paramName;

            SourceLocation ParamLoc = DRE->getBeginLoc();
            TheRewriter.ReplaceText(ParamLoc, paramName.length(), replacement);
        }
        return true;
    }

    // Visitor function to handle function calls
    bool VisitCallExpr(CallExpr *CE) {
    if (!CE->getDirectCallee())
        return true;

    const FunctionDecl *Callee = CE->getDirectCallee();

    // Skip built-in functions, implicit functions, or functions without a body
    if (Callee->isImplicit() || Callee->getBuiltinID() != 0 || !Callee->doesThisDeclarationHaveABody())
        return true;

    std::string functionName = Callee->getNameAsString();

    // Skip overloaded operators (e.g. operator<<)
    if (functionName.find("operator") == 0)
        return true;

    // Inside VisitCallExpr(CallExpr *CE)
    std::string argsString;
    const SourceManager &SM1 = TheRewriter.getSourceMgr();
    for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
        const Expr *arg = CE->getArg(i);
        SourceRange argRange = arg->getSourceRange();
        llvm::StringRef argText = Lexer::getSourceText(CharSourceRange::getTokenRange(argRange), SM1, TheRewriter.getLangOpts());
        if (i > 0) {
            argsString += ", ";
        }
        argsString += argText.str();
    }

    // Prepare the text to insert and the replacement text
    std::string pushThreadStmt = "int index; \n { \n unique_lock<mutex> lock(mutexes[thread_idx]);\n if ( " + 
        functionName + "_params_index_pool.empty()){\n index = " + 
        functionName + "_params.size();\n" + 
        functionName + "_params.emplace_back();\n }\n else { \n index = " +
        functionName + "_params.index.pool.front(); \n" + 
        functionName + "_params_index_pool.pop(); \n }\n" + 
        functionName + "_params[index] = {" + argsString +"};\n }\n" + "pushToThread(" + functionName + "_enumidx);\n";
    
     if (!Callee->getReturnType()->isVoidType()) {
        pushThreadStmt += "while (!" + functionName + "_params[index]." + functionName 
        + "_done) {\n if(!queues[thread_idx].empty()) execute(thread_idx); \n} \n";
     }
    std::string returnReplacement = functionName + "_params[index]." + functionName + "_return";

    // Get the token range for the call expression
    SourceRange callRange = CE->getSourceRange();
    CharSourceRange charRange = CharSourceRange::getTokenRange(callRange);
    SourceLocation callStart = charRange.getBegin();

    // Get the SourceManager and determine the start of the line where the call occurs
    const SourceManager &SM = TheRewriter.getSourceMgr();
    SourceLocation expansionLoc = SM.getExpansionLoc(callStart);
    FileID fid = SM.getFileID(expansionLoc);
    unsigned offset = SM.getFileOffset(expansionLoc);
    unsigned colNo = SM.getColumnNumber(fid, offset);
    SourceLocation lineStart = callStart.getLocWithOffset(-static_cast<int>(colNo) + 1);

    // Insert the pushToThread statement at the beginning of the line
    TheRewriter.InsertTextBefore(lineStart, pushThreadStmt);

    // If the function returns a value, replace only the call expression tokens.
    if (!Callee->getReturnType()->isVoidType()) {
        TheRewriter.ReplaceText(charRange, returnReplacement);
    } else {
        // For void functions, try to remove the entire statement including the trailing semicolon.
        SourceLocation callEnd = CE->getEndLoc();
        // Use the Lexer to find the semicolon after the call.
        SourceLocation semiLoc = Lexer::findLocationAfterToken(
            callEnd, tok::semi, SM, TheRewriter.getLangOpts(), /*SkipTrailingWhitespace=*/false);
        if (semiLoc.isValid()) {
            // Remove from the beginning of the call to the semicolon.
            TheRewriter.RemoveText(SourceRange(callStart, semiLoc));
        } else {
            // Fallback: remove only the call expression tokens.
            TheRewriter.RemoveText(charRange);
        }
    }

    return true;
}

private:
    Rewriter &TheRewriter;
    const FunctionDecl *CurrentFunction;
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