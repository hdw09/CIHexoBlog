#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"
#include <sstream>
#include <string>
#include <regex>

using namespace std;
using namespace clang;
using namespace oclint;

namespace HDW {
    
    class MissingStrongifyInCatchSelfBlockRule : public AbstractASTVisitorRule<MissingStrongifyInCatchSelfBlockRule>
    {
    public:
        
        MissingStrongifyInCatchSelfBlockRule()
        {
            this->lastClassMethodNode = nullptr;
            
            /// 处理下面类方法block不检查strongify，其他类方法中的block跟普通block一样接受检查
            this->classWhiteList.push_back("UIView");
        }
        
        virtual const string name() const override
        {
            return "含有self的block中没有使用strongify";
        }
        
        virtual int priority() const override
        {
            return 1;
        }
        
        virtual const string category() const override
        {
            return "HDW";
        }
        
        bool VisitObjCMessageExpr(ObjCMessageExpr *node)
        {
            if (node->isClassMessage() && this->isWhiteListClass(node)) {
                int argCount = node->getNumArgs();
                Expr **exprArray = node->getArgs();
                for (int i = 0; i < argCount; i ++ ){
                    BlockExpr *expr = dyn_cast_or_null<BlockExpr>(exprArray[i]);
                    if (expr && expr->getBlockDecl()) {
                        this->lastClassMethodNode = expr->getBlockDecl();
                    }
                }
            }
            return true;
        }
        
        bool VisitBlockDecl(BlockDecl *node)
        {
            if (node == this->lastClassMethodNode) {
                return true;
            }
            for (BlockDecl::capture_const_iterator iterator = node->capture_begin() ; iterator != node->capture_end(); iterator ++) {
                ImplicitParamDecl *implicitParamDecl = dyn_cast_or_null<ImplicitParamDecl>(iterator->getVariable());
                if (implicitParamDecl && implicitParamDecl->getName() == "self") {
                    if (!BlockDeclHasStrongify(node) && !isMasnoryBlock(node)) {
                        addViolation(node, this, description("这个block中引用了self，但是没有使用@strongify"));
                    }
                }
            }
            return true;
        }
        
    private:
        
        BlockDecl *lastClassMethodNode;
        vector<string> classWhiteList;
        
        string description(const string& info)
        {
            ostringstream stream;
            stream <<"亲爱的：" << info;
            return stream.str();
        }
        
        bool isMasnoryBlock(BlockDecl *node)
        {
            for (BlockDecl::param_iterator iterator = node->param_begin() ; iterator != node->param_end(); iterator ++) {
                if ((*iterator)->getType().getAsString().find("MASConstraintMaker") != string::npos)
                    return true;
            }
            return false;
        }
        
        bool isWhiteListClass(ObjCMessageExpr *node)
        {
            string type = node->getClassReceiver().getAsString();
            vector<string>::iterator ret = std::find(this->classWhiteList.begin(), this->classWhiteList.end(), type);
            if(ret == this->classWhiteList.end())
                return false;
            else
                return true;
        }
        
        bool BlockDeclHasStrongify(BlockDecl *node)
        {
            regex pattern("\\^[^\\{\\}\\^]*\\{[^\\{\\}]*\\}");
            regex patternAnnotationStrongify("//[^\n]*@strongify");
            regex patternOtherAnnotationStrongify("/\\*[\\s\\S]*@strongify[\\s\\S]*\\*/");
            
            clang::SourceManager *sourceManager = &_carrier->getSourceManager();
            SourceLocation startLocation = node->getLocStart();
            SourceLocation endLocation = node->getLocEnd();
            if (sourceManager->isMacroBodyExpansion(startLocation)
                || sourceManager->isInSystemMacro(startLocation)
                || sourceManager->isMacroArgExpansion(startLocation)) {
                return true;
            }
            SourceLocation startSpellingLoc = startLocation, endSpellingLoc = endLocation;
            if (!startLocation.isFileID()) {
                startSpellingLoc = sourceManager->getSpellingLoc(startLocation);
                endSpellingLoc = sourceManager->getSpellingLoc(endLocation);
            }
            int length = sourceManager->getFileOffset(endSpellingLoc) -  sourceManager->getFileOffset(startSpellingLoc);
            if (length > 0) {
                string blockStatementString = StringRef(sourceManager->getCharacterData(startLocation) + 1, length).str();
                match_results<string::const_iterator> result;
                string newBlockStatementString(blockStatementString);
                do {
                    blockStatementString = newBlockStatementString;
                    newBlockStatementString = std::regex_replace(blockStatementString, pattern, "");
                } while (newBlockStatementString.length() != blockStatementString.length());
                blockStatementString = std::regex_replace(blockStatementString, patternAnnotationStrongify, "");
                blockStatementString = std::regex_replace(blockStatementString, patternOtherAnnotationStrongify, "");
                if (blockStatementString.find("@strongify") == string::npos) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        }
    };
}

static RuleSet rules(new HDW::MissingStrongifyInCatchSelfBlockRule());
