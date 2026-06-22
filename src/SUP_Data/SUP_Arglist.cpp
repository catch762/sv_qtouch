#include "SUP_Arglist.h"
#include "SUP_Data.h"

std::string SUP_Arglist::toString() const
{
    if (namedExpressions.empty()) return "SUP_Arglist{}";

    std::string res = "SUP_Arglist{\n";
    int idx = 0;
    for (auto& expr : namedExpressions)
    {
        res += std::format("{}) \"{}\": {}\n", idx, expr.first, expr.second);
        idx++;
    }
    res += "}";
    return res;
    //return std::format("{}", namedExpressions);
}

bool SUP_Arglist::operator==(const SUP_Arglist& other) const
{
    if (namedExpressions.size() != other.namedExpressions.size()) return false;

    for (int i = 0; i < namedExpressions.size(); ++i)
    {
        const SUP_NamedExpr& thisVal  = namedExpressions[i];
        const SUP_NamedExpr& otherVal = other.namedExpressions[i];
        if (thisVal.first  != otherVal.first) return false;
        if (thisVal.second != otherVal.second) return false;
    }

    return true;
}

const SUP_Expr* SUP_Arglist::getArg(int index, QStringOpt optionalName) const
{
    if (!isValidIndex(index, namedExpressions.size()))
    {
        return nullptr;
    }
    const auto& namedExpr = namedExpressions[index];
    const QString& exprName = namedExpr.first;
    if (!exprName.isEmpty() && optionalName && *optionalName != exprName)
    {
        return nullptr;
    }
    return &namedExpr.second;
}

const SUP_Expr *SUP_Arglist::getArgByName(const QString &name) const
{
    int idx = getNamedArgIndex(name);
    if (idx == -1) return nullptr;
    else return &namedExpressions[idx].second;
}

int SUP_Arglist::getNamedArgIndex(const QString& name) const
{
    auto found = std::find_if(namedExpressions.begin(), namedExpressions.end(), [&](auto &e){ return e.first == name; });
    if (found == namedExpressions.end()) return -1;
    else return std::distance(namedExpressions.begin(), found);
}

bool SUP_Arglist::allArgsAreNamed() const
{
    return std::all_of(namedExpressions.begin(), namedExpressions.end(), [](const auto& item)
    {
        return !item.first.isEmpty();
    });
}

StringErrOpt SUP_Arglist::replaceAllSymbolTokensWithDictEntries(const SUP_VariablesDict& dict)
{
    StringSet notFoundNames;

    for(auto& namedExpr : namedExpressions)
    {
        if (auto notFoundNamesForThis = dict.replaceAllSymbolTokensWithDictEntries(namedExpr.second))
        {
            notFoundNames.merge(*notFoundNamesForThis);
        }
    }

    if (notFoundNames.empty()) return {}; //success
    else return std::format("Failed to form valid SUP_Arglist, following SUP_VariablesDict entries were not found: {}", notFoundNames);
}