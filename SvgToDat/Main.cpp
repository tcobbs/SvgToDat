#include "SvgLoader.h"
#include <string>

int main(int argc, char *argv[])
{
	SvgLoader oLoader;

	if (argc > 1)
	{
		// "C:/Dev/SvgToDat/TestInput/sw209.svg"
		// "C:/Dev/SvgToDat/TestInput/sw040 simplified.svg"
		// "C:/Dev/SvgToDat/TestInput/sw040 simplified 2.svg"
		if (oLoader.Load(argv[1]))
		{
			const char *pszDot = strrchr(argv[1], '.');
			const char *pszSlash = strrchr(argv[1], '/');
			const char *pszBackslash = strrchr(argv[1], '\\');
			std::string strOutFilename(argv[1]);

			if (pszBackslash > pszSlash)
			{
				pszSlash = pszBackslash;
			}
			if (pszDot > pszSlash)
			{
				strOutFilename.resize(pszDot - argv[1]);
			}
			strOutFilename += ".ldr";
			oLoader.WriteDat(strOutFilename);
			return 0;
		}
	}
	else
	{
		printf("Usage: SvgToDat <SVG Filename>\r\n");
	}
	return 1;
}
