#pragma once
#include <string>

namespace mif::ila_lseexport::string_utils {

constexpr std::string& replaceAll(std::string& str, std::string_view oldValue, std::string_view newValue) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += newValue.length()) {
        if ((pos = str.find(oldValue, pos)) != std::string::npos) str.replace(pos, oldValue.length(), newValue);
        else break;
    }
    return str;
}


namespace CodePage {
enum : unsigned int {
    DefaultACP = 0,  // default to ANSI code page
    ThreadACP  = 3,  // current thread's ANSI code page
    Symbol     = 42, // SYMBOL translations
    GB2312     = 936,
    UTF8       = 65001,
};
} // namespace CodePage

std::wstring str2wstr(std::string_view str, unsigned int codePage = CodePage::UTF8);

std::string wstr2str(std::wstring_view wstr, unsigned int codePage = CodePage::UTF8);

std::string str2str(
    std::string_view str,
    unsigned int     fromCodePage = CodePage::DefaultACP,
    unsigned int     toCodePage   = CodePage::UTF8
);

inline std::string u8str2str(std::u8string str) {
    std::string& tmp = *reinterpret_cast<std::string*>(&str);
    return {std::move(tmp)};
}

} // namespace mif::ila_lseexport::string_utils