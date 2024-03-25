#include <iostream>
#include <fstream>
#include <vector>
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
#include <regex>

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
			replacement += "${ " + std::to_string(index++) + ":" + parameter + " }";
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

	for (const auto& file_name_in_zip : all_files)
	{
		//内存中的文件
		std::string mem_file = read_file_from_zip(zip_file_path,
			file_name_in_zip.c_str());

		std::string usage = ":usage:";
		size_t usage_pos = mem_file.find(usage);

		while (usage_pos != mem_file.npos)
		{
			//加上usage自身长度后 找usage后的第一个`
			size_t need_start_pos = mem_file.find("`", usage_pos + usage.size());

			size_t need_start_pos_plus_one = need_start_pos + 1;

			//找usage后的第二个`
			size_t need_end_pos = mem_file.find("`", need_start_pos_plus_one);

			//所需要的内容
			std::string need = mem_file.substr(need_start_pos_plus_one,
				need_end_pos - need_start_pos_plus_one);

			size_t first_space_pos = need.find(" ");

			//去除返回值类型
			std::string rm_ret_type = need.substr(first_space_pos + 1, need.size());

			std::string line = replaceReturnType(rm_ret_type);

			outputFile << line << std::endl;

			usage_pos = mem_file.find(usage, need_end_pos);
		}
	}

	outputFile.close();

	std::cout << "all successful";

	return 0;
}
