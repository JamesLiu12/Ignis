#include "TemplateProcessor.h"
#include "Ignis/Core/File/FileSystem.h"
#include "Ignis/Core/Log.h"

namespace ignis {

bool TemplateProcessor::ProcessTemplate(
	const std::filesystem::path& template_path,
	const std::filesystem::path& output_path,
	const std::map<std::string, std::string>& variables)
{
	// Read template file
	std::string template_content = FileSystem::ReadTextFile(template_path);
	if (template_content.empty() && !FileSystem::Exists(template_path))
	{
		Log::CoreError("Template file not found: {}", template_path.string());
		return false;
	}
	
	// Replace variables
	std::string processed = ReplaceVariables(template_content, variables);
	
	// Write output file
	if (!FileSystem::WriteTextFile(output_path, processed))
	{
		Log::CoreError("Failed to write processed template: {}", output_path.string());
		return false;
	}
	
	Log::CoreInfo("Processed template: {} -> {}", template_path.filename().string(), output_path.filename().string());
	return true;
}

std::string TemplateProcessor::ReplaceVariables(
	const std::string& text,
	const std::map<std::string, std::string>& variables)
{
	std::string result = text;
	
	for (const auto& [key, value] : variables)
	{
		size_t pos = 0;
		while ((pos = result.find(key, pos)) != std::string::npos)
		{
			result.replace(pos, key.length(), value);
			pos += value.length();
		}
	}
	
	return result;
}

std::string TemplateProcessor::ToUpperCase(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

} // namespace ignis
