#pragma once
#include "sv_qtcommon.h"
#include "BasicTokenizer/BasicTokenizer.h"
#include "SUP_Arglist.h"


BasicTokenizer& SUP_getBasicTokenizerForUiString();

class SUP_ArglistParser
{
public:
    SUP_ArglistOrError parseToArglist(const UiMacroString& uiMacroString);
    SUP_ArglistOrError parseToArglistAndReplaceSymbolTokensWithDictEntries(const UiMacroString& uiMacroString, const SUP_VariablesDict& dict);

private:
    //outLastParsedTokenIndex is only valid if function succeeded
    SUP_NamedExprOrError tryParseNextPotentiallyNamedExpr(const BasicTokenVec& tokens, int startIndex, int &outLastParsedTokenIndex);
    SUP_ExprOrError      tryParseNextExpr                (const BasicTokenVec& tokens, int startIndex, int &outLastParsedTokenIndex);

    bool isEquals(const BasicToken& token) const;
    bool isComma(const BasicToken& token) const;
    bool isMinus(const BasicToken& token) const;
    bool isArrayOpenBracket(const BasicToken& token) const;
    bool isArrayCloseBracket(const BasicToken& token) const;

private:
};