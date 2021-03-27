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

			if (args.Length == 3 || args.Length == 4)
			{
				var target = args[0];
				var directoryPath = args[1];
                directoryPath = Path.Combine(directoryPath, "../../");

				var forLightning = (target == "-Lightning");
				var forPlasma = !forLightning;

				var plasmaPath = directoryPath;
				var lightningPath = directoryPath;


				if (forPlasma)
                    lightningPath = Path.Combine(directoryPath, @"Source\Lightning\LightningAdditions");

				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(lightningPath, @"Project\Lightning"), "*.cpp");

				var standardLibraries = Path.Combine(lightningPath, @"Project\StandardLibraries");

				if (forPlasma)
					standardLibraries = Path.Combine(plasmaPath, @"Source");

				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Core\Common"), "*.cpp");

				// Windows platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Windows")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_WINDOWS)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Windows"), "*.cpp");
				}

				// Posix platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Posix")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_POSIX)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Posix"), "*.cpp");
				}

				// Empty platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Empty")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_EMSCRIPTEN)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Empty"), "*.cpp");
				}

				compactor.HppDirectories.Add(Path.Combine(standardLibraries, @"Core\Common"));
				compactor.HppDirectories.Add(standardLibraries);

				var hppPath = args[2];

				var cppPath = String.Empty;
				if (args.Length == 4)
					cppPath = args[3];
				
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

		static void CompileLib(String vsVersion, String vsYear, String chipset, String configuration, String runtime, String arguments)
		{
			Process compiler = StartCmd(@"C:\LightningBuildOutput\DistributionOut");

			String fileName = "lightning_" + vsYear + "_" + chipset + "_" + runtime;

			String pathIf64 = "";
			String vcVarsVersion = "32";
			if (chipset == "x64")
			{
				pathIf64 = "x86_amd64\\";
				vcVarsVersion = "x86_amd64";
			}

			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"vcvars" + vcVarsVersion + @".bat""");
			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"cl""  /c /Zi /nologo /W4 /WX " + arguments + @" /" + runtime + @" /Gm- /GS /Gy /fp:fast /Gd /Fd""C:\LightningBuildOutput\Distribution\" + fileName + @".pdb"" /Fa""C:\LightningBuildOutput\DistributionOut\\"" /Fo""C:\LightningBuildOutput\DistributionOut\" + fileName + @".obj"" ""C:\Sandbox\Plasma\Lightning\Project\LightningAll\Lightning.cpp""");
			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"lib"" /OUT:""C:\LightningBuildOutput\Distribution\" + fileName + @".lib"" /NOLOGO /LTCG ""C:\LightningBuildOutput\DistributionOut\" + fileName + @".obj""");
			EndCmd(compiler);

			// Capitolize the PDB (cl outputs pdb as all lowercase)
			String pdbName = @"C:\LightningBuildOutput\Distribution\" + fileName + ".pdb";
			File.Move(pdbName, pdbName);
		}

		static Process StartCmd(String workingDirectory)
		{
			Process compiler = new Process();
			compiler.StartInfo.FileName = "cmd.exe";
			compiler.StartInfo.WorkingDirectory = @"C:\LightningBuildOutput\DistributionOut";
			compiler.StartInfo.RedirectStandardInput = true;
			compiler.StartInfo.RedirectStandardOutput = true;
			compiler.StartInfo.UseShellExecute = false;
			compiler.Start();
			return compiler;
		}

		static void EndCmd(Process compiler)
		{
			compiler.StandardInput.WriteLine(@"exit");
			Debug.Write(compiler.StandardOutput.ReadToEnd());
			compiler.WaitForExit();
			compiler.Close();
		}
	}
}
