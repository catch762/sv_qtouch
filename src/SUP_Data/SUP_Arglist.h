#pragma once
#include "sv_qtcommon.h"
#include "BasicTokenizer/BasicTokenizer.h"
//#include "SUP_Data.h"

//todo: make this single place where ui macro string is explained. Use this alias everywhere.

using UiMacroString = QString;
SV_DECL_ALIASES(UiMacroString);

//************************************************************************************** 
// So, ui string (that exist in glsl code files that we parse) typically looks like this:
//
//      arg0 = whatever, lims = [ [0, 1], [2, -3.5, 'hello'] ]
//          
// Yea, this used to go to Python eval(), so its essentially Python code. Although
// we dont have to support expressions like function calls, only the simplest things.
//
// This small SUP_ArglistParser takes text, turns it to BasicToken's with common
// component BasicTokenizer, then turns the tokens into final data representation -
// see UiStringParsedData below.
//
// Lets explain how we expect example string above to be parsed.
// First of all 'arg0 = whatever' part. 'whatever' is a variable that exist in
// %variable dictionary%. Assume it exists and holds value equivalent to BasicToken::makeNumberInt(5)
// 
// So, our resulting UiStringParsedData.namedExpressions would be:
//
//  {
//      {"arg0" :   Leaf SUPExpr{containing BasicToken with int value of 5},
//      {"lims" :   Composite SUPExpr{
//                      Composite SUPExpr{
//                          Leaf SupExpr{int token 0},
//                          Leaf SupExpr{int token 1},
//                    },
//                      Composite SUPExpr{
//                          Leaf SupExpr{int token 2},
//                          Leaf SupExpr{double token -3,5},
//                          Leaf SupExpr{string token "hello"},
//                    },
//                }
//      }
// }



using SUP_Expr = CompositeNode<BasicToken>;
SV_DECL_ALIASES(SUP_Expr);

namespace sup_expr_helpers
{
template<typename... ChildrenArgs>
inline SUP_Expr comp(ChildrenArgs... children)
{
    return SUP_Expr(std::vector<SUP_Expr>{children...});
}
inline SUP_Expr leaf(BasicToken token)
{
    return SUP_Expr(std::move(token));
}
};

using SUP_NamedExpr = std::pair<QString /*name*/, SUP_Expr>;
SV_DECL_ERR(SUP_NamedExpr);

class SUP_VariablesDict;

struct SUP_Arglist
{
public:
    std::string toString() const;

    bool operator==(const SUP_Arglist& other) const;

    //you dont have to pass name, but if you do AND expr on this index
    //also has a name, then you only get result if names match
    const SUP_Expr* getArg(int index, QStringOpt optionalName = {}) const;

    const SUP_Expr* getArgByName(const QString& name) const;

    int getNamedArgIndex(const QString& name) const;

    bool allArgsAreNamed() const;

    StringErrOpt replaceAllSymbolTokensWithDictEntries(const SUP_VariablesDict& dict);

public:
    //every expression may or may not have name.
    std::vector<SUP_NamedExpr> namedExpressions;
};
SV_DECL_STD_FORMATTER(SUP_Arglist, obj.toString());
SV_DECL_ERR(SUP_Arglist)