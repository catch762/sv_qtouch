#pragma once
#include "sv_qtcommon.h"

class GlslStructParser
{
public:
    static void Test()
    {
        GlslStructParser parser;
        parser.ParseFile("C:/home/code/sv_qtouch/glsl_example.h");
    }

    void ParseFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            SV_ERROR(std::format("GlslStructParser: ParseFile failed, cannot open file: {}", filePath));
            return;
        }

        QTextStream in(&file);
        while (!in.atEnd())
        {
            if (!processLine(in.readLine()))
            {
                SV_ERROR("GlslStructParser: ParseFile failed and terminated.");
            }
        }
    };

private:
    enum State
    {
        LookingForStruct,
        ParsingStructMembers
    };

    //returns success; failure will halt entire parsing operation.
    bool processLine(QString line)
    {
        line = line.trimmed();

        //SV_LOG(std::format("LINE: {}", line));

        if (state == State::LookingForStruct)
        {
            return processPotentialStructDeclBegin(line);
        }
        else if (state == State::ParsingStructMembers)
        {
            return processStructMemberLine(line);
        }
        else SV_UNREACHABLE();
    }

    bool processPotentialStructDeclBegin(const QString& line)
    {
        if( line.startsWith(StructDeclBegin) )
        {
            auto structName = tryExtractStructNameFromDeclBeginLine(line);
            if (!structName) return false;

            currentStructName = *structName;
            state = State::ParsingStructMembers;

            SV_LOG("FOUND STRUCT: " + currentStructName.toStdString());
        }
        return true;
    }

    QStringOpt tryExtractStructNameFromDeclBeginLine(const QString& line)
    {
        int declBeginIndex = line.indexOf(StructDeclBegin);
        if (declBeginIndex == -1)
        {
            onLineError("Didnt find struct declaration begin", line);
            return {};
        }

        QString everythingAfter = line.mid(declBeginIndex + StructDeclBegin.length());
        int parenthIndex = everythingAfter.indexOf('(');
        if (parenthIndex == -1)
        {
            onLineError("After struct declaration begin did not find a ( parenthesis", line);
            return {};
        }

        QString structName = everythingAfter.left(parenthIndex).trimmed();
        if (structName.isEmpty())
        {
            onLineError("Empty struct name", line);
            return {};
        }

        return structName;
    }

    //returns success;
    struct StructMember
    {
        QString varType;
        QString varName;
        QStringOpt uiMacroArg; //not including quotes

        std::string toString()
        {
            return std::format("StructMember[varType={} varName={} ui={}]", varType, varName, uiMacroArg);
        }
    };
    SV_DECL_OPT(StructMember);

    bool processStructMemberLine(const QString& line)
    {
        auto member = parseStructMemberLine(line);
        if(!member) return false;

        SV_LOG(member->toString());
        return true;
    }

    StructMemberOpt parseStructMemberLine(const QString& line)
    {
        // Input looks like this:
        //
        // Ordinary line:
        //      float, manphase, ui("...")
        // Last line, note the ) symbol instead of comma:
        //      ivec3, renmode) ui("...")

        if (!line.contains(Comma))
        {
            onLineError("No ',' commas at all", line);
            return {};
        }

        const auto variableType = line.section(Comma, 0,0).trimmed();
        const auto theRest = line.section(Comma,1).trimmed();
        if (variableType.isEmpty() || theRest.isEmpty())
        {
            onLineError("Bad content", line);
            return {};
        }

        const auto secondSeparators = ",)";
        const auto secondSep = firstPosOfAnyCharFromList(theRest, secondSeparators);
        if(secondSep == -1)
        {
            onLineError(QString("After first comma, there were no separators found: [%1]").arg(secondSeparators), line);
            return {};
        }

        if (theRest[secondSep] == ')')
        {
            //This is last member.
            state = State::LookingForStruct;
        }

        const auto variableName = theRest.left(secondSep).trimmed();
        if (variableName.isEmpty())
        {
            onLineError("Bad variableName content", line);
            return {};
        }

        //everything after second separator; it may optionally contain the ui("...") macro part; it may also be empty
        const auto lastPart = theRest.mid(secondSep+1);

        StructMember member;
        member.varType = variableType;
        member.varName = variableName;
        member.uiMacroArg = tryParseUIMacroContent(theRest);

        return member;
    }

    void onLineError(const QString &error, const QString &line)
    {
        SV_ERROR(std::format("Error [{}] on a line: {}", error, line));
    }

    // If text contains ui("12whatever34"), it will return 12whatever34, without quotes.
    // Warning:
    //  ui( "spaces within macro are ok" )
    //  ui ("but space after ui will NOT work")
    QStringOpt tryParseUIMacroContent(const QString& text)
    {
        auto indexUiBegin = text.indexOf(UiBegin);
        if (indexUiBegin == -1) return {};

        auto theRest = text.mid(indexUiBegin + UiBegin.length()).trimmed();
        if(theRest.isEmpty()) return {};

        std::pair<int,int> quotes = getFirstTwoUnescapedQuotesIndexes(theRest);
        if (quotes.first < 0 || quotes.second < 0) return {};

        auto argumentBetweenQuotes = theRest.mid(quotes.first + 1, quotes.second - quotes.first - 1).trimmed();
        if (argumentBetweenQuotes.isEmpty()) return {};

        return argumentBetweenQuotes;
    }

private:
    static inline const QChar   Comma = ',';
    static inline const QString StructDeclBegin = "#define STRMETA_";
    static inline const QString UiBegin = "ui(";

private:
    State state = State::LookingForStruct;
    QString currentStructName;
};