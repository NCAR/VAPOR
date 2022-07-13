#include <vapor/GLContextProviderUtil.h>
#include <vapor/GLInclude.h>

String GLContextProviderUtil::GetGLVersion() {
    const char *s = (const char *)glGetString(GL_VERSION);
    assert(s);
    if (!s)
        return "<FAILED glGetString(GL_VERSION)>";
    return String(s);
}

void GLContextProviderUtil::GetGLVersion(int *major, int *minor)
{
    // Only >=3.0 guarentees glGetIntegerv

    String version = GetGLVersion();
    version = version.substr(0, version.find(" "));
    const String majorString = version.substr(0, version.find("."));
    *major = std::stoi(majorString);
    if (majorString.length() < version.length()) {
        version = version.substr(majorString.length() + 1);
        const String minorString = version.substr(0, version.find("."));
        if (!minorString.empty())
            *minor = std::stoi(minorString);
        else
            *minor = 0;
    }
}
