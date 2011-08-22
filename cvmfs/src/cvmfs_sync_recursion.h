#ifndef CVMFS_SYNC_RECURSION_H
#define CVMFS_SYNC_RECURSION_H 1

#include <string>
#include <assert.h>

#include <iostream> // TODO: remove me

#include "compat.h"
#include "util.h"
#include "cvmfs_sync_aufs.h"

namespace cvmfs {

class UnionFilesystemSync;

enum DirEntryType {
	DE_DIR,
	DE_FILE,
	DE_SYMLINK
};

class DirEntry {
private:
	DirEntryType mType;

	typedef struct {
		bool obtained;
		int errorCode;
		PortableStat64 stat;
	} EntryStat;

	EntryStat mRepositoryStat;
	EntryStat mUnionStat;
	EntryStat mOverlayStat;
	
	bool mWhiteout;

	std::string mRelativePath;
	std::string mFilename;

public:
	DirEntry(const std::string &dirPath, const std::string &filename, const DirEntryType entryType);
	virtual ~DirEntry();

	inline bool isDirectory() const { return mType == DE_DIR; }
	inline bool isRegularFile() const { return mType == DE_FILE; }
	inline bool isSymlink() const { return mType == DE_SYMLINK; }
	inline bool isWhiteout() const { return mWhiteout; }
	
	inline std::string getFilename() const { return mFilename; }

	inline std::string getRelativePath() const { return (mRelativePath.empty()) ? mFilename : mRelativePath + "/" + mFilename; }
	inline std::string getRepositoryPath() const { return UnionFilesystemSync::sharedInstance()->getRepositoryPath() + "/" + getRelativePath(); }
	inline std::string getUnionPath() const { return UnionFilesystemSync::sharedInstance()->getUnionPath() + "/" + getRelativePath(); }
	inline std::string getOverlayPath() const { return UnionFilesystemSync::sharedInstance()->getOverlayPath() + "/" + getRelativePath(); }

	void markAsWhiteout();

	unsigned int getUnionLinkcount();
	uint64_t getUnionInode();
	bool isNew();

private:
	// lazy evaluation and caching of results of file stats
	inline void statRepository() { if (mRepositoryStat.obtained) return; statGeneric(getRepositoryPath(), &mRepositoryStat); } 
	inline void statUnion() { if (mUnionStat.obtained) return; statGeneric(getUnionPath(), &mUnionStat); } 
	inline void statOverlay() { if (mOverlayStat.obtained) return; statGeneric(getOverlayPath(), &mOverlayStat); } 
	void statGeneric(const std::string &path, EntryStat *statStructure);
};

/**
 *  @brief a simple recursion engine to abstract the recursion of directories.
 *  It provides several callback hooks to instrument and control the recursion.
 *  Hooks will be called on the provided delegate object which has to be of type T
 *
 *  Found directory entries are sent back to the delegate object as a pointer to a
 *  DirEntry structure. The delegate object becomes the owner of these objects and
 *  is responsible for them to be freed
 */
template <class T>
class RecursionEngine {
private:
	/** the delegate all hooks are called on */
	T *mDelegate;

	/** dirPath in callbacks will be relative to this directory */
	std::string mRelativeToDirectory;
	bool mRecurse;

public:
	/** message if a directory is entered by the recursion */
	void (T::*enteringDirectory)(DirEntry *entry);

	/** message if a directory is left by the recursion */
	void (T::*leavingDirectory)(DirEntry *entry);

	/** invoked to see if the delegate is interested in a found directory entry */
	bool (T::*caresAbout)(const std::string &filename);

	/** message if a file was found */
	void (T::*foundRegularFile)(DirEntry *entry);

	/**
	 *  message if a directory was found
	 *  if the callback returns true, the recursion will continue in the found directory
	 *  otherwise it will skip it and continue with the next entry in the current director
	 */
	bool (T::*foundDirectory)(DirEntry *entry);

	/** message if a link was found */
	void (T::*foundSymlink)(DirEntry *entry);

public:
	RecursionEngine(T *delegate, const std::string &relativeToDirectory);
	RecursionEngine(T *delegate, const std::string &relativeToDirectory, bool recurse);

	/**
	 *  start the recursion at the given directory
	 */
	void recurse(const std::string &dirPath) const;

private:
	void doRecursion(DirEntry *entry) const;

	bool notifyForDirectory(DirEntry *entry) const;
	void notifyForRegularFile(const std::string &dirPath, const std::string &filename) const;
	void notifyForSymlink(const std::string &dirPath, const std::string &filename) const;
	
	void init(T *delegate, const std::string &relativeToDirectory);

	std::string getRelativePath(const std::string &absolutePath) const;
};

/**********************************
 ** Template implementation
 **********************************/

template <class T>
RecursionEngine<T>::RecursionEngine(T *delegate, const std::string &relativeToDirectory) {
	init(delegate, relativeToDirectory);
	mRecurse = true;
}

template <class T>
RecursionEngine<T>::RecursionEngine(T *delegate, const std::string &relativeToDirectory, bool recurse) {
	init(delegate, relativeToDirectory);
	mRecurse = recurse;
}

template <class T>
void RecursionEngine<T>::init(T *delegate, const std::string &relativeToDirectory) {
	mDelegate = delegate;
	mRelativeToDirectory = canonical_path(relativeToDirectory);

	enteringDirectory = NULL;
	leavingDirectory = NULL;
	caresAbout = NULL;
	foundRegularFile = NULL;
	foundDirectory = NULL;
	foundSymlink = NULL;
}

template <class T>
void RecursionEngine<T>::recurse(const std::string &dirPath) const {
	assert(enteringDirectory != NULL || leavingDirectory != NULL || caresAbout != NULL || foundRegularFile != NULL || foundDirectory != NULL || foundSymlink != NULL);
	assert(mRelativeToDirectory.length() == 0 || dirPath.substr(0, mRelativeToDirectory.length()) == mRelativeToDirectory);

	std::string relativePath = getRelativePath(dirPath);
	DirEntry *directory = new DirEntry(get_parent_path(relativePath), get_file_name(relativePath), DE_DIR);

	doRecursion(directory);
}

template <class T>
void RecursionEngine<T>::doRecursion(DirEntry *entry) const {
	DIR *dip;
	PortableDirent *dit;
	std::string filename, absolutePath;
	
	std::cout << "start recursion... " << entry->getUnionPath() << std::endl;

	// obtain the absolute path by adding the relative portion
	absolutePath = mRelativeToDirectory + "/" + entry->getRelativePath();

	// get into directory and notify the user
	if ((dip = opendir(absolutePath.c_str())) == NULL) {
		return;
	}
	if (enteringDirectory != NULL) (mDelegate->*enteringDirectory)(entry);

	while ((dit = portableReaddir(dip)) != NULL) {
		// skip "virtual" directories
		if (strcmp(dit->d_name, ".") == 0 || strcmp(dit->d_name, "..") == 0) {
			continue;
		}

		filename = dit->d_name;

		// check if user cares about knowing something more
		if (caresAbout != NULL && not (mDelegate->*caresAbout)(filename)) {
			std::cout << "!!! ignored - " << filename << std::endl;
			continue;
		}

		// notify user about found directory entry
		switch (dit->d_type) {
			case DT_DIR:
				// we are only creating a new directory entry if the user wants to see it, if has
				// to be used for recursion or both... otherwise we are skipping the whole stuff
				if (foundDirectory != NULL || mRecurse) {
					DirEntry *newEntry = new DirEntry(entry->getRelativePath(), filename, DE_DIR);
					if (notifyForDirectory(newEntry)) {
						std::cout << "???? recursing into " << newEntry->getRelativePath() << std::endl;
						doRecursion(newEntry); // user can decide to skip directories from recursion
					}
				}
				break;
			case DT_REG:
				notifyForRegularFile(entry->getRelativePath(), filename);
				break;
			case DT_LNK:
				notifyForSymlink(entry->getRelativePath(), filename);
				break;
		}
	}

	// close directory and notify user
	if (closedir(dip) != 0) {
		return;
	}
	if (leavingDirectory != NULL) (mDelegate->*leavingDirectory)(entry);
	
	std::cout << "end recursion... " << entry->getUnionPath() << std::endl;
	
	if (foundDirectory == NULL && enteringDirectory == NULL && leavingDirectory == NULL) {
		delete entry; // the entry for this directory never left the scope and is finished now...
	}
}

template <class T>
bool RecursionEngine<T>::notifyForDirectory(DirEntry *entry) const {
	bool recurse = true;
	if (foundDirectory != NULL) {
		recurse = (mDelegate->*foundDirectory)(entry);
	}
	
	// we are only recursing, if it is generally enabeld (mRecurse) and if the user wants us to
	return recurse && mRecurse;
}

template <class T>
void RecursionEngine<T>::notifyForRegularFile(const std::string &dirPath, const std::string &filename) const {
	if (foundRegularFile == NULL) {
		return;
	}

	DirEntry *entry = new DirEntry(dirPath, filename, DE_FILE);
	(mDelegate->*foundRegularFile)(entry);
}

template <class T>
void RecursionEngine<T>::notifyForSymlink(const std::string &dirPath, const std::string &filename) const {
	if (foundSymlink == NULL) {
		return;
	}

	DirEntry *entry = new DirEntry(dirPath, filename, DE_SYMLINK);
	(mDelegate->*foundRegularFile)(entry);
}

template <class T>
std::string RecursionEngine<T>::getRelativePath(const std::string &absolutePath) const {
	// be careful of trailing '/' --> ( +1 if needed)
	return (absolutePath.length() == mRelativeToDirectory.length()) ? "" : absolutePath.substr(mRelativeToDirectory.length() + 1);
}

}

#endif
