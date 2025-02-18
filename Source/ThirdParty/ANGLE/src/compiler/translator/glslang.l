/*
//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

This file contains the Lex specification for GLSL ES.
Based on ANSI C grammar, Lex specification:
http://www.lysator.liu.se/c/ANSI-C-grammar-l.html

IF YOU MODIFY THIS FILE YOU ALSO NEED TO RUN generate_parser.sh,
WHICH GENERATES THE GLSL ES LEXER (glslang_lex.cpp).
*/

%top{
//
// Copyright (c) 2012-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// This file is auto-generated by generate_parser.sh. DO NOT EDIT!

// Ignore errors in auto-generated code.
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#elif defined(_MSC_VER)
#pragma warning(disable: 4005)
#pragma warning(disable: 4065)
#pragma warning(disable: 4189)
#pragma warning(disable: 4244)
#pragma warning(disable: 4505)
#pragma warning(disable: 4701)
#pragma warning(disable: 4702)
#endif
}

%{
#include "compiler/translator/glslang.h"
#include "compiler/translator/ParseContext.h"
#include "compiler/preprocessor/Token.h"
#include "compiler/translator/util.h"
#include "compiler/translator/length_limits.h"
#include "glslang_tab.h"

/* windows only pragma */
#ifdef _MSC_VER
#pragma warning(disable : 4102)
#endif

// Workaround for flex using the register keyword, deprecated in C++11.
#ifdef __cplusplus
#if __cplusplus > 199711L
#define register
#endif
#endif

#define YY_USER_ACTION                                 \
    yylloc->first_file = yylloc->last_file = yycolumn; \
    yylloc->first_line = yylloc->last_line = yylineno;

#define YY_INPUT(buf, result, max_size) \
    result = string_input(buf, max_size, yyscanner);

static yy_size_t string_input(char* buf, yy_size_t max_size, yyscan_t yyscanner);
static int check_type(yyscan_t yyscanner);
static int reserved_word(yyscan_t yyscanner);
static int ES2_reserved_ES3_keyword(TParseContext *context, int token);
static int ES2_keyword_ES3_reserved(TParseContext *context, int token);
static int ES2_ident_ES3_keyword(TParseContext *context, int token);
static int uint_constant(TParseContext *context);
static int int_constant(yyscan_t yyscanner);
static int float_constant(yyscan_t yyscanner);
static int floatsuffix_check(TParseContext* context);
%}

%option noyywrap nounput never-interactive
%option yylineno reentrant bison-bridge bison-locations
%option extra-type="TParseContext*"
%x FIELDS

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
O           [0-7]

%%

%{
    TParseContext* context = yyextra;
%}

"invariant"    { return INVARIANT; }
"highp"        { return HIGH_PRECISION; }
"mediump"      { return MEDIUM_PRECISION; }
"lowp"         { return LOW_PRECISION; }
"precision"    { return PRECISION; }

"attribute"    { return ES2_keyword_ES3_reserved(context, ATTRIBUTE); }
"const"        { return CONST_QUAL; }
"uniform"      { return UNIFORM; }
"varying"      { return ES2_keyword_ES3_reserved(context, VARYING); }

"break"        { return BREAK; }
"continue"     { return CONTINUE; }
"do"           { return DO; }
"for"          { return FOR; }
"while"        { return WHILE; }

"if"           { return IF; }
"else"         { return ELSE; }
"switch"       { return ES2_reserved_ES3_keyword(context, SWITCH); }
"case"         { return ES2_ident_ES3_keyword(context, CASE); }
"default"      { return ES2_reserved_ES3_keyword(context, DEFAULT); }

"centroid"     { return ES2_ident_ES3_keyword(context, CENTROID); }
"flat"         { return ES2_reserved_ES3_keyword(context, FLAT); }
"smooth"       { return ES2_ident_ES3_keyword(context, SMOOTH); }

"in"           { return IN_QUAL; }
"out"          { return OUT_QUAL; }
"inout"        { return INOUT_QUAL; }

"float"        { return FLOAT_TYPE; }
"int"          { return INT_TYPE; }
"uint"         { return ES2_ident_ES3_keyword(context, UINT_TYPE); }
"void"         { return VOID_TYPE; }
"bool"         { return BOOL_TYPE; }
"true"         { yylval->lex.b = true;  return BOOLCONSTANT; }
"false"        { yylval->lex.b = false; return BOOLCONSTANT; }

"discard"      { return DISCARD; }
"return"       { return RETURN; }

"mat2"         { return MATRIX2; }
"mat3"         { return MATRIX3; }
"mat4"         { return MATRIX4; }

"mat2x2"         { return ES2_ident_ES3_keyword(context, MATRIX2); }
"mat3x3"         { return ES2_ident_ES3_keyword(context, MATRIX3); }
"mat4x4"         { return ES2_ident_ES3_keyword(context, MATRIX4); }

"mat2x3"         { return ES2_ident_ES3_keyword(context, MATRIX2x3); }
"mat3x2"         { return ES2_ident_ES3_keyword(context, MATRIX3x2); }
"mat2x4"         { return ES2_ident_ES3_keyword(context, MATRIX2x4); }
"mat4x2"         { return ES2_ident_ES3_keyword(context, MATRIX4x2); }
"mat3x4"         { return ES2_ident_ES3_keyword(context, MATRIX3x4); }
"mat4x3"         { return ES2_ident_ES3_keyword(context, MATRIX4x3); }

"vec2"         { return VEC2; }
"vec3"         { return VEC3; }
"vec4"         { return VEC4; }
"ivec2"        { return IVEC2; }
"ivec3"        { return IVEC3; }
"ivec4"        { return IVEC4; }
"bvec2"        { return BVEC2; }
"bvec3"        { return BVEC3; }
"bvec4"        { return BVEC4; }
"uvec2"        { return ES2_ident_ES3_keyword(context, UVEC2); }
"uvec3"        { return ES2_ident_ES3_keyword(context, UVEC3); }
"uvec4"        { return ES2_ident_ES3_keyword(context, UVEC4); }

"sampler2D"            { return SAMPLER2D; }
"samplerCube"          { return SAMPLERCUBE; }
"samplerExternalOES"   { return SAMPLER_EXTERNAL_OES; }
"sampler3D"            { return ES2_reserved_ES3_keyword(context, SAMPLER3D); }
"sampler3DRect"        { return ES2_reserved_ES3_keyword(context, SAMPLER3DRECT); }
"sampler2DRect"        { return SAMPLER2DRECT; }
"sampler2DArray"       { return ES2_ident_ES3_keyword(context, SAMPLER2DARRAY); }
"isampler2D"           { return ES2_ident_ES3_keyword(context, ISAMPLER2D); }
"isampler3D"           { return ES2_ident_ES3_keyword(context, ISAMPLER3D); }
"isamplerCube"         { return ES2_ident_ES3_keyword(context, ISAMPLERCUBE); }
"isampler2DArray"      { return ES2_ident_ES3_keyword(context, ISAMPLER2DARRAY); }
"usampler2D"           { return ES2_ident_ES3_keyword(context, USAMPLER2D); }
"usampler3D"           { return ES2_ident_ES3_keyword(context, USAMPLER3D); }
"usamplerCube"         { return ES2_ident_ES3_keyword(context, USAMPLERCUBE); }
"usampler2DArray"      { return ES2_ident_ES3_keyword(context, USAMPLER2DARRAY); }
"sampler2DShadow"      { return ES2_reserved_ES3_keyword(context, SAMPLER2DSHADOW); }
"samplerCubeShadow"    { return ES2_ident_ES3_keyword(context, SAMPLERCUBESHADOW); }
"sampler2DArrayShadow" { return ES2_ident_ES3_keyword(context, SAMPLER2DARRAYSHADOW); }

"struct"       { return STRUCT; }

"layout"  { return ES2_ident_ES3_keyword(context, LAYOUT); }

    /* Reserved keywords for GLSL ES 3.00 that are not reserved for GLSL ES 1.00 */
"coherent"          |
"restrict"          |
"readonly"          |
"writeonly"         |
"resource"          |
"atomic_uint"       |
"noperspective"     |
"patch"             |
"sample"            |
"subroutine"        |
"common"            |
"partition"         |
"active"            |

"filter"            |
"image1D"           |
"image2D"           |
"image3D"           |
"imageCube"	        |
"iimage1D"          |
"iimage2D"          |
"iimage3D"          |
"iimageCube"        |
"uimage1D"          |
"uimage2D"          |
"uimage3D"          |
"uimageCube"        |
"image1DArray"      |
"image2DArray"      |
"iimage1DArray"     |
"iimage2DArray"     |
"uimage1DArray"     |
"uimage2DArray"     |
"image1DShadow"     |
"image2DShadow"     |
"image1DArrayShadow" |
"image2DArrayShadow" |
"imageBuffer"       |
"iimageBuffer"      |
"uimageBuffer"      |

"sampler1DArray"    |
"sampler1DArrayShadow" |
"isampler1D"        |
"isampler1DArray"   |
"usampler1D"        |
"usampler1DArray"   |
"isampler2DRect"    |
"usampler2DRect"    |
"samplerBuffer"     |
"isamplerBuffer"    |
"usamplerBuffer"    |
"sampler2DMS"       |
"isampler2DMS"      |
"usampler2DMS"      |
"sampler2DMSArray"  |
"isampler2DMSArray" |
"usampler2DMSArray" { 
    if (context->shaderVersion < 300) {
		yylval->lex.string = NewPoolTString(yytext); 
	    return check_type(yyscanner); 
	}
	return reserved_word(yyscanner);
}

    /* Reserved keywords in GLSL ES 1.00 that are not reserved in GLSL ES 3.00 */
"packed"  {
    if (context->shaderVersion >= 300)
    {
        yylval->lex.string = NewPoolTString(yytext);
        return check_type(yyscanner);
    }

    return reserved_word(yyscanner);
}

    /* Reserved keywords */
"asm"          |

"class"        |
"union"        |
"enum"         |
"typedef"      |
"template"     |
"this"         |

"goto"         |

"inline"       |
"noinline"     |
"volatile"     |
"public"       |
"static"       |
"extern"       |
"external"     |
"interface"    |

"long"         |
"short"        |
"double"       |
"half"         |
"fixed"        |
"unsigned"     |
"superp"       |

"input"        |
"output"       |

"hvec2"        |
"hvec3"        |
"hvec4"        |
"dvec2"        |
"dvec3"        |
"dvec4"        |
"fvec2"        |
"fvec3"        |
"fvec4"        |

"sampler1D"    |
"sampler1DShadow" |
"sampler2DRectShadow" |

"sizeof"       |
"cast"         |

"namespace"    |
"using"        { return reserved_word(yyscanner); }

{L}({L}|{D})*       {
   yylval->lex.string = NewPoolTString(yytext); 
   return check_type(yyscanner);
}

0[xX]{H}+         { return int_constant(yyscanner); }
0{O}+             { return int_constant(yyscanner); }
{D}+              { return int_constant(yyscanner); }

0[xX]{H}+[uU]     { return uint_constant(context); }
0{O}+[uU]         { return uint_constant(context); }
{D}+[uU]          { return uint_constant(context); }

{D}+{E}           { return float_constant(yyscanner); }
{D}+"."{D}*({E})? { return float_constant(yyscanner); }
"."{D}+({E})?     { return float_constant(yyscanner); }

{D}+{E}[fF]           { return floatsuffix_check(context); }
{D}+"."{D}*({E})?[fF] { return floatsuffix_check(context); }
"."{D}+({E})?[fF]     { return floatsuffix_check(context); }

"+="            { return ADD_ASSIGN; }
"-="            { return SUB_ASSIGN; }
"*="            { return MUL_ASSIGN; }
"/="            { return DIV_ASSIGN; }
"%="            { return MOD_ASSIGN; }
"<<="           { return LEFT_ASSIGN; }
">>="           { return RIGHT_ASSIGN; }
"&="            { return AND_ASSIGN; }
"^="            { return XOR_ASSIGN; }
"|="            { return OR_ASSIGN; }

"++"            { return INC_OP; }
"--"            { return DEC_OP; }
"&&"            { return AND_OP; }
"||"            { return OR_OP; }
"^^"            { return XOR_OP; }
"<="            { return LE_OP; }
">="            { return GE_OP; }
"=="            { return EQ_OP; }
"!="            { return NE_OP; }
"<<"            { return LEFT_OP; }
">>"            { return RIGHT_OP; }
";"             { return SEMICOLON; }
("{"|"<%")      { return LEFT_BRACE; }
("}"|"%>")      { return RIGHT_BRACE; }
","             { return COMMA; }
":"             { return COLON; }
"="             { return EQUAL; }
"("             { return LEFT_PAREN; }
")"             { return RIGHT_PAREN; }
("["|"<:")      { return LEFT_BRACKET; }
("]"|":>")      { return RIGHT_BRACKET; }
"."             { BEGIN(FIELDS); return DOT; }
"!"             { return BANG; }
"-"             { return DASH; }
"~"             { return TILDE; }
"+"             { return PLUS; }
"*"             { return STAR; }
"/"             { return SLASH; }
"%"             { return PERCENT; }
"<"             { return LEFT_ANGLE; }
">"             { return RIGHT_ANGLE; }
"|"             { return VERTICAL_BAR; }
"^"             { return CARET; }
"&"             { return AMPERSAND; }
"?"             { return QUESTION; }

<FIELDS>{L}({L}|{D})* {
    BEGIN(INITIAL);
    yylval->lex.string = NewPoolTString(yytext);
    return FIELD_SELECTION;
}
<FIELDS>[ \t\v\f\r] {}

[ \t\v\n\f\r] { }
<*><<EOF>>    { yyterminate(); }
<*>.          { return 0; }

%%

yy_size_t string_input(char* buf, yy_size_t max_size, yyscan_t yyscanner) {
    pp::Token token;
    yyget_extra(yyscanner)->preprocessor.lex(&token);
    yy_size_t len = token.type == pp::Token::LAST ? 0 : token.text.size();
    if (len < max_size)
        memcpy(buf, token.text.c_str(), len);
    yyset_column(token.location.file, yyscanner);
    yyset_lineno(token.location.line, yyscanner);

    if (len >= max_size)
        YY_FATAL_ERROR("Input buffer overflow");
    else if (len > 0)
        buf[len++] = ' ';
    return len;
}

int check_type(yyscan_t yyscanner) {
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner;
    
    int token = IDENTIFIER;
    TSymbol* symbol = yyextra->symbolTable.find(yytext, yyextra->shaderVersion);
    if (symbol && symbol->isVariable()) {
        TVariable* variable = static_cast<TVariable*>(symbol);
        if (variable->isUserType()) {
            token = TYPE_NAME;
        }
    }
    yylval->lex.symbol = symbol;
    return token;
}

int reserved_word(yyscan_t yyscanner) {
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner;

    yyextra->error(*yylloc, "Illegal use of reserved word", yytext, "");
    yyextra->recover();
    return 0;
}

int ES2_reserved_ES3_keyword(TParseContext *context, int token)
{
    yyscan_t yyscanner = (yyscan_t) context->scanner;

    if (context->shaderVersion < 300)
    {
        return reserved_word(yyscanner);
    }

    return token;
}

int ES2_keyword_ES3_reserved(TParseContext *context, int token)
{
    yyscan_t yyscanner = (yyscan_t) context->scanner;

    if (context->shaderVersion >= 300)
    {
        return reserved_word(yyscanner);
    }

    return token;
}

int ES2_ident_ES3_keyword(TParseContext *context, int token)
{
    struct yyguts_t* yyg = (struct yyguts_t*) context->scanner;
    yyscan_t yyscanner = (yyscan_t) context->scanner;

    // not a reserved word in GLSL ES 1.00, so could be used as an identifier/type name
    if (context->shaderVersion < 300)
    {
        yylval->lex.string = NewPoolTString(yytext);
        return check_type(yyscanner);
    }

    return token;
}

int uint_constant(TParseContext *context)
{
    struct yyguts_t* yyg = (struct yyguts_t*) context->scanner;
    yyscan_t yyscanner = (yyscan_t) context->scanner;

    if (context->shaderVersion < 300)
    {
        context->error(*yylloc, "Unsigned integers are unsupported prior to GLSL ES 3.00", yytext, "");
        context->recover();
        return 0;
    }

    if (!atoi_clamp(yytext, &(yylval->lex.i)))
        yyextra->warning(*yylloc, "Integer overflow", yytext, "");

    return UINTCONSTANT;
}

int floatsuffix_check(TParseContext* context)
{
    struct yyguts_t* yyg = (struct yyguts_t*) context->scanner;

    if (context->shaderVersion < 300)
    {
        context->error(*yylloc, "Floating-point suffix unsupported prior to GLSL ES 3.00", yytext);
        context->recover();
        return 0;
    }

    if (!atof_clamp(yytext, &(yylval->lex.f)))
        yyextra->warning(*yylloc, "Float overflow", yytext, "");

    return(FLOATCONSTANT);
}

void yyerror(YYLTYPE* lloc, TParseContext* context, void *scanner, const char* reason) {
    context->error(*lloc, reason, yyget_text(scanner));
    context->recover();
}

int int_constant(yyscan_t yyscanner) {
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner;

    if (!atoi_clamp(yytext, &(yylval->lex.i)))
        yyextra->warning(*yylloc, "Integer overflow", yytext, "");
    return INTCONSTANT;
}

int float_constant(yyscan_t yyscanner) {
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner;

    if (!atof_clamp(yytext, &(yylval->lex.f)))
        yyextra->warning(*yylloc, "Float overflow", yytext, "");
    return FLOATCONSTANT;
}

int glslang_initialize(TParseContext* context) {
    yyscan_t scanner = NULL;
    if (yylex_init_extra(context, &scanner))
        return 1;

    context->scanner = scanner;
    return 0;
}

int glslang_finalize(TParseContext* context) {
    yyscan_t scanner = context->scanner;
    if (scanner == NULL) return 0;
    
    context->scanner = NULL;
    yylex_destroy(scanner);

    return 0;
}

int glslang_scan(size_t count, const char* const string[], const int length[],
                 TParseContext* context) {
    yyrestart(NULL, context->scanner);
    yyset_column(0, context->scanner);
    yyset_lineno(1, context->scanner);

    // Initialize preprocessor.
    if (!context->preprocessor.init(count, string, length))
        return 1;

    // Define extension macros.
    const TExtensionBehavior& extBehavior = context->extensionBehavior();
    for (TExtensionBehavior::const_iterator iter = extBehavior.begin();
         iter != extBehavior.end(); ++iter) {
        context->preprocessor.predefineMacro(iter->first.c_str(), 1);
    }
    if (context->fragmentPrecisionHigh)
        context->preprocessor.predefineMacro("GL_FRAGMENT_PRECISION_HIGH", 1);

    context->preprocessor.setMaxTokenSize(GetGlobalMaxTokenSize(context->shaderSpec));

    return 0;
}

