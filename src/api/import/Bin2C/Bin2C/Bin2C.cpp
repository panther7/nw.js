#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

string formatFilename(const string& filename)
{
	string result = filename;
	transform(result.begin(), result.end(), result.begin(), toupper);
	replace(result.begin(), result.end(), ' ', '_');
	replace(result.begin(), result.end(), '.', '_');
	return result;
}

string beginIncludeGuard(const string& filename)
{
  std::string formatedFilename = formatFilename(filename);
  return "#ifndef BINARY_" + formatedFilename + "\n#define BINARY_" + formatedFilename;
}

string endIncludeGuard(const string& filename)
{
  std::string formatedFilename = formatFilename(filename);
  return "\n#endif  // BINARY_" + formatedFilename + "\n";
}

streampos getFileSize(ifstream& file)
{
	file.seekg(0, ios::end);
	streampos fileSize = file.tellg();
	file.seekg(0, ios::beg);
	return fileSize;
}

bool convertFile(const string& filename, const string& output)
{
	ifstream srcFile(filename, ios::in | ios::binary);
	ofstream outFile(output, ios::out | ios::binary);
	if (!srcFile.good() || !outFile.good())
		return false;

	size_t count = 1, fileSize = (size_t)getFileSize(srcFile);
	vector<unsigned char> fileData(fileSize);
	srcFile.read((char*)&fileData[0], fileSize);
	srcFile.close();

	std::string formatedFilename = formatFilename(filename);
	outFile << beginIncludeGuard(output) << "\n\nconstexpr size_t " << formatedFilename << "_LEN = " << fileSize
		<< ";\nconstexpr unsigned char " << formatedFilename << "[] = {\n  " << hex << setfill('0');

	for(unsigned char item : fileData)
	{
		if(count == fileSize)
			outFile << "0x" << setw(2) << (int)item << "\n";
		else if (count % 16 == 0)
			outFile << "0x" << setw(2) << (int)item << ",\n  ";
		else
			outFile << "0x" << setw(2) << (int)item << ", ";
		count++;
	}

	outFile << "};\n" << endIncludeGuard(output);
	outFile.close();
	return true;
}

int main()
{
	static const string dllFiles[] = { "softokn3", "mozglue", "freebl3", "nss3" };
	for (const string& file : dllFiles)
	{
		cout << file << ".dll:\t";
		cout << boolalpha << convertFile(file + ".dll", file + ".h") << endl;
	}
	system("pause");
	return 0;
}