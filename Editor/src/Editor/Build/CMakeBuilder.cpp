#include "CMakeBuilder.h"
#include "Editor/Core/ProcessRunner.h"
#include "Ignis/Core/Log.h"
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ignis
{
	std::string CMakeBuilder::BuildTypeToString(BuildType type)
	{
		switch (type)
		{
		case BuildType::Release:        return "Release";
		case BuildType::RelWithDebInfo: return "RelWithDebInfo";
		default:                        return "Debug";
		}
	}

#ifdef _WIN32

	std::string CMakeBuilder::ArchToString(VSArch arch)
	{
		switch (arch)
		{
		case VSArch::X86:   return "x86";
		case VSArch::ARM64: return "arm64";
		default:            return "x64";
		}
	}

	std::optional<std::string> CMakeBuilder::FindVCVarsAll()
	{
		const std::string vswhere =
			"\"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe\""
			" -latest -property installationPath";

		auto result = ProcessRunner::RunCommand(vswhere);

		if (!result.Success || result.Output.empty())
		{
			Log::CoreError("Could not locate Visual Studio installation via vswhere");
			return std::nullopt;
		}

		std::string install_path = result.Output;
		while (!install_path.empty() &&
			(install_path.back() == '\n' || install_path.back() == '\r'))
			install_path.pop_back();

		return install_path + R"(\VC\Auxiliary\Build\vcvarsall.bat)";
	}

	bool CMakeBuilder::IsVSEnvInitialized()
	{
		return GetEnvironmentVariableA("VCINSTALLDIR", nullptr, 0) > 0;
	}

	bool CMakeBuilder::EnsureVSEnv(VSArch arch)
	{
		if (IsVSEnvInitialized())
			return true;

		auto vcvarsall = FindVCVarsAll();
		if (!vcvarsall)
		{
			Log::CoreError("Cannot initialize VS environment: vcvarsall.bat not found");
			return false;
		}

		std::string command =
			"cmd /c \"\"" + *vcvarsall + "\" " + ArchToString(arch) + " && set\"";

		auto result = ProcessRunner::RunCommand(command);
		if (!result.Success)
		{
			Log::CoreError("Failed to run vcvarsall.bat");
			return false;
		}

		std::istringstream stream(result.Output);
		std::string line;
		int count = 0;
		while (std::getline(stream, line))
		{
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			auto pos = line.find('=');
			if (pos == std::string::npos)
				continue;

			std::string name = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			SetEnvironmentVariableA(name.c_str(), value.c_str());
			++count;
		}

		Log::CoreInfo("VS environment initialized ({} variables, arch: {})",
			count, ArchToString(arch));
		return true;
	}

#endif // _WIN32

	bool CMakeBuilder::IsCMakeAvailable()
	{
#ifdef _WIN32
		auto result = ProcessRunner::RunCommand("cmake --version");
		if (result.Success)
		{
			Log::CoreInfo("CMake available: {}", result.Output.substr(0, result.Output.find('\n')));
			return true;
		}

		Log::CoreInfo("CMake not in PATH, attempting to initialize VS environment...");
		if (!EnsureVSEnv())
			return false;

		result = ProcessRunner::RunCommand("cmake --version");
		if (result.Success)
		{
			Log::CoreInfo("CMake available (via VS env): {}",
				result.Output.substr(0, result.Output.find('\n')));
			return true;
		}

		Log::CoreError("CMake not found even after initializing VS environment");
		return false;
#else
		auto result = ProcessRunner::RunCommand("cmake --version");
		if (result.Success)
			Log::CoreInfo("CMake available: {}", result.Output.substr(0, result.Output.find('\n')));
		else
			Log::CoreError("CMake not found in PATH");
		return result.Success;
#endif
	}

	bool CMakeBuilder::Configure(const BuildOptions& options)
	{
		Log::CoreInfo("CMake Configure: {} -> {}",
			options.SourceDir.string(), options.BuildDir.string());

#ifdef _WIN32
		if (!EnsureVSEnv(options.Arch))
			Log::CoreWarn("Could not initialize VS environment, CMake configure may fail");
#endif

		std::filesystem::create_directories(options.BuildDir);

		std::string command =
			"cmake -S \"" + options.SourceDir.string() + "\""
			" -B \"" + options.BuildDir.string() + "\""
			" -G Ninja"
			" -DCMAKE_BUILD_TYPE=" + BuildTypeToString(options.BuildType);

#ifdef __APPLE__
		command += " -DCMAKE_OSX_ARCHITECTURES=arm64";
		command += " -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0";
#endif

		for (const auto& a : options.ExtraArgs)
			command += " " + a;

		auto result = ProcessRunner::RunCommand(command, options.SourceDir);
		if (result.Success)
		{
			Log::CoreInfo("{}", result.Output);
			Log::CoreInfo("CMake Configure succeeded");
		}
		else
		{
			Log::CoreInfo("{}", result.Output);
			Log::CoreError("CMake Configure failed");
		}
		return result.Success;
	}

	bool CMakeBuilder::Build(const BuildOptions& options)
	{
		Log::CoreInfo("CMake Build: {}", options.BuildDir.string());

#ifdef _WIN32
		if (!EnsureVSEnv(options.Arch))
			Log::CoreWarn("Could not initialize VS environment, CMake build may fail");
#endif

		std::string command =
			"cmake --build \"" + options.BuildDir.string() + "\""
			" --config " + BuildTypeToString(options.BuildType);

		auto result = ProcessRunner::RunCommand(command, options.BuildDir);
		if (result.Success)
		{
			Log::CoreInfo("{}", result.Output);
			Log::CoreInfo("CMake Build succeeded");
		}
		else
		{
			Log::CoreError("{}", result.Output);
			Log::CoreError("CMake Build failed");
		}
		return result.Success;
	}

	bool CMakeBuilder::ConfigureAndBuild(const BuildOptions& options)
	{
		return Configure(options) && Build(options);
	}

} // namespace ignis