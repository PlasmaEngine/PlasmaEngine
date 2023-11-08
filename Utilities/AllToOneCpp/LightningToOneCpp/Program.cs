using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AllToOneCpp;
using System.Diagnostics;
using System.IO;

namespace LightningToOneCpp
{
	class Program
	{
		static int Main(string[] args)
		{
			var compactor = new Compactor();

			var returnValue = 0;

			compactor.EmitLineDirectives = false;
			compactor.Minify = false;

			var overrideArgs = new String[] { "-Plasma", Directory.GetCurrentDirectory(), "Lightning.hpp" };
			//args = overrideArgs;

			if (args.Length == 3 || args.Length == 4)
			{
				var target = args[0];
				var directoryPath = args[1];

				var forLightning = (target == "-Lightning");
				var forPlasma = !forLightning;

				var plasmaPath = directoryPath;
				var lightningPath = directoryPath;

				if (forPlasma)
					lightningPath = Path.Combine(directoryPath, @"Source\Lightning\LightningCore");

				compactor.FilesToProcess.AddFilesFromDirectory(lightningPath, "*.cpp");

				var standardLibraries = Path.Combine(lightningPath, @"Project\StandardLibraries");

				if (forPlasma)
					standardLibraries = Path.Combine(plasmaPath, @"Source");

				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Core/Common"), "*.cpp");

				// Windows platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Windows")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PlasmaTargetOsWindows)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Windows"), "*.cpp");
				}

				//// Posix platform
				//{
				//	compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Posix")), new CompacterDirectives()
				//	{
				//		PreprocessorCondition = "defined(PLATFORM_POSIX)",
				//		CppOnly = true,
				//	});
				//	compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Posix"), "*.cpp");
				//}

				//// Empty platform
				//{
				//	compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Empty")), new CompacterDirectives()
				//	{
				//		PreprocessorCondition = "defined(PLATFORM_EMSCRIPTEN)",
				//		CppOnly = true,
				//	});
				//	compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Empty"), "*.cpp");
				//}

				compactor.HppDirectories.Add(Path.Combine(standardLibraries, @"Common"));
				compactor.HppDirectories.Add(standardLibraries);

				var hppPath = args[2];

				var cppPath = String.Empty;
				if (args.Length == 4)
					cppPath = args[3];

				//var output = Path.Combine(lightningPath, @"Project\lightningAll\NonCompacted\");
				compactor.Compact(cppPath, hppPath, null);

				Console.WriteLine("Done compacting");
			}
			else
			{
				Console.Error.WriteLine("Expected: <-Lightning or -Plasma> <Lightning or Plasma directory> <output.hpp> [output.cpp]");
				Console.Error.WriteLine("Given: `" + Environment.CommandLine + "`");
				returnValue = -1;
			}

			if (Debugger.IsAttached)
			{
				Console.WriteLine("Press enter to continue...");
				Console.ReadLine();
			}

			if (compactor.WasError)
				returnValue = -1;

			return returnValue;
		}
	}
}
