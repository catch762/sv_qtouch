#include "GlslStructParser.h"
#include "StdFormattersForQt.h"
#include "Formatters.h"
#include <QScopeGuard>

std::string GlslStructParser::VarListEntry::toString() const
{
    return std::format("VarListEntry[{} {} {} ui({})]",
                                macroType == MacroType::ScalarVariable ? "Scalar" : "Struct",
                                varType, varName, uiMacroArg);
}

std::string GlslStructParser::StructMember::toString() const
{
    return std::format("[{} {} ui({})]", varType, varName, uiMacroArg);
}

std::string GlslStructParser::StructDefinition::toString() const
{
    auto res = std::format("struct {} {{\n", name);
    for (auto &m : members)
    {
        res += std::format("    {}\n", m);
    }
    res += "}";

    return res;
}

bool GlslStructParser::StructDefinition::isValid() const
{
    return !name.isEmpty() && !members.empty();
}

std::string GlslStructParser::ParseResult::toString() const
{
    std::string res = std::format(  "GlslStructParser::ParseResult BEGIN [\n"
                                    "Struct Definitions ({}):\n", structDefinitions.size());
    for (auto &structDef : structDefinitions)
    {
        res += structDef.toString() + "\n";
    }

    res += std::format("--------------\nVarList entries ({}):\n", varListEntries.size());

    for (auto &varlistEntry : varListEntries)
    {
        res += varlistEntry.toString() + "\n";
    }

    res += "] GlslStructParser::ParseResult END";
    return res;
}

GlslStructParser::ParseResultOpt GlslStructParser::parseFiles(const std::vector<QString>& filePaths)
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
            SV_ERROR(std::format("GlslStructParser: parseFile({}) triggered error, terminating parsing.", filePath));
            return {};
        }
        else if (result == ParseFileResult::FileProcessedAndEverythingIsFinished)
        {
            SV_LOG(std::format("GlslStructParser parse {} Files operation succeeded, data is obtained.", filePaths.size()));
            return std::move(parseResult);
        }
    }

    SV_ASSERT(state != State::FinishedParsingVarList);

    SV_ERROR(std::format("GlslStructParser parsed all files without errors, but could not obtain data we are looking for. File list: {}",
                            filePaths));
    return {};
}

GlslStructParser::ParseFileResult GlslStructParser::ParseFile(const QString& filePath)
{
    if (state == State::FinishedParsingVarList)
    {
        SV_LOG("GlslStructParser already finished processing, so ParseFile command is ignored.");
        return ParseFileResult::FileProcessedAndEverythingIsFinished;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        SV_ERROR(std::format("GlslStructParser: ParseFile failed, cannot open file: {}", filePath));
        return ParseFileResult::Error;
    }

    int linesProcessed = 0;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        if (!processLine(in.readLine()))
        {
            SV_ERROR("GlslStructParser: ParseFile failed and terminated.");
            return ParseFileResult::Error;
        }

        linesProcessed++;

        if (state == State::FinishedParsingVarList)
        {
            SV_LOG(std::format("GlslStructParser, during processing a file, successfully obtained all the data we wanted.", linesProcessed));
            return ParseFileResult::FileProcessedAndEverythingIsFinished;
        }
    }

    return ParseFileResult::FileProcessed;
};

bool GlslStructParser::processLine(QString line)
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
        SV_WARN("Parsing line requested, but GlslStructParser already finished processing. Doing nothing.");
        return true;
    }
    else SV_UNREACHABLE();
}

bool GlslStructParser::lineIsStructDeclarationBegin(const QString& line)
{
    return line.startsWith(StructDeclBegin);
}
bool GlslStructParser::lineIsVarListBegin(const QString& line)
{
    return line.startsWith(VarListBegin);
}
bool GlslStructParser::lineIsVarListEnd(const QString& line)
{
    return line.startsWith(VarListEnd);
}



bool GlslStructParser::processVarListEntryLine(const QString& line)
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

GlslStructParser::VarListEntryOpt GlslStructParser::parseVarListEntryLine(const QString& line)
{
    auto parts = splitStringBySeparators(line, {"(", ",", ")"}, true);
    if (!parts)
    {
        SV_ERROR("GlslStructParser failed to process VarList entry line.");
        return {};
    }
    SV_ASSERT(parts->size()==4);

    const auto& macroName = (*parts)[0];
    const auto& varType   = (*parts)[1];
    const auto& varName   = (*parts)[2];
    const auto& theRest   = (*parts)[3];

    //1. macro type
    VarListEntry res;
    if (macroName == VarListEntryVarMacro)
    {
        res.macroType = VarListEntry::MacroType::ScalarVariable;
    }
    else if (macroName == VarListEntryStructMacro)
    {
        res.macroType = VarListEntry::MacroType::Struct;
    }
    else
    {
        SV_ERROR(std::format("Unrecognized macro [{}] of VarList entry line [{}]", macroName, line));
        return {};
    }

    //2. var type
    if (varType.isEmpty())
    {
        SV_ERROR(std::format("Empty var type within VarList entry line [{}]", line));
        return {};
    }
    else
    {
        res.varType = varType;
    }

    //3. var name
    if (varName.isEmpty())
    {
        SV_ERROR(std::format("Empty var name within VarList entry line [{}]", line));
        return {};
    }
    else
    {
        res.varName = varName;
    }

    //4. optional ui macro arg
    res.uiMacroArg = tryParseUIMacroContent(theRest);

    return res;
}

bool GlslStructParser::processStructDeclBegin(const QString& line)
{
    auto structName = tryExtractStructNameFromDeclBeginLine(line);
    if (!structName) return false;

    currentStruct.name = *structName;
    state = State::ParsingMemberVariablesOfStructDecl;

    //SV_LOG("FOUND STRUCT: " + currentStruct.name.toStdString());
    return true;
}

QStringOpt GlslStructParser::tryExtractStructNameFromDeclBeginLine(const QString& line)
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

bool GlslStructParser::processStructMemberLine(const QString& line)
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

        if (currentStruct.isValid())
        {
            parseResult.structDefinitions.push_back(currentStruct);
            //SV_LOG(std::format("Parsed and saved struct: {}", currentStruct));
        }
        else
        {
            SV_ERROR("Finalized parsing struct members, but resulting struct is not valid, so saving nothing.");
        }

        currentStruct = StructDefinition();
    }

    if(!member) return false;

    //SV_LOG(member->toString());
    return true;
}

GlslStructParser::StructMemberOpt GlslStructParser::parseStructMemberLine(const QString& line, bool& out_isLastMember)
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

    StructMember member;
    member.varType = variableType;
    member.varName = variableName;
    member.uiMacroArg = tryParseUIMacroContent(theRest);

    return member;
}

void GlslStructParser::onLineError(const QString &error, const QString &line)
{
    SV_ERROR(std::format("Error [{}] on a line: {}", error, line));
}

QStringOpt GlslStructParser::tryParseUIMacroContent(const QString& text)
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

void GlslStructParser::resetState()
{
    state = State::LookingForStructDeclOrVarList;
    currentStruct = StructDefinition();
    parseResult = ParseResult();
}
