#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include "miniz.h"

void list_files_in_zip(const char* zip_file_path, std::vector<std::string>& out_path)
{
	mz_zip_archive zip_archive;
	mz_zip_zero_struct(&zip_archive);

	if (!mz_zip_reader_init_file(&zip_archive, zip_file_path, 0))
	{
		std::cerr << "Failed to open ZIP file" << std::endl;
		return;
	}

	for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zip_archive); ++i)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
		{
			std::cerr << "Failed to get file stat" << std::endl;
			mz_zip_reader_end(&zip_archive);
			return;
		}

		char path[256];//包括相对路径
		if (!mz_zip_reader_get_filename(&zip_archive, i, path, sizeof(path)))
		{
			std::cerr << "Failed to get filename" << std::endl;
			mz_zip_reader_end(&zip_archive);
			return;
		}

		//只添加functions文件夹内的
		const char* is_fun = "functions/";
		const char* is_txt = ".txt";
		if (strstr(path, is_fun) != NULL
			&& strstr(path, is_txt) != NULL)
		{
			//path为functions/*.txt
			out_path.push_back(path);
		}
	}

	mz_zip_reader_end(&zip_archive);
}

std::string read_file_from_zip(const char* zip_file_path, const char* file_name)
{
	mz_zip_archive zip_archive;
	mz_zip_zero_struct(&zip_archive);

	if (!mz_zip_reader_init_file(&zip_archive, zip_file_path, 0))
	{
		std::cerr << "Failed to open ZIP file" << std::endl;
		return "";
	}

	mz_uint file_index = mz_zip_reader_locate_file(&zip_archive, file_name, nullptr, 0);
	if (file_index == UINT_MAX) {
		std::cerr << "File not found in ZIP" << std::endl;
		mz_zip_reader_end(&zip_archive);
		return "";
	}

	mz_zip_archive_file_stat file_stat;
	if (!mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat))
	{
		std::cerr << "Failed to get file stat" << std::endl;
		mz_zip_reader_end(&zip_archive);
		return "";
	}

	mz_uint file_size = static_cast<mz_uint>(file_stat.m_uncomp_size);
	char* buffer = new char[file_size];

	if (!mz_zip_reader_extract_to_mem(&zip_archive, file_index, buffer, file_size, 0))
	{
		std::cerr << "Failed to extract file from ZIP" << std::endl;
		delete[] buffer;
		mz_zip_reader_end(&zip_archive);
		return "";
	}

	mz_zip_reader_end(&zip_archive);

	std::string file_content(buffer, file_size);
	delete[] buffer;

	return file_content;
}

std::string replaceReturnType(const std::string& input)
{
	// 定义匹配模式
	std::regex pattern("([a-zA-Z_][a-zA-Z0-9_]*)\\(([^)]*)\\)");

	// 执行匹配
	std::smatch matches;
	if (std::regex_search(input, matches, pattern))
	{
		// 提取函数名和参数列表
		std::string functionName = matches[1];
		std::string parameterList = matches[2];

		// 用逗号分割参数列表
		//std::regex paramPattern("\\s*,\\s*");
		std::regex paramPattern("\\s*[;,]\\s*");	//有时是逗号 有时是分号
		std::sregex_token_iterator it(parameterList.begin(), parameterList.end(), paramPattern, -1);
		std::sregex_token_iterator end;

		std::string replacement = input + "#" + functionName + "("; // 开始替换字符串
		bool firstParam = true; // 用于跟踪是否是参数列表中的第一个参数

		// 遍历参数列表
		int index = 1;
		while (it != end)
		{
			// 提取参数
			std::string parameter = *it;
			// 将参数添加到替换字符串中
			if (!firstParam)
			{
				replacement += ", ";
			}
			else
			{
				firstParam = false;
			}
			replacement += "${" + std::to_string(index++) + ":" + parameter + "}";
			++it;
		}

		// 完成替换字符串
		replacement += ")";

		// 返回替换后的字符串
		return replacement;
	}
	else {
		// 如果没有匹配，则返回原始输入
		return input;
	}
}

int main(int argc, char* argv[])
{
	const char* zip_file_path = argv[1];//"C:/Users/forever/Desktop/vex.zip"
	std::vector<std::string> all_files;

	list_files_in_zip(zip_file_path, all_files);

	std::ofstream outputFile(argv[2]);//"C:/Users/forever/Desktop/functions.txt"

	if (!outputFile.is_open())
	{
		std::cerr << "Failed to open output txt";
		return 0;
	}

	std::regex usage_reg(R"(:usage:\s*`(.*)`)");
	std::regex func_reg(R"(^(\<?\s*\w+(\s*\|\s*\w+)*\s*\>?\s*\[?\]?)\s+(\w+)\s*\(([^)]*)\);?)");

	int error_count = 1;
	for (const auto& file_name_in_zip : all_files)
	{
		std::string mem_file = read_file_from_zip(zip_file_path,
			file_name_in_zip.c_str());	//内存中的文件

		std::sregex_iterator it(mem_file.begin(), mem_file.end(), usage_reg);
		std::sregex_iterator end;

		int usage_safe = 0;
		while (it != end)
		{
			std::smatch usage_match;
			std::string usage_line = it->str();	//usage的一整行
			if (!std::regex_match(usage_line, usage_match, usage_reg))	//整个文件无usage
				break;

			if (usage_match.size() != 2)	//usage一整行有错
			{
				std::cerr << "Error. Num:" << error_count << ". File: " << zip_file_path
					<< " => " << file_name_in_zip
					<< " Line: " << usage_line
					<< std::endl;
				error_count++;
				break;
			}

			std::string func = usage_match[1].str();	//单个函数声明 不包括usage和两端的反引号

			std::smatch func_match;
			if (!std::regex_match(func, func_match, func_reg))	//usage内写错
			{
				std::cerr << "Error. Num:" << error_count << ". File: " << zip_file_path
					<< " => " << file_name_in_zip
					<< " Func: " << func
					<< std::endl;
				error_count++;
				break;
			}

			std::string return_type = func_match[1].str();	// 返回类型
			std::string function_name = func_match[3].str();	// 函数名
			std::string all_parameter = func_match[4].str();	// 全部参数 

			std::regex pattern("[,;]");

			std::sregex_token_iterator iter(all_parameter.begin(), all_parameter.end(), pattern, -1);
			std::sregex_token_iterator end;

			std::vector<std::string> parameters;

			int para_safe = 0;
			while (iter != end)
			{
				parameters.push_back(iter->str());
				++iter;

				para_safe++;
				if (para_safe > 200)
					return 0;
			}

			std::string output;
			output.reserve(400);

			output += function_name + '(';
			for (int i = 0; i < parameters.size(); i++)
			{
				output += parameters[i];
				if (i != parameters.size() - 1)
					output += ',';
			}

			output += ")#" + function_name + '(';

			for (int i = 0; i < parameters.size(); i++)
			{
				output += "${" + std::to_string(i + 1) + ':' + parameters[i] + '}';
				if (i != parameters.size() - 1)
					output += ',';
			}

			output += ')';

			outputFile << output << std::endl;

			it++;

			usage_safe++;
			if (usage_safe > 500)
				return 0;
		}
	}

	outputFile.close();

	std::cout << "GenFunctionsTxtFromVexZip.exe finish. With "
		<< error_count - 1 << " error." << std::endl;

	return 0;
}
