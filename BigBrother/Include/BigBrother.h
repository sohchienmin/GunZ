#ifdef BIGBROTHERDLL_EXPORTS
#define BIGBROTHERDLL_API __declspec(dllexport) 
#else
#define BIGBROTHERDLL_API __declspec(dllimport) 
#endif

#define KILLVERSION	0
#define KILLNOGUNZ	1
#define TRIGGERED	2
#define OK			3

namespace BigBrother {
	class BB {
		public:
			//starts thread to loop check for spooky stuff.
			static BIGBROTHERDLL_API int startBB();
			static BIGBROTHERDLL_API int loopBB();
			static BIGBROTHERDLL_API char* gamecaller();
			//static BIGBROTHERDLL_API bool isRunning(wstring);
			//static BIGBROTHERDLL_API void killProcessByName(const wchar_t);
			static BIGBROTHERDLL_API void exitBB();
	};
}