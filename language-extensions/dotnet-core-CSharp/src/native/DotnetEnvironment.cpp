// Fixing the platform-specific issues
#include "DotnetEnvironment.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <codecvt>
    #include <locale>
#endif

using namespace std;
using string_t = std::basic_string<char_t>;

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::DotnetEnvironment
//
// Description:
// Constructor to initialize the members. The language path is defined as the root path.
//
DotnetEnvironment::DotnetEnvironment(
    std::string language_params,
    std::string language_path,
    std::string public_library_path,
    std::string private_library_path) : m_root_path(to_utf16_str(language_path))
{
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::Init
//
// Description:
// Initialize the runtime by reading the runtimeconfig.json file.
//
short DotnetEnvironment::Init()
{
    LOG("DotnetEnvironment::Init");

    if (!load_hostfxr())
    {
        return E_FAIL;  // Use platform-specific E_FAIL value
    }

    const string_t config_path = m_root_path + STR("/Microsoft.SqlServer.CSharpExtension.runtimeconfig.json");  // Fix string concatenation
    hostfxr_handle cxt = get_dotnet(config_path.c_str());
    m_load_assembly_and_get_function_pointer = get_dotnet_load_assembly(cxt);

    if (m_load_assembly_and_get_function_pointer == nullptr)
    {
        return E_FAIL;
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::to_utf16_str
//
// Description:
// Convert a utf8 string to utf16.
//
string_t DotnetEnvironment::to_utf16_str(const std::string& utf8str)
{
    LOG("DotnetEnvironment::to_utf16_str");

#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific conversion
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
    unique_ptr<wchar_t> wstr(new wchar_t[wchars_num]);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, wstr.get(), wchars_num);
    return string_t(wstr.get());
#else
    // Cross-platform: Use std::wstring_convert for UTF-8 to UTF-16 conversion
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(utf8str);
    return string_t(wstr.begin(), wstr.end());
#endif
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::to_hex_string
//
// Description:
// Convert an int to string in hex.
//
string DotnetEnvironment::to_hex_string(int value)
{
    LOG("DotnetEnvironment::to_hex_string");
    std::stringstream s;
    s << "0x" << std::hex << std::showbase << value;
    return s.str();
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::load_library
//
// Description:
// Load the library from the path
//
void* DotnetEnvironment::load_library(const char_t *path)
{
    LOG("DotnetEnvironment::load_library");

#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific library loading
    HMODULE h = ::LoadLibraryW(path);
    assert(h != nullptr);
    return (void*)h;
#else
    // Cross-platform: Use dlopen for loading shared libraries
    void* lib = dlopen(path, RTLD_LAZY);
    if (!lib)
    {
        LOG_ERROR("Failed to load library: " + string(dlerror()));
    }
    return lib;
#endif
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_export
//
// Description:
// Export the method from the library
//
void* DotnetEnvironment::get_export(void *h, const char *name)
{
    LOG("DotnetEnvironment::get_export");

#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific export loading
    void *f = ::GetProcAddress((HMODULE)h, name);
    assert(f != nullptr);
    return f;
#else
    // Cross-platform: Use dlsym to get the function pointer
    void* f = dlsym(h, name);
    if (!f)
    {
        LOG_ERROR("Failed to get export: " + string(dlerror()));
    }
    return f;
#endif
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::load_hostfxr
//
// Description:
// Load hostfxr and get desired exports
//
bool DotnetEnvironment::load_hostfxr()
{
    LOG("DotnetEnvironment::load_hostfxr");

#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific path and library loading
    string_t hostfxr_location = m_root_path + STR("\\hostfxr.dll");
    void *lib = load_library(hostfxr_location.c_str());
#else
    // Cross-platform: Use shared library loading
    string_t hostfxr_location = m_root_path + STR("/libhostfxr.so");
    void *lib = load_library(hostfxr_location.c_str());
#endif

    m_init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    m_get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    m_close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

    return (m_init_fptr && m_get_delegate_fptr && m_close_fptr);
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet_load_assembly
//
// Description:
// Load assembly function pointer from the path.
//
load_assembly_and_get_function_pointer_fn DotnetEnvironment::get_dotnet_load_assembly(hostfxr_handle cxt)
{
    LOG("DotnetEnvironment::get_dotnet_load_assembly");

    void *load_assembly_and_get_function_pointer = nullptr;

    int rc = m_get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);

    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
    {
        LOG_ERROR("Get delegate failed: " + to_hex_string(rc));
    }

    m_close_fptr(cxt);
    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet
//
// Description:
// Load the .NET from the path and get the hostfxr handle
//
hostfxr_handle DotnetEnvironment::get_dotnet(const char_t *config_path)
{
    LOG("DotnetEnvironment::get_dotnet");

    hostfxr_handle cxt = nullptr;
    int rc = m_init_fptr(config_path, nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        LOG_ERROR("Init failed: " + to_hex_string(rc));
        m_close_fptr(cxt);
        return nullptr;
    }
    return cxt;
}
