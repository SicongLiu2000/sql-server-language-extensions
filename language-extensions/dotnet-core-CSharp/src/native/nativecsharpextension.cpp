#include "nativecsharpextension.h"
#include "Logger.h"

#define nameof(x) #x

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h> // Only needed on Windows
#endif

//--------------------------------------------------------------------------------------------------
// Name: UTF8PtrToStr
//
// Description:
//  Returns the string with the address of the first character and length
//
// Returns:
//  utf8 string
//
std::string UTF8PtrToStr(SQLCHAR* str, SQLULEN len)
{
    return std::string(reinterpret_cast<char*>(str), len);
}

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//  Returns the API interface version for the extension
//
// Returns:
//  EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT GetInterfaceVersion()
{
    return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//  Initializes the .NET Core C# Extension.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
    SQLCHAR  *languageParams,
    SQLULEN  languageParamsLen,
    SQLCHAR  *languagePath,
    SQLULEN  languagePathLen,
    SQLCHAR  *publicLibraryPath,
    SQLULEN  publicLibraryPathLen,
    SQLCHAR  *privateLibraryPath,
    SQLULEN  privateLibraryPathLen
)
{
    LOG("nativecsharpextension::Init");
    
    // Create DotnetEnvironment instance
    g_dotnet_runtime = new DotnetEnvironment(
        UTF8PtrToStr(languageParams, languageParamsLen),
        UTF8PtrToStr(languagePath, languagePathLen),
        UTF8PtrToStr(publicLibraryPath, publicLibraryPathLen),
        UTF8PtrToStr(privateLibraryPath, privateLibraryPathLen)
    );

    SQLRETURN err = g_dotnet_runtime->Init();
    return err != 0 ? err :
        g_dotnet_runtime->call_managed_method<decltype(&Init)>(nameof(Init),
            languageParams,
            languageParamsLen,
            languagePath,
            languagePathLen,
            publicLibraryPath,
            publicLibraryPathLen,
            privateLibraryPath,
            privateLibraryPathLen);
}

// --------------------------------------------------------------------------------------------------
// Name: InitSession
//
// Description:
//  Initializes session-specific data. We store the schema, parameter info,
//  output input data info here.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitSession(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT numTasks,
    SQLCHAR      *script,
    SQLULEN      scriptLength,
    SQLUSMALLINT inputSchemaColumnsNumber,
    SQLUSMALLINT parametersNumber,
    SQLCHAR      *inputDataName,
    SQLUSMALLINT inputDataNameLength,
    SQLCHAR      *outputDataName,
    SQLUSMALLINT outputDataNameLength
)
{
    LOG("nativecsharpextension::InitSession");
    return g_dotnet_runtime->call_managed_method<decltype(&InitSession)>(nameof(InitSession),
        sessionId,
        taskId,
        numTasks,
        script,
        scriptLength,
        inputSchemaColumnsNumber,
        parametersNumber,
        inputDataName,
        inputDataNameLength,
        outputDataName,
        outputDataNameLength);
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//  Initializes column-specific data. We store the name and the data type of the column here.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitColumn(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT columnNumber,
    SQLCHAR      *columnName,
    SQLSMALLINT  columnNameLength,
    SQLSMALLINT  dataType,
    SQLULEN      columnSize,
    SQLSMALLINT  decimalDigits,
    SQLSMALLINT  nullable,
    SQLSMALLINT  partitionByNumber,
    SQLSMALLINT  orderByNumber
)
{
    LOG("nativecsharpextension::InitColumn");
    return g_dotnet_runtime->call_managed_method<decltype(&InitColumn)>(nameof(InitColumn),
        sessionId,
        taskId,
        columnNumber,
        columnName,
        columnNameLength,
        dataType,
        columnSize,
        decimalDigits,
        nullable,
        partitionByNumber,
        orderByNumber);
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
//  Initializes parameter-specific data.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitParam(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT paramnumber,
    SQLCHAR      *paramName,
    SQLSMALLINT  paramNameLength,
    SQLSMALLINT  dataType,
    SQLULEN      paramSize,
    SQLSMALLINT  decimalDigits,
    SQLPOINTER   paramValue,
    SQLINTEGER   strLen_or_Ind,
    SQLSMALLINT  inputOutputType
)
{
    LOG("nativecsharpextension::InitParam");
    return g_dotnet_runtime->call_managed_method<decltype(&InitParam)>(nameof(InitParam),
        sessionId,
        taskId,
        paramnumber,
        paramName,
        paramNameLength,
        dataType,
        paramSize,
        decimalDigits,
        paramValue,
        strLen_or_Ind,
        inputOutputType);
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//  Given the data from ExtHost, convert and populate the arrays in the user script. Then,
//  invoke the specified script and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Execute(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLULEN      rowsNumber,
    SQLPOINTER   *data,
    SQLINTEGER   **strLen_or_Ind,
    SQLUSMALLINT *outputSchemaColumnsNumber
)
{
    LOG("nativecsharpextension::Execute");
    return g_dotnet_runtime->call_managed_method<decltype(&Execute)>(nameof(Execute),
        sessionId,
        taskId,
        rowsNumber,
        data,
        strLen_or_Ind,
        outputSchemaColumnsNumber);
}
