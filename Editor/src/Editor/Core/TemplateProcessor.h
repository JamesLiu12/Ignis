#pragma once

namespace ignis {

class TemplateProcessor
{
public:
	// Process a template file and write to output
	static bool ProcessTemplate(
		const std::filesystem::path& template_path,
		const std::filesystem::path& output_path,
		const std::map<std::string, std::string>& variables
	);

	// Convert string to uppercase
	static std::string ToUpperCase(const std::string& str);

private:
	// Replace all occurrences of variables in text
	static std::string ReplaceVariables(
		const std::string& text,
		const std::map<std::string, std::string>& variables
	);
};

} // namespace ignis
