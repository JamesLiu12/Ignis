#pragma once
#include <filesystem>
#include <string>
#include <optional>
#include <vector>

namespace ignis
{

	class CMakeBuilder
	{
	public:
		enum class BuildType { Debug, Release, RelWithDebInfo };
		enum class VSArch { X64, X86, ARM64 };

		struct BuildOptions
		{
			std::filesystem::path SourceDir;
			std::filesystem::path BuildDir;
			BuildType BuildType = BuildType::Debug;
			VSArch    Arch = VSArch::X64;
			std::vector<std::string> ExtraArgs;
		};

		static bool IsCMakeAvailable();
		static bool Configure(const BuildOptions& options);
		static bool Build(const BuildOptions& options);
		static bool ConfigureAndBuild(const BuildOptions& options);

#ifdef _WIN32
		static bool EnsureVSEnv(VSArch arch = VSArch::X64);
		static bool IsVSEnvInitialized();
#endif

	private:
		static std::string BuildTypeToString(BuildType type);

#ifdef _WIN32
		static std::optional<std::string> FindVCVarsAll();
		static std::string ArchToString(VSArch arch);
#endif
	};

} // namespace ignis