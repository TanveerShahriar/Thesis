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
            if (Func->isImplicit() || !Func->isDefined())
                return;
            
            bool isMain = (Func->getNameAsString() == "main");
            CurrentFunction = Func;

            if (!isMain){
                // (1) Rewrite return type and function signature as you already do.
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

                // (2) Process return statements if needed.
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

            // (3) Traverse function body to rewrite parameter references and record call expressions.
            nonVoidCallees.clear();
            TraverseDecl(const_cast<FunctionDecl *>(Func));

            // (4) Insert the extra lines at the end of the function body.
            if (const CompoundStmt *Body = dyn_cast<CompoundStmt>(Func->getBody())) {
                SourceLocation InsertLoc = Body->getRBracLoc(); // before the closing '}'
                std::string extraCode;
                // For each non-void callee that was called inside, insert the push statement.
                for (const auto &callee : nonVoidCallees) {
                    // NOTE: This assumes that the variable 'index' is in scope.
                    // You might need to adjust your code so that 'index' is declared in a scope
                    // accessible here (for example, store the computed index from the call expression).
                    extraCode += callee + "_params_index_pool.push(index);\n";
                }
                // If the current function returns a value, mark it done.
                if (!Func->getReturnType()->isVoidType() && !isMain) {
                    extraCode += Func->getNameAsString() + "_params[param_index]." + Func->getNameAsString() + "_done = true;\n";
                }
                TheRewriter.InsertTextBefore(InsertLoc, extraCode);
            }

            CurrentFunction = nullptr;
        }
    }

    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (!CurrentFunction)
            return true;

        // (1) Existing action: Rewrite parameter references.
        if (const ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(DRE->getDecl())) {
            std::string functionName = CurrentFunction->getNameAsString();
            std::string paramName = PVD->getNameAsString();
            std::string replacement = functionName + "_params[param_index]." + paramName;
            SourceLocation ParamLoc = DRE->getBeginLoc();
            TheRewriter.ReplaceText(ParamLoc, paramName.length(), replacement);
        }

        // (2) Global variable usage rewriting: wrap the entire line in a block with a lock.
        if (const VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            const SourceManager &SM = TheRewriter.getSourceMgr();
            // Skip globals defined in system headers.
            if (SM.isInSystemHeader(VD->getLocation()))
                return true;

            if (VD->getDeclContext()->isTranslationUnit() || VD->hasGlobalStorage()) {
                SourceLocation loc = DRE->getBeginLoc();
                unsigned line = SM.getSpellingLineNumber(loc);

                // Only process this line once.
                if (processedGlobalLines.find(line) == processedGlobalLines.end()) {
                    processedGlobalLines.insert(line);

                    // Compute the start of the line.
                    unsigned offset = SM.getFileOffset(loc);
                    unsigned colNo = SM.getColumnNumber(SM.getFileID(loc), offset);
                    SourceLocation lineStart = loc.getLocWithOffset(-static_cast<int>(colNo) + 1);

                    // Get the buffer for the file to find the end of the line.
                    FileID fid = SM.getFileID(loc);
                    bool invalid = false;
                    StringRef buffer = SM.getBufferData(fid, &invalid);
                    if (!invalid) {
                        // Find the end of the line by advancing until a newline or end-of-buffer.
                        unsigned lineEndOffset = offset;
                        while (lineEndOffset < buffer.size() && buffer[lineEndOffset] != '\n')
                            ++lineEndOffset;
                        SourceLocation lineEnd = SM.getLocForStartOfFile(fid).getLocWithOffset(lineEndOffset);

                        // Insert the opening brace and lock at the beginning of the line.
                        std::string lockBlockStart = "{ unique_lock<mutex> lock(mutexes[thread_idx]);";
                        TheRewriter.InsertText(lineStart, lockBlockStart, true, true);
                        // Insert the closing brace after the line.
                        std::string lockBlockEnd = " }";
                        TheRewriter.InsertTextAfterToken(lineEnd, lockBlockEnd);
                    }
                }
            }
        }

        return true;
    }


    // Visitor to handle function calls.
    bool VisitCallExpr(CallExpr *CE) {
        if (!CE->getDirectCallee())
            return true;
        const FunctionDecl *Callee = CE->getDirectCallee();
        if (Callee->isImplicit() || Callee->getBuiltinID() != 0 || !Callee->doesThisDeclarationHaveABody())
            return true;

        std::string functionName = Callee->getNameAsString();
        if (functionName.find("operator") == 0)
            return true;

        const SourceManager &SM1 = TheRewriter.getSourceMgr();
        std::string argsString;
        for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
            const Expr *arg = CE->getArg(i);
            SourceRange argRange = arg->getSourceRange();
            llvm::StringRef argText = Lexer::getSourceText(CharSourceRange::getTokenRange(argRange), SM1, TheRewriter.getLangOpts());
            if (i > 0) argsString += ", ";
            argsString += argText.str();
        }

        std::string pushThreadStmt = "int index; \n { \n unique_lock<mutex> lock(mutexes[thread_idx]);\n if (" +
            functionName + "_params_index_pool.empty()){\n index = " + functionName +
            "_params.size();\n" + functionName + "_params.emplace_back();\n }\n else { \n index = " +
            functionName + "_params_index_pool.front(); \n" + functionName + "_params_index_pool.pop(); \n }\n" +
            functionName + "_params[index] = {" + argsString + "};\n }\n" +
            "pushToThread(" + functionName + "_enumidx);\n";

        if (!Callee->getReturnType()->isVoidType()) {
            pushThreadStmt += "while (!" + functionName + "_params[index]." + functionName +
                "_done) {\n if(!queues[thread_idx].empty()) execute(thread_idx); \n} \n";
            // Record the callee name so that later we add the push statement
            nonVoidCallees.push_back(functionName);
        }

        SourceRange callRange = CE->getSourceRange();
        CharSourceRange charRange = CharSourceRange::getTokenRange(callRange);
        SourceLocation callStart = charRange.getBegin();

        // Find the beginning of the line where the call occurs.
        const SourceManager &SM = TheRewriter.getSourceMgr();
        SourceLocation expansionLoc = SM.getExpansionLoc(callStart);
        FileID fid = SM.getFileID(expansionLoc);
        unsigned offset = SM.getFileOffset(expansionLoc);
        unsigned colNo = SM.getColumnNumber(fid, offset);
        SourceLocation lineStart = callStart.getLocWithOffset(-static_cast<int>(colNo) + 1);

        // Insert the pushThreadStmt before the line.
        TheRewriter.InsertTextBefore(lineStart, pushThreadStmt);

        if (!Callee->getReturnType()->isVoidType()) {
            std::string returnReplacement = functionName + "_params[index]." + functionName + "_return";
            TheRewriter.ReplaceText(charRange, returnReplacement);
        } else {
            SourceLocation callEnd = CE->getEndLoc();
            SourceLocation semiLoc = Lexer::findLocationAfterToken(
                callEnd, tok::semi, SM, TheRewriter.getLangOpts(), false);
            if (semiLoc.isValid()) {
                TheRewriter.RemoveText(SourceRange(callStart, semiLoc));
            } else {
                TheRewriter.RemoveText(charRange);
            }
        }

        return true;
    }

private:
    Rewriter &TheRewriter;
    const FunctionDecl *CurrentFunction;
    std::vector<std::string> nonVoidCallees;
    std::set<unsigned> processedGlobalLines;
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