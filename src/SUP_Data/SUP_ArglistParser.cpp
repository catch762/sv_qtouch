#include "SUP_ArglistParser.h"

BasicTokenizer &SUP_getBasicTokenizerForUiString()
{
    static BasicTokenizer tokenizer = []()
    {
        BasicTokenizer tokenizer;

        //yes, only make strings with ', not "
        tokenizer.enableParsingStringTokens("'");
        //however you can escape both ' and " inside of a string
        tokenizer.enableEscapingStringCharachters({
            {'n',  '\n'}, {'t',  '\t'}, {'\"',  '\"'}, {'\'',  '\''}
        });
        //idk, i think its all i need out of all python
        tokenizer.enableParsingSpecialCharachterTokens(",-=[]>");
        //default are fine
        tokenizer.enableBreakCharachters();

        tokenizer.enableParsingNumbers();
        //this will interpret "a-5" as {symbol("a"), int(-5)}, but i dont
        //intend to use these kind of expressions anyway
        tokenizer.enableApplyingMinusCharToNumberTokens();

        return tokenizer;
    }();

    return tokenizer;
}

SUP_ArglistOrError SUP_ArglistParser::parseToArglistWithoutSymbolSubstitutions(const UiMacroString &uiMacroString)
{
    if (uiMacroString.isEmpty())
    {
        return "Cant parse empty uiMacroString";
    }

    auto tokensOrErr = SUP_getBasicTokenizerForUiString().parse(uiMacroString.toStdString());
    if (auto err = getError(tokensOrErr))
    {
        return *err;
    }

    BasicTokenVec& basicTokens = std::get<BasicTokenVec>(tokensOrErr);
    if (basicTokens.empty())
    {
        return "No basic tokens returned by BasicTokenizer";
    }

    int currentTokenIdx = 0;

    SUP_Arglist result;
    while(true)
    {
        int lastParsedIdx;
        auto namedExprOrErr = tryParseNextPotentiallyNamedExpr(basicTokens, currentTokenIdx, lastParsedIdx);
        if (auto err = getError(namedExprOrErr))
        {
            return *err;
        }

        result.namedExpressions.push_back(std::move(std::get<SUP_NamedExpr>(namedExprOrErr)));

        const int remainingTokens = basicTokens.size() - lastParsedIdx - 1;
        if (remainingTokens == 0)
        {
            //well, we are done
            return result;
        }

        const auto& supposedlyNextComma = basicTokens[lastParsedIdx+1];
        if (!isComma(supposedlyNextComma))
        {
            return std::format("After parsing SUP_NamedExpr, found unexpected next token {}, it should be comma", supposedlyNextComma);
        }
        else if (remainingTokens < 2)
        {
            return "After parsing SUP_NamedExpr, found comma, but nothing afterwards";
        }

        //ok, there is comma, there is something else after it, lets loop again and parse next
        currentTokenIdx = lastParsedIdx + 2;
    }

    SV_UNREACHABLE(); 
}

SUP_ArglistOrError SUP_ArglistParser::parseToArglistAndReplaceSymbolTokensWithDictEntries(const UiMacroString &uiMacroString, const SUP_VariablesDict &dict)
{
    /*
    if (uiMacroString.startsWith("lims=camdata")) {
        SV_LOG("ok");
    }
    */

    auto arglistOrErr = parseToArglistWithoutSymbolSubstitutions(uiMacroString);
    if (auto err = getError(arglistOrErr))
    {
        return *err;
    }

    SUP_Arglist& arglist = std::get<0>(arglistOrErr);
    if (auto errOpt = arglist.replaceAllSymbolTokensWithDictEntries(dict))
    {
        return std::format("Parsing arglist succeeded initially, but then dict error happened: {}", *errOpt);
    }
    
    return arglist;
}

SUP_NamedExprOrError SUP_ArglistParser::tryParseNextPotentiallyNamedExpr(const BasicTokenVec &tokens, int startIndex, int &outLastParsedTokenIndex)
{
    // If its named, then, from the startIndex we have at least 3 tokens:
    //      argname = ...one or more tokens...

    auto isNamed = [&]()
    {
        const int maxTokensToParse = tokens.size() - startIndex;
        if (maxTokensToParse >= 3) 
        {
            if (tokens[startIndex].isSymbol() && isEquals(tokens[startIndex+1]))
            {
                return true;
            }
        }
        return false;
    };

    QString exprName = QString();
    int indexToReadExprFrom = startIndex;
    if (isNamed())
    {
        exprName = QString::fromStdString(tokens[startIndex].getSymbolData());
        indexToReadExprFrom += 2;
    }

    auto exprOrError = tryParseNextExpr(tokens, indexToReadExprFrom, outLastParsedTokenIndex);
    if (auto err = getError(exprOrError))
    {
        return *err;
    }

    return SUP_NamedExpr{std::move(exprName), std::move(std::get<SUP_Expr>(exprOrError))};
}

SUP_ExprOrError SUP_ArglistParser::tryParseNextExpr(const BasicTokenVec &tokens, const int startIndex, int &outLastParsedTokenIndex)
{
    //well its either immediate expr, or array.
    //and its either last, and theres nothing else in token list, or theres COMMA(,) and next expression

    if (auto indexErr = getErrorIfInvalidIndex(startIndex, tokens.size(), "SUP_ArglistParser::tryParseNextExpr"))
    {
        return *indexErr;
    }

    const auto& firstToken = tokens[startIndex];

    const bool startsAsLegalNonArrayExpr = firstToken.isSymbol() || firstToken.isString() || firstToken.isNumber() ||
                                           (firstToken.isSpecialCharachter()  && !isArrayOpenBracket(firstToken)
                                                                              && !isArrayCloseBracket(firstToken)
                                                                              && !isComma(firstToken));

    const int remainingTokens = tokens.size() - startIndex;

    if (startsAsLegalNonArrayExpr)
    {
        // If theres normal negative number like "-5" we dont even get minus tokens here, we just get number token of -5.
        // However, there just might be "-varname", which will look like SpecChar(-), Symbol(varname)
        // and in that case we need to handle it as single SUP_Expr with negative flag set.

        if (isMinus(firstToken))
        {
            if (remainingTokens < 1)
            {
                return "Error, was parsing expr, found Minus token, but nothing afterwards";
            }

            const int nextExprIndex = startIndex + 1;
            const auto& nextToken = tokens[nextExprIndex];

            if (!nextToken.isSymbol())
            {
                return std::format("Error, after Minus token, expected Symbol token, but next was {}", nextToken);
            }

            //ok, all good, lets form that negative symbol token
            outLastParsedTokenIndex = nextExprIndex;
            return SUP_Expr(TokenWithModifier(nextToken, true));
        }
        else
        {
            outLastParsedTokenIndex = startIndex;
            return SUP_Expr(firstToken);
        }
    }
    else
    {
        if (!isArrayOpenBracket(firstToken))
        {
            return std::format("Error: was parsing expr, and first token {} is illegal - neither array expr, nor non-array expr",
                                firstToken);
        }
        else if (remainingTokens == 1)
        {
            return "Error parsing expr: Array open bracket was found, but nothing afterwards";
        }

        SUP_Expr arrayExpr = SUP_Expr::makeComposite();

        int nextExprIndex = startIndex + 1;
        while(true)
        {
            if (!isValidIndex(nextExprIndex, tokens.size()))
            {
                return "During parsing array expr, ran out of tokens before finding closing bracket";
            }
            const auto& nextToken = tokens[nextExprIndex];
            if (isArrayCloseBracket(nextToken))
            {
                //we are done, array is parsed
                outLastParsedTokenIndex = nextExprIndex;
                return arrayExpr;
            }

            int lastParsedIdxForChild;
            auto parsedChildOrErr = tryParseNextExpr(tokens, nextExprIndex, lastParsedIdxForChild);
            if (auto err = getError(parsedChildOrErr))
            {
                return *err;
            }

            arrayExpr.getChildren()->push_back(std::move(std::get<SUP_Expr>(parsedChildOrErr)));
            
            outLastParsedTokenIndex = lastParsedIdxForChild;
            nextExprIndex = lastParsedIdxForChild + 1;

            //Ok, we did read expr, now check whats next:
            {
                if (isValidIndex(nextExprIndex, tokens.size()))
                {
                    const auto& tokenCommaOrBracket = tokens[nextExprIndex];

                    if (isComma(tokenCommaOrBracket))
                    {
                        nextExprIndex += 1;
                        outLastParsedTokenIndex += 1;
                    }
                    else if (isArrayCloseBracket(tokenCommaOrBracket))
                    {
                        //do nothing, handle it next loop
                    }
                    else
                    {
                        return std::format("During parsing array expr, after found unexpected {} which is neither ',' nor ']'",
                                                tokenCommaOrBracket);
                    }//todo whats line in orig file? and whats happening now, print all tokens
                }
            }
        }
    }

    SV_UNREACHABLE();
}

bool SUP_ArglistParser::isEquals(const BasicToken &token) const
{
    return token.isSpecialCharachter() && token.getSpecialCharachterData() == '=';
}

bool SUP_ArglistParser::isComma(const BasicToken &token) const
{
    return token.isSpecialCharachter() && token.getSpecialCharachterData() == ',';
}

bool SUP_ArglistParser::isMinus(const BasicToken& token) const
{
    return token.isSpecialCharachter() && token.getSpecialCharachterData() == '-';
}

bool SUP_ArglistParser::isArrayOpenBracket(const BasicToken &token) const
{
    return token.isSpecialCharachter() && token.getSpecialCharachterData() == '[';
}

bool SUP_ArglistParser::isArrayCloseBracket(const BasicToken &token) const
{
    return token.isSpecialCharachter() && token.getSpecialCharachterData() == ']';
}
