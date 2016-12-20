#pragma once

#define STARBASE_NAME         "Starbase"
#define STARBASE_NAME_LOWER   "starbase"

#if defined(__GNUC__) || defined(__clang___)
	#define SB_LIKELY(x) __builtin_expect(x, 1)
	#define SB_UNLIKELY(x) __builtin_expect(x, 0)
#else
	#define SB_LIKELY(x) (x)
	#define SB_UNLIKELY(x) (x)
#endif
