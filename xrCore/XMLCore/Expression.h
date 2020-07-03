//Giperion May 2018
//[EUREKA] 3.6
//X-Ray Oxygen, Oxygen Team
#pragma once

typedef float(*GetFloatFunc)();
typedef bool(*GetBoolFunc)();
typedef int(*GetIntFunc)();
typedef LPCSTR(*GetStringFunc)();

typedef u64 ExpressionData;

enum eVariableType : u8
{
    eDT_FLOAT,
    eDT_INT,
    eDT_BOOL,
    eDT_STRING
};

struct SXmlExpressionDelegate
{
    static u32 IdGenerator;

    u32             Id;
    shared_str      Name;
    void*           Func;
    eVariableType   Type;

    SXmlExpressionDelegate(shared_str& InName, void* InFunc, eVariableType InType)
        : Name(InName), Func(InFunc), Type(InType)
    {
        Id = ++IdGenerator;
    }
};

union ExpressionVarVariadic
{
    float       Flt;
    int         Int;
    u64         LongInt;
    shared_str  Str;
    bool        Boolean;
    void*       Ptr;

    ExpressionVarVariadic()
        : Ptr(nullptr)
    {}

    ExpressionVarVariadic(float InFlt)
        : Flt(InFlt)
    {}

    ExpressionVarVariadic(int InInt)
        : Int(InInt)
    {}

    ExpressionVarVariadic(bool InBoolean)
        : Boolean(InBoolean)
    {}

    ExpressionVarVariadic(LPCSTR InStr)
        : Str(InStr)
    {}

    ~ExpressionVarVariadic()
    {}

    ExpressionVarVariadic& operator=(const ExpressionVarVariadic& other)
    {
        Ptr = other.Ptr;
        return *this;
    }

    ExpressionVarVariadic(const ExpressionVarVariadic& Other)
    {
        Ptr = Other.Ptr;
    }

    u64 GetData() const
    {
        return LongInt;
    }
};

static_assert(sizeof(ExpressionVarVariadic) == 8); //ExpressionVarVariadic must be placed in 8-aligned expression stack

class XRCORE_API CExpressionManager
{
public:
    static const u32 INVALID_VARIABLE_INDEX = (u32)-1;

    void RegisterVariable(shared_str Name, GetFloatFunc delegate);
    void RegisterVariable(shared_str Name, GetIntFunc delegate);
    void RegisterVariable(shared_str Name, GetStringFunc delegate);

    u32 GetVariableIdByName(shared_str Name);

    ExpressionVarVariadic GetVariableById(int Id);

    SXmlExpressionDelegate* GetVariableDescById(int Id);

private:
    xr_map<int, SXmlExpressionDelegate> m_delegates;
};

extern XRCORE_API CExpressionManager g_uiExpressionMgr;

enum ExpressionByteCode
{
    UI_NONE, //mark as end
    UI_CONSTANT_INT,
    UI_CONSTANT_FLOAT,
    UI_CONSTANT_STRING,
    UI_VARIABLE,
    UI_ADD,
    UI_ADD_INTEGER,
    UI_ADD_FLOAT,
    UI_SUBTRACT,
    UI_SUBTRACT_INTEGER,
    UI_SUBTRACT_FLOAT,
    UI_MULTIPLE,
    UI_MULTIPLE_INTEGER,
    UI_MULTIPLE_FLOAT,
    UI_DIVIDE,
    UI_DIVIDE_INTEGER,
    UI_DIVIDE_FLOAT,
    UI_FLOOR,
    UI_CEIL,
};

class XRCORE_API CExpression
{
public:
    CExpression();
    ~CExpression();
    void CompileExpression(xr_string& ExpressionStr);

    ExpressionVarVariadic ExecuteExpression();

    bool IsCompiled() const;

private:

    ExpressionByteCode GetBytecodeByFunctionName(xr_string& FunctionName);

    void FailCompileWithReason(xr_string& reason) const;
    void FailCompileWithReason(LPCSTR reason) const;
    void FailCompileWithReason() const;
    bool IsValidFloatConstantDeclaration(xr_string& LexemStr) const;
    bool IsValidIntConstantDeclaration(xr_string& LexemStr) const;

    void SetCompileError(LPCSTR reason) const;
    void FlushCompileError();
    
    xr_string m_originalExpression;
    ExpressionData* m_expression;
    mutable char* m_dbgCompileError;
};