#include<string>
#include<vector>

class Entry {
public:
	Entry(std::string name, bool isDir = false) : name(name), isDir(isDir) {
		if (isDir) {
			entries = new std::vector<Entry>;
		}
	}
	bool isDirectory() {
		return isDir;
	}
	void addEntry(Entry e) {
		entries->push_back(e);
	}
private:
	std::string name;
	bool isDir;
	std::vector<Entry> *entries;
};
