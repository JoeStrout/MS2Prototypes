// String utilities: conversion between ints and strings, etc.

using System;
// CPP: #include "IOHelper.g.h"

namespace MiniScript {

    public static class StringUtils {
		public const String hexDigits = "0123456789ABCDEF";
		
		public static String ToHex(UInt32 value) {
			char[] hexChars = new char[8]; // CPP: char hexChars[9]; hexChars[8] = 0;
			for (Int32 i = 7; i >= 0; i--) {
				hexChars[i] = hexDigits[(int)(value & 0xF)];
				value >>= 4;
			}
			return new String(hexChars);
		}
		
		//*** BEGIN CS_ONLY ***
		public static string Left(this string s, int n) {
			if (String.IsNullOrEmpty(s)) return "";
			return s.Substring(0, Math.Min(n, s.Length));
		}

		public static string Right(this string s, int n) {
			if (String.IsNullOrEmpty(s)) return "";
			return s.Substring(Math.Max(s.Length-n, 0));
		}
		
		//*** END CS_ONLY ***
	}

}
