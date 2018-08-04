#pragma once

#include <string>
#include <cstdarg>
#include <algorithm>

std::string fmt(const char* fmtTemplate, ...)
{
  va_list ap;
  va_start(ap, fmtTemplate);
  char buf[2048];
  int rv;
  rv = vsnprintf(buf, sizeof(buf), fmtTemplate, ap);
#ifdef __MINGW32__
  // MINGW32 vsnprintf returns -1 if output is truncated.
  if (rv < 0 && rv != -1) {
    // Reachable?
    buf[0] = '\0';
  }
#else  // !__MINGW32__
  if (rv < 0) {
    buf[0] = '\0';
  }
#endif // !__MINGW32__
  va_end(ap);
  return buf;
}

bool isAlpha(const char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool isDigit(const char c) { return '0' <= c && c <= '9'; }

bool inRFC3986UnreservedChars(const char c)
{
  static const char unreserved[] = { '-', '.', '_', '~' };
  return isAlpha(c) || isDigit(c) ||
    std::find(std::begin(unreserved), std::end(unreserved), c) != std::end(unreserved);
}

std::string percentEncode(const unsigned char* target, size_t len)
{
  std::string dest;
  for (size_t i = 0; i < len; ++i) {
    if (inRFC3986UnreservedChars(target[i])) {
      dest += target[i];
    }
    else {
      dest.append(fmt("%%%02X", target[i]));
    }
  }
  return dest;
}

std::string percentEncode(const std::string& target)
{
  if (std::find_if_not(target.begin(), target.end(),
    inRFC3986UnreservedChars) == target.end()) {
    return target;
  }
  return percentEncode(reinterpret_cast<const unsigned char*>(target.c_str()),
    target.size());
}
