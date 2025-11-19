//*** BEGIN CS_ONLY ***
// Extra functions/utilities for the C# String class, to parallel things
// we have to do in C++ (for example, global ToString functions).  This
// file is independent of the runtime Value system, and should be usable
// even in test projects that know nothing about Value or GC.

using System;

namespace MS2Proto5 {
	public static class StringExtras {
		public static String ToString(Double d, String format=null) {
			return d.ToString(format);
		}
	}
}

//*** END CS_ONLY ***