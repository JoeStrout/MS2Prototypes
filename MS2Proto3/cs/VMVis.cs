using System;

/*** BEGIN CPP_ONLY ***
#include "StringUtils.g.h"
#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/ioctl.h>
	#include <unistd.h>
#endif
*** END CPP_ONLY ***/


namespace MiniScript {

	public class VMVis {
		private const String Esc = "\x1b";
		private const String Clear = Esc + "]2J";
		private const String Reset = Esc + "c";
		private const String Bold = Esc + "[1m";
		private const String Dim = Esc + "[2m";
		private const String Underline = Esc + "[4m";
		private const String Inverse = Esc + "[7m";
		private const String Normal = Esc + "[m";
		
		private const String CursorHome = Esc + "[f";
		private const Int32 CodeDisplayColumn = 0;

		private Int32 _screenWidth;
		private Int32 _screenHeight;
		private VM _vm;

		public VMVis(VM vm) {
			_vm = vm;
			UpdateScreenSize();
		}

		public void UpdateScreenSize() {
			//*** BEGIN CS_ONLY ***
			try {
				_screenWidth = Console.WindowWidth;		// CPP: // see below
				_screenHeight = Console.WindowHeight;	// CPP: 
			} catch {
				_screenWidth = 80;
				_screenHeight = 24;
			}
			//*** END CS_ONLY ***
			/*** BEGIN CPP_ONLY ***
			#ifdef _WIN32
				_screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				_screenHeight = rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;	
			#else
				struct winsize w;
				ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				_screenWidth = w.ws_row;
				_screenHeight = w.ws_col;
			#endif
			*** END CPP_ONLY ***/
		}

		public Int32 ScreenWidth { get { return _screenWidth; } }
		public Int32 ScreenHeight { get { return _screenHeight; } }

		public String CursorGoTo(int column, int row) {
			return StringUtils.Format("\x1b[{0};{1}H", row, column);
		}

		private void Write(String s) {
			Console.Write(s); // CPP: std::cout << s.c_str();
		}

		public void ClearScreen() {
			UpdateScreenSize();
			Write(Reset + Clear + CursorHome);
		}
		

		public void GoTo(int column, int row) {
			Write(CursorGoTo(column, row));
		}

		private void DrawCodeDisplay() {
			if (_vm.CurrentFunction == null) return;

			FuncDef func = _vm.CurrentFunction;
			Int32 pc = _vm.PC;

			// Draw function name header
			GoTo(CodeDisplayColumn + 1, 1);
			String header = Bold + func.Name + Normal;
			Write(StringUtils.SpacePad(header, 32));

			// Draw code, with current line in bold
			Int32 startLine = Math.Max(0, pc - (_screenHeight - 4) / 2);
			Int32 endLine = Math.Min(func.Code.Count - 1, startLine + _screenHeight - 4);

			for (Int32 i = startLine; i <= endLine; i++) {
				String prefix = (i == pc) ? "PC: " + Bold : "    ";
				String addr = StringUtils.ZeroPad(i, 4);
				String instruction = Disassembler.ToString(func.Code[i]);
				String line = prefix + addr + ": " + instruction;
				if (i == pc) line += Normal;

				GoTo(CodeDisplayColumn + 1, i - startLine + 2);
				Write(StringUtils.SpacePad(line, i == pc ? 40 : 32));
			}

			// Clear remaining lines if we switched to a shorter function
			Int32 totalCodeLines = _screenHeight - 4;
			Int32 linesDrawn = endLine - startLine + 1;
			for (Int32 i = linesDrawn; i < totalCodeLines; i++) {
				GoTo(CodeDisplayColumn + 1, i + 2);
				Write(StringUtils.SpacePad("", 32));
			}
		}
		
		public void UpdateDisplay() {
			DrawCodeDisplay();
			GoTo(1, _screenHeight - 2);
		}
	}
}
