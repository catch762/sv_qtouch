#include "SUP_DataParser.h"
#include "StdFormattersForQt.h"
#include "Formatters.h"
#include <QScopeGuard>


SUP_DataOpt SUP_DataParser::parseFiles(const std::vector<QString>& filePaths)
{
    //No matter what, after this operation, class will be in clean state, ready for next operation
    auto onExitFromThisFunction = qScopeGuard([this]
    {
        resetState();
    });

    for (const auto& filePath : filePaths)
    {
        ParseFileResult result = ParseFile(filePath);

        if (result == ParseFileResult::Error)
        {
            SV_ERROR(std::format("SUP_DataParser: parseFile({}) triggered error, terminating parsing.", filePath));
            return {};
        }
        else if (result == ParseFileResult::FileProcessedAndEverythingIsFinished)
        {
            SV_LOG(std::format("SUP_DataParser parse {} Files operation succeeded, data is obtained.", filePaths.size()));
            return std::move(parseResult);
        }
    }

    SV_ASSERT(state != State::FinishedParsingVarList);

    SV_ERROR(std::format("SUP_DataParser parsed all files without errors, but could not obtain data we are looking for. File list: {}",
                            filePaths));
    return {};
}

SUP_DataParser::ParseFileResult SUP_DataParser::ParseFile(const QString& filePath)
{
    if (state == State::FinishedParsingVarList)
    {
        SV_LOG("SUP_DataParser already finished processing, so ParseFile command is ignored.");
        return ParseFileResult::FileProcessedAndEverythingIsFinished;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        SV_ERROR(std::format("SUP_DataParser: ParseFile failed, cannot open file: {}", filePath));
        return ParseFileResult::Error;
    }

    int linesProcessed = 0;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        if (!processLine(in.readLine()))
        {
            SV_ERROR("SUP_DataParser: ParseFile failed and terminated.");
            return ParseFileResult::Error;
        }

        linesProcessed++;

        if (state == State::FinishedParsingVarList)
        {
            SV_LOG(std::format("SUP_DataParser, during processing a file, successfully obtained all the data we wanted.", linesProcessed));
            return ParseFileResult::FileProcessedAndEverythingIsFinished;
        }
    }

    return ParseFileResult::FileProcessed;
};

bool SUP_DataParser::processLine(QString line)
{
    line = line.trimmed();

    //SV_LOG(std::format("LINE: {}", line));

    if (state == State::LookingForStructDeclOrVarList)
    {
        if (lineIsStructDeclarationBegin(line))
        {
            return processStructDeclBegin(line);
        }
        else if (lineIsVarListBegin(line))
        {
            state = State::ParsingEntriesOfVarList;
            return true;
        }
        else return true;
    }
    else if (state == State::ParsingMemberVariablesOfStructDecl)
    {
        return processStructMemberLine(line);
    }
    else if (state == State::ParsingEntriesOfVarList)
    {
        return processVarListEntryLine(line);
    }
    else if (state == State::FinishedParsingVarList)
    {
        SV_WARN("Parsing line requested, but SUP_DataParser already finished processing. Doing nothing.");
        return true;
    }
    else SV_UNREACHABLE();
}

bool SUP_DataParser::lineIsStructDeclarationBegin(const QString& line)
{
    return line.startsWith(StructDeclBegin);
}
bool SUP_DataParser::lineIsVarListBegin(const QString& line)
{
    return line.startsWith(VarListBegin);
}
bool SUP_DataParser::lineIsVarListEnd(const QString& line)
{
    return line.startsWith(VarListEnd);
}



bool SUP_DataParser::processVarListEntryLine(const QString& line)
{
    if (lineIsVarListEnd(line))
    {
        state = State::FinishedParsingVarList;
        return true;
    }

    if (line.startsWith(VarListEntrySkipText))
    {
        return true;
    }

    auto entry = parseVarListEntryLine(line);
    if(!entry) return false;

    parseResult.varListEntries.push_back(*entry);

    //SV_LOG("Saving varlist entry: " + entry->toString());
    return true;
}

SUP_VariableOpt SUP_DataParser::parseVarListEntryLine(const QString& line)
{
    auto parts = splitStringBySeparators(line, {"(", ",", ")"}, true);
    if (!parts)
    {
        SV_ERROR("SUP_DataParser failed to process VarList entry line.");
        return {};
    }
    SV_ASSERT(parts->size()==4);

    const auto& macroName = (*parts)[0];
    const auto& varType   = (*parts)[1];
    const auto& varName   = (*parts)[2];
    const auto& theRest   = (*parts)[3];

    //1. macro type
    //actually we ignore it.

    //2. var type and name
    if (varType.isEmpty())
    {
        SV_ERROR(std::format("Empty var type within VarList entry line [{}]", line));
        return {};
    }
    if (varName.isEmpty())
    {
        SV_ERROR(std::format("Empty var name within VarList entry line [{}]", line));
        return {};
    }

    return SUP_Variable{varType, varName, tryParseUIMacroContent(theRest)};
}

bool SUP_DataParser::processStructDeclBegin(const QString& line)
{
    auto structName = tryExtractStructNameFromDeclBeginLine(line);
    if (!structName) return false;

    currentStruct.name = *structName;
    state = State::ParsingMemberVariablesOfStructDecl;

    //SV_LOG("FOUND STRUCT: " + currentStruct.name.toStdString());
    return true;
}

QStringOpt SUP_DataParser::tryExtractStructNameFromDeclBeginLine(const QString& line)
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

bool SUP_DataParser::processStructMemberLine(const QString& line)
{
    bool isLastMember;
    auto member = parseStructMemberLine(line, isLastMember);

    // Ok this is a bit tricky, on what we do on all combinations of 'member' and 'isLastMember'.
    // I feel like doing this:

    if (member)
    {
        currentStruct.members.push_back(*member);
    }

    if (isLastMember)
    {
        state = State::LookingForStructDeclOrVarList;

        if (!currentStruct.isValid())
        {
            SV_ERROR(std::format("Finalized parsing struct [{}] members, but resulting struct is not valid", currentStruct.name));
            return false;
        }

        if (parseResult.getStruct(currentStruct.name) != nullptr)
        {
            SV_ERROR(std::format("Finalized parsing struct [{}] members, but such struct name already exists", currentStruct.name));
            return false;
        }

        parseResult.structDefinitions[currentStruct.name] = currentStruct;

        currentStruct = SUP_StructDefinition();
    }

    if(!member) return false;

    //SV_LOG(member->toString());
    return true;
}

SUP_VariableOpt SUP_DataParser::parseStructMemberLine(const QString& line, bool& out_isLastMember)
{
    // Input looks like this:
    //
    // Ordinary line:
    //      float, manphase, ui("...")
    // Last line, note the ) symbol instead of comma:
    //      ivec3, renmode) ui("...")

    out_isLastMember = false;

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
        out_isLastMember = true;
    }

    const auto variableName = theRest.left(secondSep).trimmed();
    if (variableName.isEmpty())
    {
        onLineError("Bad variableName content", line);
        return {};
    }

    //everything after second separator; it may optionally contain the ui("...") macro part; it may also be empty
    const auto lastPart = theRest.mid(secondSep+1);

    return SUP_Variable{variableType, variableName, tryParseUIMacroContent(theRest)};;
}

void SUP_DataParser::onLineError(const QString &error, const QString &line)
{
    SV_ERROR(std::format("Error [{}] on a line: {}", error, line));
}

QStringOpt SUP_DataParser::tryParseUIMacroContent(const QString& text)
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

void SUP_DataParser::resetState()
{
    state = State::LookingForStructDeclOrVarList;
    currentStruct = SUP_StructDefinition();
    parseResult = SUP_Data();
}
