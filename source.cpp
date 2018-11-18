#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <list>
#include <future>
#include <mutex>

using namespace std;
using namespace filesystem;


mutex m, b;


void file_access_output(ofstream output_file, string ss)
{
	unique_lock<mutex> lock{ b };
	if (output_file.is_open())
		output_file << ss << endl;
}

void cout_access_output(string ss)
{
	unique_lock<mutex> lock{ m };
	cout << ss << endl;
}

void thread1(path file_path, path seq_path, path results_path)
{
	const long long buf_size = 256;
	ifstream big{ file_path }, small{ seq_path };
	char buf1[buf_size], buf2[buf_size];
	big.rdbuf()->pubsetbuf(buf1, buf_size);
	small.rdbuf()->pubsetbuf(buf2, buf_size);
	if (big.is_open() && small.is_open())
	{
		bool equals = true;
		big.seekg(0, ios::end);
		unsigned long long size_of_big = big.tellg();
		big.seekg(0, ios::beg);
		small.seekg(0, ios::end);
		unsigned long long size_of_small = small.tellg();
		small.seekg(0, ios::beg);
		int big_s = 0;
		for (unsigned long long i = size_of_small; i <= size_of_big; ++i)
		{
			big.seekg(big_s, ios::beg);
			small.seekg(0, ios::beg);
			for (unsigned long long j = big_s; j < i; ++j)
			{
				char a, b;
				big.read(&a, 1);
				small.read(&b, 1);
				if (a != b)
				{
					equals = false;
					break;
				}
			}
			if (equals == true)
			{
				string ans = string{ "found substring in " } +file_path.filename().generic_string();
				file_access_output(ofstream{ results_path,ios::app }, ans);
				cout_access_output(ans);
				return;
			}
			equals = true;
			++big_s;
		}
		string f_ans = string{ "haven't found substring in " } +file_path.filename().generic_string();
		cout_access_output(f_ans);
		return;
	}
}

pair<string, string> split_wstring(const string& str)
{
	pair<string, string> _pair_obj;
	bool have_passed_point = false;
	auto r_itr = str.rbegin();
	for (auto r_itr = str.rbegin(); r_itr != str.rend(); ++r_itr)
	{
		if (have_passed_point == false)
		{
			if (*r_itr == '.')
			{
				have_passed_point = true;
				reverse(_pair_obj.second.begin(), _pair_obj.second.end());
			}
			else
			{
				_pair_obj.second += *r_itr;
			}
		}
		else
		{
			_pair_obj.first += *r_itr;
		}
	}
	reverse(_pair_obj.first.begin(), _pair_obj.first.end());
	return _pair_obj;
}

class thread_manager
{
private: list<future<void>> future_vec;
public:
	bool add_task(path p, path seq, path res)
	{
		if (future_vec.size() != thread::hardware_concurrency() - 1)
		{
			future_vec.push_back(async(std::launch::async, thread1, p, seq, res));
			return true;
		}
		return false;
	}
	bool _thread_availble()
	{
		auto itr = future_vec.begin();
		while (itr != future_vec.end())
		{
			if (itr->wait_for(0ms) == future_status::ready)
			{
				itr->get();
				itr = future_vec.erase(itr);
			}
			else
			{
				++itr;
			}
		}

		if (future_vec.size() != thread::hardware_concurrency() - 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void clean_up()
	{
		for (future<void>& a : future_vec)
			a.get();
	}
};
bool mask_compatable(const string& mask, const string& filename)
{
	auto mask_pair = split_wstring(mask);
	if (mask_pair.first[0] == L'*' && mask_pair.second[0] == L'*')
	{
		return true;
	}
	else if (mask_pair.first[0] == L'*' && mask_pair.second[0] != L'*')
	{
		auto file_pair = split_wstring(filename);
		if (file_pair.second == mask_pair.second)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (mask_pair.first[0] != L'*' && mask_pair.second[0] == L'*')
	{
		auto file_pair = split_wstring(filename);
		if (file_pair.first == mask_pair.first)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (mask_pair.first[0] != L'*' && mask_pair.second[0] != L'*')
	{
		auto file_pair = split_wstring(filename);
		if (file_pair.first == mask_pair.first && file_pair.second == mask_pair.second)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		throw runtime_error{ "undefined mask" };
	}
}
int main(int argc, char * argv[])
{
	path search_directory_path{ argv[2] }, results_path{ argv[3] }, subsequnce_path{ argv[4] };
	string mask{ argv[5] };

	/*ПРИМЕР */
	/*path search_directory_path{ "C:\\Users\\" }, results_path{ "output.txt" }, subsequnce_path{ "subs.txt" };
	string mask{ "*.*" };*/

	thread_manager object{};

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	ofstream write_results_stream{ results_path.generic_string(),ios::trunc };

	recursive_directory_iterator end_file_itr{};

	for (recursive_directory_iterator file_itr{ search_directory_path }; file_itr != end_file_itr; ++file_itr)
	{
		path current_file_path = *file_itr;
		if (mask_compatable(mask, current_file_path.filename().generic_string()))
		{
			while (!object._thread_availble());
			object.add_task(current_file_path, subsequnce_path, results_path);
		}
	}
	
	object.clean_up();
	end = std::chrono::system_clock::now();

	long long  elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>
		(end - start).count();
	cout << "Search time: " << elapsed_seconds << endl;

	cin.get();
	return 0;
}
