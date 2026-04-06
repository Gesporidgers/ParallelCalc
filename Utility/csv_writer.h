#pragma once
#include <iostream>
#include <fstream>
class csv_writer
{
private:
	std::ofstream file;
	bool first = true;
public:
	csv_writer(std::string filename) : file(filename) {}
	~csv_writer() { file.close(); }

	template<typename T>
	inline csv_writer& operator<<(const T& value) {
		if (!first)
			file << ',';
		file << value;
		first = false;
		return *this;
	}
	inline void end_row() { file << '\n'; first = true; }
	inline void close() { file.close(); }
};

