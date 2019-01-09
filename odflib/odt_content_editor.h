#ifndef ODT_CONTENT_EDITOR_H
#define ODT_CONTENT_EDITOR_H

//~ #define ODT_CONTENT_EDITOR_H_USE_COPY

#include <string>
extern "C"{
#include <zip.h>
}

#ifdef _WIN32
	#define SLASH '\\'
#else	
	#define SLASH '/'
#endif

class ODTContentEditor{
	struct zip *m_zs;
	int m_idx;
	
	std::string outputfilename;
	std::string content;
#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY
	std::string office_text;
	static std::string getTextBetween(const std::string *str, const char* begin, const char* end);	
	static void renameStylesAndObjects(std::string *str, const std::string prefix);
	static std::string getStylesText(const std::string *str);
	static std::string getOfficeText(const std::string *str);
	void addOfficeText(const std::string value);
#endif	
	std::string getContent(const char* odtFileName, bool main_file);
public:	
	ODTContentEditor(const char* inputfilename, const char* _outputfilename
#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY	
		, bool usecopy = true
#endif	
	);
	~ODTContentEditor();
	
	void replaceAll(const std::string key, const std::string value);
	void replaceField(const std::string key, const std::string value);
	void replaceFieldWithParagraph(const std::string key, const std::string value);
	void addStyles(const std::string value);
	void addRow(const std::string key, const std::string value);
	void insertRow(const std::string key, unsigned rowsBefore, std::string value);
	void deleteTable(const std::string tableName);
#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY	
	void copyfile(const char* filename);
	void copy();
#endif
	static void replaceAll(std::string *str, const std::string key, const std::string value);
};

#endif //ODT_CONTENT_EDITOR_H
