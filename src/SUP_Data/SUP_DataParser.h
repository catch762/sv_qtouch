#pragma once
#include "sv_qtcommon.h"

class SUP_DataParser
{
public:
    static void Test()
    {
        SUP_DataParser parser;
        auto res = parser.parseFiles({"C:/home/code/sv_qtouch/glsl_example.h"});

        SV_LOG(std::format("Result: {}", res ? res->toString() : std::string("failure")));
    }
    struct VarListEntry
    {
        enum MacroType
        {
            ScalarVariable, //e.g. 'float'
            Struct
        };

        MacroType   macroType;
        QString     varType;
        QString     varName;
        QStringOpt  uiMacroArg;

        std::string toString() const;
    };
    SV_DECL_OPT(VarListEntry);

    struct StructMember
    {
        QString varType;
        QString varName;
        QStringOpt uiMacroArg; //not including quotes

        std::string toString() const;
    };
    
    SV_DECL_OPT(StructMember);

    struct StructDefinition
    {
        QString name;
        std::vector<StructMember> members;

        std::string toString() const;
        bool isValid() const;
    };

    struct ParseResult
    {
        std::vector<StructDefinition>   structDefinitions;
        std::vector<VarListEntry>       varListEntries;

        std::string toString() const;
    };
    SV_DECL_OPT(ParseResult);

    ParseResultOpt parseFiles(const std::vector<QString>& filePaths);

private:
    enum State
    {
        LookingForStructDeclOrVarList,
        ParsingMemberVariablesOfStructDecl,
        ParsingEntriesOfVarList,
        FinishedParsingVarList //this means entire parsing operation is over, we are done with the file.
    };

    enum ParseFileResult
    {
        Error,
        FileProcessed,
        FileProcessedAndEverythingIsFinished
    };
    ParseFileResult ParseFile(const QString& filePath);

    //returns success; failure will halt entire parsing operation.
    bool processLine(QString line);

    bool lineIsStructDeclarationBegin(const QString& line);
    bool lineIsVarListBegin(const QString& line);
    bool lineIsVarListEnd(const QString& line);

    

    bool processVarListEntryLine(const QString& line);

    // Example lines:
    //      VAR(vec4, te)		ui("...")
	//	    STR(RenFin, renfin) ui("...")
    VarListEntryOpt parseVarListEntryLine(const QString& line);

    bool processStructDeclBegin(const QString& line);

    QStringOpt tryExtractStructNameFromDeclBeginLine(const QString& line);

    bool processStructMemberLine(const QString& line);

    StructMemberOpt parseStructMemberLine(const QString& line, bool& out_isLastMember);

    void onLineError(const QString &error, const QString &line);

    // If text contains ui("12whatever34"), it will return 12whatever34, without quotes.
    // Warning:
    //  ui( "spaces within macro are ok" )
    //  ui ("but space after ui will NOT work")
    QStringOpt tryParseUIMacroContent(const QString& text);

    void resetState();

private:
    static inline const QString StructDeclBegin         = "#define STRMETA_";
    static inline const QString VarListBegin            = "SUP_VARS_BEGIN";
    static inline const QString VarListEnd              = "SUP_VARS_END";
    static inline const QString VarListEntryStructMacro = "STR";
    static inline const QString VarListEntryVarMacro    = "VAR";
    static inline const QString VarListEntrySkipText    = "#line";
    static inline const QString UiBegin                 = "ui(";
    static inline const QChar   Comma                   = ',';

private:
    State state = State::LookingForStructDeclOrVarList;
    
    StructDefinition currentStruct; //if we are in State::ParsingMemberVariablesOfStructDecl, we are filling this.

    ParseResult parseResult;
};

SV_DECL_STD_FORMATTER(SUP_DataParser::VarListEntry, obj.toString());
SV_DECL_STD_FORMATTER(SUP_DataParser::StructMember, obj.toString());
SV_DECL_STD_FORMATTER(SUP_DataParser::StructDefinition, obj.toString());
SV_DECL_STD_FORMATTER(SUP_DataParser::ParseResult, obj.toString());